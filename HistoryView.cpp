// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		05jan09	in SetViewPos, use cached image if available
		02		07feb09	in SetViewPos, save escape times
		03		14feb09	in default restore, render viewed item, not last item
		04		24feb09	in Render, add multiple render
		05		01mar09	handle navigation in PreTranslateMessage
		06		13mar09	add get/set project
		07		14mar09	add slide show manager
		08		02jul09	in OnLButtonUp, offset both coordinates
		09		02jul09	in DropHitTest, use HWND to avoid temp objects
		10		11jul09	allow unlimited record points
		11		03dec09	if drag is canceled, stop scrolling
		12		03dec09	highlight drop position
		13		06jan10	W64: make OnTimer 64-bit compatible
		14		06jan10	W64: in AddItem, cast item index to 32-bit
		15		06jan10	W64: in OnMouseWheel, cast SendMessage to BOOL
		16		06jan10	W64: cast SelIdx array size to 32-bit

		history view
 
*/

// HistoryView.cpp : implementation file
//

#include "stdafx.h"
#include "Fractice.h"
#include "HistoryBar.h"
#include "HistoryView.h"
#include "FracticeDoc.h"
#include "FracticeView.h"
#include "MainFrm.h"
#include "RenderingDlg.h"
#include "CustomMsgBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHistoryView

IMPLEMENT_DYNCREATE(CHistoryView, CScrollView)

#define CFV CFracticeView

#define DUMP_LEVEL	0

CHistoryView::CHistoryView() :
	m_Clipboard(NULL, _T("Fractice_1_0"))
{
	m_View = NULL;
	m_SelCount = 0;
	m_SelMark = -1;
	m_CurPos = -1;
	m_ViewPos = -1;
	m_Rows = 0;
	m_Cols = 0;
	m_ThumbSize = CSize(0, 0);
	m_ContextPos = -1;
	m_DragState = DTS_NONE;
	m_DragOrigin = CPoint(0, 0);
	m_DragPos = -1;
	m_ViewPosIcon = NULL;
	m_ViewPosIconSize = CSize(0, 0);
	m_UndoArg = 0;
	m_ScrollDelta = 0;
	m_DropPos = -1;
}

CHistoryView::~CHistoryView()
{
}

void CHistoryView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();
}

/////////////////////////////////////////////////////////////////////////////
// CHistoryView operations

void CHistoryView::GetProject(CProject& Proj) const
{
	GetHistory(Proj.m_History);	// copy history
	Proj.m_Snapshot.SetEmpty();	// assume no project snapshot
	// if there's no current history item, or current history item doesn't have
	// an image, or differs from view's current state, create a project snapshot
	if (m_ViewPos < 0 || !m_History[m_ViewPos]->HasImage() 
	|| *m_History[m_ViewPos] != m_View->GetSnapshot()) {
		if (m_View->IsIdle())	// but only if view is idle, to avoid corruption
			Proj.m_Snapshot.CreateObj(m_View->GetSnapshot());	// save view's state
	}
	Proj.m_HistPos = m_ViewPos;	// save history position
}

void CHistoryView::SetProject(const CProject& Proj)
{
	SetHistory(Proj.m_History);	// copy history
	int	HistPos = Proj.m_HistPos;
	if (Proj.HasSnapshot())	{	// if project has a snapshot
		m_View->SetSnapshot(Proj.m_Snapshot);	// render snapshot
	} else {	// project doesn't have a snapshot
		if (HistPos >= 0)	// if history position is valid
			m_View->SetSnapshotCached(m_History[HistPos]);	// render history item
	}
	m_ViewPos = HistPos;	// restore history position
	UpdateLayout(HistPos);	// update layout and scroll to position
}

inline CPoint CHistoryView::GetOrigin(int ItemIdx) const
{
	CPoint	pt;
	pt.x = ItemIdx % m_Cols * (m_ThumbSize.cx + THUMB_MARGIN) + THUMB_MARGIN;
	pt.y = ItemIdx / m_Cols * (m_ThumbSize.cy + THUMB_MARGIN) + THUMB_MARGIN;
	return(pt);
}

void CHistoryView::UpdateLayout(int VisibleItem)
{
	int	items = GetItemCount();
	m_ThumbSize = m_View->GetThumbSize();
	CSize	st = m_ThumbSize + CSize(THUMB_MARGIN, THUMB_MARGIN);
	CRect	rc;
	GetClientRect(rc);
	if (items) {
		m_Cols = CLAMP(rc.Width() / st.cx, 1, items);
		m_Rows = (items - 1) / m_Cols + 1;
	} else {
		m_Cols = 0;
		m_Rows = 0;
	}
	CSize	ss(m_Cols * st.cx + THUMB_MARGIN, m_Rows * st.cy + THUMB_MARGIN);
	if (IsWindowVisible()) {
		SetRedraw(FALSE);	// avoid flicker
		SetScrollSizes(MM_TEXT, ss);
		if (VisibleItem >= 0)
			EnsureVisible(VisibleItem);
		SetRedraw(TRUE);
		Invalidate();
	} else {	// we're hidden
		// don't use SetRedraw, else we'll repaint even though we're hidden
		SetScrollSizes(MM_TEXT, ss);
		if (VisibleItem >= 0)
			EnsureVisible(VisibleItem);
	}
}

void CHistoryView::EnsureVisible(int ItemIdx, int Count)
{
	CPoint	sp = GetScrollPosition();
	CRect	rc;
	GetClientRect(rc);
	if (GetTotalSize().cy > rc.Height()) {	// if vertically scrollable
		CPoint	org = GetOrigin(ItemIdx);
		int	y = org.y - THUMB_MARGIN;
		if (y - sp.y < rc.top)
			ScrollToPosition(CPoint(sp.x, y));
		else {
			y = org.y + (m_ThumbSize.cy + THUMB_MARGIN) * Count;
			if (y - sp.y > rc.bottom)
				ScrollToPosition(CPoint(sp.x, y - rc.bottom));
		}
	}
	Invalidate();
}

void CHistoryView::RedrawItem(int ItemIdx)
{
	CPoint	pt = GetOrigin(ItemIdx) - GetScrollPosition();
	CRect	r(pt, m_ThumbSize);
	InvalidateRect(r);
}

int CHistoryView::GetCountPerPage() const
{
	CRect	r;
	GetClientRect(r);
	return(r.Height() / (m_ThumbSize.cy + THUMB_MARGIN) * m_Cols);
}

void CHistoryView::ShowDetail(int ItemIdx, bool Enable)
{
	m_History[ItemIdx].m_ShowDetail = Enable;
	Invalidate();
}

int CHistoryView::FindItem(CPoint Point) const
{
	Point += GetScrollPosition();
	int	items = GetItemCount();
	for (int i = 0; i < items; i++) {
		CPoint	org = GetOrigin(i);
		CRect	rt(org, m_ThumbSize);
		rt.InflateRect(THUMB_MARGIN, THUMB_MARGIN);	// include selection border
		if (rt.PtInRect(Point))
			return(i);
	}
	return(-1);
}

void CHistoryView::RemoveAll()
{
	m_History.RemoveAll();
	m_SelCount = 0;
	m_SelMark = -1;
	m_CurPos = -1;
	m_ViewPos = -1;
}

void CHistoryView::AddItem(CHistoryItem& Item, bool DeferUpdate)
{
	m_ViewPos = INT64TO32(m_History.Add(Item));
	if (!DeferUpdate)
		UpdateLayout(m_ViewPos);	// scroll to end
}

void CHistoryView::InsertItem(int ItemIdx, CHistoryItem& Item, bool DeferUpdate)
{
	ASSERT(ItemIdx >= 0 && ItemIdx <= GetItemCount());	// ok to insert at end
	m_History.InsertAt(ItemIdx, Item);
	if (IsSelected(ItemIdx))
		m_SelCount++;
	m_CurPos = ItemIdx;
	if (ItemIdx <= m_ViewPos)	// if inserting below viewed item
		m_ViewPos++;				// compensate viewed item position
	if (!DeferUpdate)
		UpdateLayout();
}

void CHistoryView::RemoveItem(int ItemIdx, bool DeferUpdate)
{
	ASSERT(ItemIdx >= 0 && ItemIdx < GetItemCount());
	if (IsSelected(ItemIdx))
		m_SelCount--;
	m_History.RemoveAt(ItemIdx);
	if (ItemIdx == m_ViewPos)		// if deleting viewed item
		m_ViewPos = -1;					// invalidate viewed item position
	else if (ItemIdx < m_ViewPos)	// if deleting below viewed item
		m_ViewPos--;					// compensate viewed item position
	if (ItemIdx > 0 && ItemIdx == GetItemCount())	// if removing at end
		ShowDetail(ItemIdx - 1, FALSE);	// hide preceding item's detail rect
	if (!DeferUpdate)
		UpdateLayout();
}

void CHistoryView::SetViewPos(int ItemIdx, bool AllowRemap)
{
	ASSERT(ItemIdx < GetItemCount());
	if (ItemIdx == m_ViewPos)
		return;	// nothing to do
	if (ItemIdx >= 0) {
		m_View->SetSnapshotCached(m_History[ItemIdx], AllowRemap);
		EnsureVisible(ItemIdx);
	} else
		Invalidate();
	m_ViewPos = ItemIdx;
}

void CHistoryView::RenderItem(int ItemIdx, UINT Flags)
{
	Deselect();
	if ((Flags & RF_UNDOABLE) && ItemIdx != m_ViewPos && ItemIdx >= 0)
		m_View->NotifyUndoableEdit(CFV::UCODE_HISTORY_RENDER);
	SetViewPos(ItemIdx, (Flags & RF_ALLOW_REMAP) != 0);
}

void CHistoryView::SelectItem(int ItemIdx, bool Selected, bool DeferUpdate)
{
	ASSERT(ItemIdx >= 0 && ItemIdx < GetItemCount());
	if (Selected == IsSelected(ItemIdx))
		return;	// nothing to do
	if (Selected)
		m_SelCount++;
	else
		m_SelCount--;
	m_History[ItemIdx].m_Selected = Selected;
	m_CurPos = ItemIdx;
	if (!DeferUpdate) {
		Invalidate();
		EnsureVisible(ItemIdx);
	}
}

void CHistoryView::ToggleSelect(int ItemIdx)
{
	ASSERT(ItemIdx >= 0 && ItemIdx < GetItemCount());
	m_SelMark = ItemIdx;
	SelectItem(ItemIdx, !IsSelected(ItemIdx));
}

void CHistoryView::SelectSpan(int ItemIdx)
{
	ASSERT(ItemIdx >= 0 && ItemIdx < GetItemCount());
	int	anchor = max(GetSelectionMark(), 0);
	int	start, end;
	if (anchor < ItemIdx) {
		start = anchor;
		end = ItemIdx;
	} else {
		start = ItemIdx;
		end = anchor;
	}
	Deselect();
	for (int i = start; i <= end; i++)
		SelectItem(i, TRUE, TRUE);	// defer update
	m_CurPos = ItemIdx;
	Invalidate();
	EnsureVisible(ItemIdx);
}

void CHistoryView::SelectAll()
{
	int	items = GetItemCount();
	if (items) {
		m_SelMark = 0;
		SelectSpan(items - 1);
	}
}

void CHistoryView::Deselect()
{
	if (!m_SelCount)
		return;	// nothing to do
	int	items = GetItemCount();
	for (int i = 0; i < items; i++) {
		if (IsSelected(i))
			m_History[i].m_Selected = FALSE;
	}
	Invalidate();
	m_SelCount = 0;
}

void CHistoryView::SetSelectionMark(int ItemIdx)
{
	ASSERT(ItemIdx < GetItemCount());
	Deselect();
	m_SelMark = ItemIdx;
	if (ItemIdx >= 0)
		SelectItem(ItemIdx, TRUE);
}

int CHistoryView::GetSelectionMark() const
{
	return(m_SelMark >= 0 && m_SelMark < GetItemCount() ? m_SelMark : -1);
}

void CHistoryView::DeleteSelectedItems()
{
	if (!HaveSelection())
		return;	// nothing to do
	int i = 0;
	while (i < GetItemCount()) {	// don't cache size; deletion changes it
		if (IsSelected(i)) {
			RemoveItem(i, TRUE);	// defer update
		} else	// item not selected
			i++;	// next item
	}
	UpdateLayout();
}

void CHistoryView::MoveSelectedItems(int ItemIdx)
{
	ASSERT(ItemIdx >= 0 && ItemIdx <= GetItemCount());	// ok to move to end
	if (!HaveSelection())
		return;	// nothing to do
	CHistoryItem	CurItem;
	if (m_ViewPos >= 0)	// if viewed item position is valid
		CurItem = m_History[m_ViewPos];	// save pointer to viewed item
	int	sels = m_SelCount;
	CHistory	mover;	// backup of moving items
	mover.SetSize(sels);
	int	DstPos = ItemIdx;
	int	i = 0, j = 0;
	while (i < GetItemCount()) {	// don't cache size; deletion changes it
		if (IsSelected(i)) {	// if item selected
			mover[j++] = m_History[i];	// copy item to backup array
			RemoveItem(i, TRUE);	// delete from history; defer update
			if (i < DstPos)	// if deletion occurred below destination
				DstPos--;	// compensate destination
		} else	// item not selected
			i++;	// next item
	}
	m_SelMark = DstPos;
	for (i = 0; i < sels; i++) {
		InsertItem(DstPos, mover[i], TRUE);	// defer update
		if (mover[i].GetObj() == CurItem.GetObj())	// if item was viewed
			m_ViewPos = DstPos;	// reset viewed item position
		DstPos++;
	}
	UpdateLayout();
}

void CHistoryView::GetSelection(CDWordArray& SelIdx) const
{
	SelIdx.RemoveAll();
	int	items = GetItemCount();
	for (int i = 0; i < items; i++) {
		if (IsSelected(i))
			SelIdx.Add(i);
	}
}

void CHistoryView::SetSelection(const CDWordArray& SelIdx)
{
	Deselect();
	int	sels = INT64TO32(SelIdx.GetSize());
	if (sels) {
		for (int i = 0; i < sels; i++)
			SelectItem(SelIdx[i], TRUE, TRUE);	// selected, defer update
		m_SelMark = SelIdx[sels - 1];
		EnsureVisible(SelIdx[0]);
	}
}

bool CHistoryView::Copy()
{
	CSnapshotArray	sa;
	int	items = GetItemCount();
	for (int i = 0; i < items; i++) {
		if (IsSelected(i))
			sa.Add(m_History[i]);
	}
	if (!sa.GetSize())
		return(FALSE);
	CMemFile	mf;
	{
		CArchive	ar(&mf, CArchive::store);
		sa.Serialize(ar);
	}
	DWORD	len = static_cast<DWORD>(mf.GetLength());
	BYTE	*pBuf = mf.Detach();
	m_Clipboard.Write(pBuf, len);
	free(pBuf);	// we're responsible for cleaning up detached memory
	return(TRUE);
}

bool CHistoryView::Cut()
{
	if (!Copy())
		return(FALSE);
	m_View->NotifyUndoableEdit(CFV::UCODE_HISTORY_CUT);
	DeleteSelectedItems();
	return(TRUE);
}

bool CHistoryView::Paste()
{
	DWORD	len;
	PBYTE	pData = (PBYTE)m_Clipboard.Read(len);
	if (pData == NULL)
		return(FALSE);
	CMemFile	mf;
	mf.Attach(pData, len);
	CSnapshotArray	sa;
	TRY {
		CArchive	ar(&mf, CArchive::load);
		ar.m_strFileName = _T("Clipboard");	// in case serialize throws
		sa.Serialize(ar);
	}
	CATCH (CArchiveException, e) {
		e->ReportError();
		delete pData;	// clean up attached memory regardless
		return(FALSE);
	}
	END_CATCH
	delete pData;	// we're responsible for cleaning up attached memory
	Deselect();
	int	NewItems = sa.GetSize();
	int	InsPos = GetSelectionMark();
	if (InsPos < 0)	// if invalid selection mark
		InsPos = GetItemCount();	// append
	int	j = InsPos;
	for (int i = 0; i < NewItems; i++) {
		CHistoryItem	item;
		item.CreateObj(sa[i]);
		item.m_Selected = TRUE;	// select for SaveUndoState
		InsertItem(j, item, TRUE);	// defer update
		j++;
	}
	m_View->NotifyUndoableEdit(CFV::UCODE_HISTORY_PASTE);
	UpdateLayout();
	EnsureVisible(InsPos, NewItems);
	m_SelMark = InsPos;
	return(TRUE);
}

bool CHistoryView::Delete()
{
	if (!HaveSelection())
		return(FALSE);
	m_View->NotifyUndoableEdit(CFV::UCODE_HISTORY_DELETE);
	DeleteSelectedItems();
	return(TRUE);
}

bool CHistoryView::CanRender() const
{
	int	pos = GetSelectionMark();
	return(HaveSelection() && pos >= 0 && (pos != m_ViewPos || m_SelCount > 1));
}

void CHistoryView::Render()
{
	if (CanRender()) {
		if (m_SelCount > 1) {	// if multiple selection
			CRenderingDlg	dlg;
			dlg.DoModal();	// do multiple render
		} else {
			int	pos = GetSelectionMark();
			RenderItem(pos, RF_UNDOABLE | RF_ALLOW_REMAP);	// render selected item
		}
	}
}

void CHistoryView::AddRecordPoints(int InsPos, bool Replace)
{
	if (GetSelectedCount()) {
		int	items = GetItemCount();
		CSnapshotArray	SnapList;
		for (int i = 0; i < items; i++) {
			if (IsSelected(i))
				SnapList.Add(m_History[i]);
		}
		CRecordDlg&	RecDlg = theApp.GetMain()->GetRecordDlg();
		RecDlg.Show();	// show record dialog
		if (Replace && RecDlg.GetSnapCount()) {
			CString	sReplace((LPCTSTR)IDS_BT_REPLACE);
			CString	sAppend((LPCTSTR)IDS_BT_APPEND);
			LPCTSTR	aButton[] = {0, 0, 0, 0, 0, sReplace, sAppend};
			int	id = CCustomMsgBox::MsgBox(LDS(IDS_REC_REPLACE_SCRIPT_WARN),
				MB_YESNOCANCEL | MB_ICONQUESTION, aButton, 7);
			switch (id) {
			case IDYES:
				RecDlg.SetSnaps(SnapList);
				break;
			case IDNO:
				RecDlg.InsertSnaps(-1, SnapList);
				break;
			}
		} else
			RecDlg.InsertSnaps(InsPos, SnapList);
	}
}

void CHistoryView::Goto(int LocationCode)
{
	if (m_DragState == DTS_DRAG)	// if dragging
		return;	// not supported during drag
	if (GetItemCount() > 0) {
		int	pos;
		switch (LocationCode) {
		case LC_TOP:
			pos = 0;
			break;
		case LC_BOTTOM:
			pos = GetItemCount() - 1;
			break;
		case LC_UP:
			pos = m_CurPos - 1;
			break;
		case LC_DOWN:
			pos = m_CurPos + 1;
			break;
		case LC_PAGE_UP:
			pos = m_CurPos - GetCountPerPage();
			break;
		case LC_PAGE_DOWN:
			pos = m_CurPos + GetCountPerPage();
			break;
		default:
			ASSERT(0);	// logic error
			pos = 0;
		}
		pos = CLAMP(pos, 0, GetItemCount() - 1);
		if (GetAsyncKeyState(VK_SHIFT) & GKS_DOWN)
			SelectSpan(pos);
		else
			SetSelectionMark(pos);
	}
}

void CHistoryView::EndDrag()
{
	if (m_DragState != DTS_NONE) {	// if dragging
		ReleaseCapture();
		EraseDropPos();
		theApp.GetMain()->GetRecordDlg().EndDrag();
		m_ScrollTimer.Run(FALSE);	// stop scrolling
		m_DragState = DTS_NONE;	// end drag
	}
}

int CHistoryView::DropHitTest(CPoint point)
{
	CPoint	pt(point);
	ClientToScreen(&pt);
	HWND	hWnd = ::WindowFromPoint(pt);	// avoid temporary window objects
	CRecordDlg&	rd = theApp.GetMain()->GetRecordDlg();
	if (hWnd == rd.m_hWnd || ::IsChild(rd.m_hWnd, hWnd))
		return(DT_RECORD);
	if (hWnd == m_hWnd)
		return(DT_HISTORY);
	if (hWnd == m_View->m_hWnd)
		return(DT_VIEW);
	return(DT_NONE);
}

int CHistoryView::GetInsertPos(CPoint point)
{
	// offset point to find nearest boundary between thumbnails
	if (m_Cols > 1) {	// if multiple columns
		point.x += m_ThumbSize.cx / 2 + THUMB_MARGIN;	// offset by half a column
		int	x = point.x + GetScrollPosition().x;
		if (x > GetTotalSize().cx) {	// if past end of row
			point.x = 0;	// wrap to next row
			point.y += m_ThumbSize.cy;
		}
	} else {	// single column
		if (m_Rows > 1) {	// if multiple rows
			point.x = 0;	// ignore x
			point.y += m_ThumbSize.cy / 2 + THUMB_MARGIN;	// offset by half a row
		}
	}
	int	InsPos = FindItem(point);
	if (InsPos < 0)	// if not over an item
		InsPos = GetItemCount();	// append
	return(InsPos);
}

void CHistoryView::UpdateDropPos(CPoint point)
{
	int	DropPos = GetInsertPos(point);
	if (DropPos != m_DropPos) {	// if drop position changed
		EraseDropPos();	// erase previous drop indicator
		DrawDropPos(DropPos, TRUE);	// draw new drop indicator
		m_DropPos = DropPos;	// update drop position
	}
}

void CHistoryView::EraseDropPos()
{
	if (m_DropPos >= 0) {	// if drop position is visible
		DrawDropPos(m_DropPos, FALSE);	// erase drop indicator
		m_DropPos = -1;	// reset drop position
	}
}

void CHistoryView::DrawDropPos(int Item, bool Enable)
{
	ASSERT(Item >= 0 && Item <= GetItemCount());
	CPoint	pt(GetOrigin(Item) - GetScrollPosition());
	CRect	cr(pt, CSize(m_ThumbSize.cx, m_ThumbSize.cy));
	static const int	ARROWS = 2;
	CPoint	arpt[ARROWS];
	bool	vert = m_Cols > 1;
	if (vert) {
		int	ox = cr.left - THUMB_MARGIN / 2;
		arpt[0] = CPoint(ox, cr.top - THUMB_MARGIN);
		arpt[1] = CPoint(ox, cr.bottom + THUMB_MARGIN);
	} else {
		int	oy = cr.top - THUMB_MARGIN / 2;
		arpt[0] = CPoint(cr.left - THUMB_MARGIN, oy);
		arpt[1] = CPoint(cr.right + THUMB_MARGIN, oy);
	}
	CRgn	arrow[ARROWS];
	MakeArrow(arpt[0], TRUE, vert, arrow[0]);
	MakeArrow(arpt[1], FALSE, vert, arrow[1]);
	CDC	*pDC = GetDC();
	if (Enable) {
		COLORREF	InsColor = RGB(255, 0, 0);
		CPen	pen(PS_DOT, 1, InsColor);
		CBrush	brush(InsColor);
		CPen	*pPrevPen = pDC->SelectObject(&pen);
		pDC->SetBkMode(TRANSPARENT);
		pDC->MoveTo(arpt[0]);	// draw line
		pDC->LineTo(arpt[1]);
		for (int i = 0; i < ARROWS; i++)	// draw arrows
			pDC->FillRgn(&arrow[i], &brush);
		pDC->SelectObject(pPrevPen);
	} else {	// erase marker
		CRect	r(arpt[0], arpt[1]);
		r.BottomRight() += vert ? CPoint(1, 0) : CPoint(0, 1);
		RedrawWindow(&r, NULL);	// erase line
		for (int i = 0; i < ARROWS; i++) {
			arrow[i].GetRgnBox(r);	// get arrow's bounding box
			RedrawWindow(&r, NULL);	// erase arrow
		}
	}
	ReleaseDC(pDC);
}

void CHistoryView::MakeArrow(CPoint point, bool left, bool vert, CRgn& rgn)
{
	static const POINT	ArrowHorz[] = {
		{-5, 0}, {0, 5}, {0, -5}
	};
	static const POINT	ArrowVert[] = {
		{0, -5}, {5, 0}, {-5, 0}
	};
	static const int	pts = sizeof(ArrowHorz) / sizeof(POINT);
	POINT	pta[pts];
	int	dir = left ? -1 : 1;
	const POINT	*pArrow = vert ? ArrowVert : ArrowHorz;
	for (int i = 0; i < pts; i++) {
		CPoint	a(pArrow[i]);
		if (vert)
			a.y *= dir;
		else
			a.x *= dir;
		pta[i].x = point.x + a.x;
		pta[i].y = point.y + a.y;
	}
	rgn.CreatePolygonRgn(pta, pts, ALTERNATE);
}

void CHistoryView::SaveUndoState(CUndoState& State)
{
	CFV::UValViewPos(State) = m_ViewPos;
	switch (State.GetCode()) {
	case CFV::UCODE_HISTORY_RENDER:
		break;
	case CFV::UCODE_HISTORY_DELETE:
	case CFV::UCODE_HISTORY_CUT:
	case CFV::UCODE_HISTORY_PASTE:
		if (!CFV::UValFlags(State)) {
			CClipboardUndoState	*cus = new CClipboardUndoState;
			int	items = GetItemCount();
			for (int i = 0; i < items; i++) {
				if (IsSelected(i)) {
					CHistoryItemEx	item;	// extended history item
					CHistoryItem&	bhi = item;	// downcast to base class
					bhi = m_History[i];	// copy base history item
					item.m_OrigPos = i;	// include item's original position
					cus->Add(item);	// add history item to undo state
				}
			}
			State.SetObj(cus);
		}
		break;
	case CFV::UCODE_HISTORY_MOVE:
		if (!CFV::UValFlags(State)) {
			CMoveUndoState *mus = new CMoveUndoState;
			GetSelection(mus->m_SelIdx);
			mus->m_DstPos = m_UndoArg;
			State.SetObj(mus);
		}
		break;
	case CFV::UCODE_EDIT_PALETTE:
		{
			CPaletteUndoState	*pus = new CPaletteUndoState;
			m_View->GetPalette(pus->m_Palette);
			State.SetObj(pus);
		}
		break;
	default:
		if (CFV::UValFlags(State) & CFV::UFLAG_UNDO) {
			ASSERT(GetItemCount() > 0);
			if (GetItemCount() > 0) {	// must have at least one item
				CHistoryItem&	item = m_History[GetItemCount() - 1];
				State.SetObj(item);
			}
		}
	}
}

void CHistoryView::RestoreUndoState(const CUndoState& State)
{
	switch (State.GetCode()) {
	case CFV::UCODE_HISTORY_RENDER:
		RenderItem(CFV::UValViewPos(State), RF_ALLOW_REMAP);
		break;
	case CFV::UCODE_HISTORY_DELETE:
	case CFV::UCODE_HISTORY_CUT:
	case CFV::UCODE_HISTORY_PASTE:
		RestoreClipboardUndoState(State);
		break;
	case CFV::UCODE_HISTORY_MOVE:
		RestoreMoveUndoState(State);
		break;
	case CFV::UCODE_EDIT_PALETTE:
		{
			CPaletteUndoState	*pus = (CPaletteUndoState *)State.GetObj();
			m_View->SetPalette(pus->m_Palette, FALSE);
		}
		break;
	default:
		if (CFV::UValFlags(State) & CFV::UFLAG_UNDO) {	// if undoing
			if (GetItemCount() > 0)
				RemoveItem(GetItemCount() - 1);
			if (State.GetCode() == CFV::UCODE_APPLY_PALETTE) {	// if apply palette
				theApp.GetMain()->GetPaletteDlg().SetModify(TRUE);
				m_View->GetPalette().m_Modified = TRUE;
			} else {	// normal case; render viewed history item
				int	ViewPos = CFV::UValViewPos(State);
				ASSERT(ViewPos < GetItemCount());
				if (ViewPos < GetItemCount())	// validate view position
					RenderItem(ViewPos, RF_ALLOW_REMAP);
			}
		} else {	// redoing
			if (GetItemCount() > 0)
				ShowDetail(GetItemCount() - 1, TRUE);
			CHistoryItem	item;
			item.SetObj((CSnapshot *)State.GetObj());
			ASSERT(!item.IsEmpty());
			if (!item.IsEmpty()) {	// shouldn't be empty unless save failed
				item.m_ShowDetail = FALSE;
				AddItem(item);
				m_ViewPos = -1;	// spoof no-op test in SetViewPos
				RenderItem(GetItemCount() - 1, RF_ALLOW_REMAP);
			}
		}
	}
}

void CHistoryView::RestoreClipboardUndoState(const CUndoState& State)
{
	Deselect();
	bool	insert = (CFV::UValFlags(State) & CFV::UFLAG_UNDO) != 0;
	if (State.GetCode() == CFV::UCODE_HISTORY_PASTE)	// if pasting
		insert ^= 1;	// paste is opposite of cut/delete
	CClipboardUndoState *cus = (CClipboardUndoState *)State.GetObj();
	int	items = cus->GetSize();
	ASSERT(items > 0);
	if (insert) {	// if undoing cut/delete, or redoing paste
		int	FirstPos = (*cus)[0].m_OrigPos;
		if (FirstPos > 0 && FirstPos == GetItemCount())	// if restoring at end
			ShowDetail(FirstPos - 1, TRUE);	// show preceding item's detail rect
		for (int i = 0; i < items; i++) {
			int	pos = (*cus)[i].m_OrigPos;
			InsertItem(pos, (*cus)[i], FALSE);	// defer update
		}
		SetViewPos(CFV::UValViewPos(State));	// restore viewed item position
		UpdateLayout();
		EnsureVisible(FirstPos, items);
	} else {	// redoing cut/delete, or undoing paste
		for (int i = items - 1; i >= 0; i--) {	// delete in reverse order
			int	pos = (*cus)[i].m_OrigPos;
			RemoveItem(pos, TRUE);	// defer update
		}
		UpdateLayout();
	}
}

void CHistoryView::RestoreMoveUndoState(const CUndoState& State)
{
	CMoveUndoState *mus = (CMoveUndoState *)State.GetObj();
	if (CFV::UValFlags(State) & CFV::UFLAG_UNDO) {	// if undoing
		const DWORD	*SelIdx = mus->m_SelIdx.GetData();
		int	sels = INT64TO32(mus->m_SelIdx.GetSize());
		int	DstPos = mus->m_DstPos;
		int	i;	// emulate move's deletion loop
		for (i = 0; i < sels; i++) {	// for each selection index
			int	pos = SelIdx[i];	// get item's original position
			if (pos - i < DstPos)	// if below destination
				DstPos--;	// compensate destination
		}
		Deselect();	// clear selection before copying items
		CHistory	mover;	// backup of moved items
		mover.SetSize(sels);
		for (i = sels - 1; i >= 0; i--)	{	// for each moved item
			int	pos = DstPos + i;
			mover[i] = m_History[pos];	// copy item to backup array
			RemoveItem(pos, TRUE);	// delete item, defer update
		}
		for (i = 0; i < sels; i++) {	// for each moved item
			int	pos = SelIdx[i];	// get item's original position
			mover[i].m_Selected = TRUE;
			if (pos > 0 && pos == GetItemCount())	// if restoring at end
				ShowDetail(pos - 1, TRUE);	// show preceding item's detail rect
			InsertItem(pos, mover[i], TRUE);	// restore item, defer update 
		}
		m_ViewPos = CFV::UValViewPos(State);	// restore viewed item position
		UpdateLayout();
		EnsureVisible(SelIdx[0], SelIdx[sels - 1] - SelIdx[0] + 1);
	} else {	// redoing
		SetSelection(mus->m_SelIdx);	// restore original selection
		MoveSelectedItems(mus->m_DstPos);	// redo move
		EnsureVisible(mus->m_DstPos);
	}
}

void CHistoryView::DumpState() const
{
	static int PrvItemCount, PrvSelCount, PrvSelMark, PrvCurPos, PrvViewPos;
	if (GetItemCount() != PrvItemCount
	|| m_SelCount != PrvSelCount
	|| m_SelMark != PrvSelMark
	|| m_CurPos != PrvCurPos
	|| m_ViewPos != PrvViewPos) {
		_tprintf(_T("items=%d sels=%d mark=%d cur=%d view=%d\n"), 
			GetItemCount(), m_SelCount, m_SelMark, m_CurPos, m_ViewPos);
		PrvItemCount = GetItemCount();
		PrvSelCount = m_SelCount;
		PrvSelMark = m_SelMark;
		PrvCurPos = m_CurPos;
		PrvViewPos = m_ViewPos;
#if DUMP_LEVEL > 1
		DumpHistory();
#endif
	}
}

void CHistoryView::DumpHistory() const
{
	for (int i = 0; i < GetItemCount(); i++) {
		const CSnapshot&	Snap = m_History[i];
		_tprintf(_T("%d: I=%d T=%d S=%dx%d Q=%d A=%d CL=%d CO=%g MP=%d DZ=%d P='%s'\n"),
			i,
			Snap.HasImage() ? Snap.m_Image->GetRefCount() : 0,
			Snap.HasEscTimes() ? Snap.m_EscTimes->GetRefCount() : 0,
			Snap.m_ImageSize.cx, 
			Snap.m_ImageSize.cy, 
			Snap.m_Quality, 
			Snap.m_Antialias, 
			Snap.m_CycleLen,
			Snap.m_ColorOffset,
			Snap.m_MathPrec,
			Snap.m_DeepZoom,
			Snap.m_Palette.m_Name);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CHistoryView drawing

void CHistoryView::OnDraw(CDC* pDC)
{
#if DUMP_LEVEL
	DumpState();
#endif
	CRect	rcb;
	pDC->GetClipBox(rcb);
	int	items = GetItemCount();
	if (items) {
		CDC	mem;
		mem.CreateCompatibleDC(pDC);
		HGDIOBJ	PrevBmp = mem.GetCurrentBitmap();
		pDC->SetROP2(R2_NOT);	// draw detail rectangles as inverted outlines
		pDC->SelectObject(GetStockObject(NULL_BRUSH));
		for (int i = 0; i < items; i++) {	// for each history item
			CPoint	org = GetOrigin(i);
			CRect	ri, rt(org, m_ThumbSize), rs(rt);
			rs.InflateRect(THUMB_MARGIN, THUMB_MARGIN);	// include selection border
			if (ri.IntersectRect(rs, rcb)) {	// if thumbnail intersects clip box
				CHistoryItem&	item = m_History[i];
				CRefBitmap	*thumb = item->m_Thumb;
				if (thumb != NULL) {
					mem.SelectObject(thumb);
					pDC->BitBlt(rt.left, rt.top, rt.Width(), rt.Height(), 
						&mem, 0, 0, SRCCOPY);
					if (!item->m_ThumbDetail.IsRectNull() && item.m_ShowDetail) {
						CRect	r = item->m_ThumbDetail;
						r += org;
						pDC->Rectangle(r);	// draw detail rectangle
					}
					if (i == m_ViewPos) {	// if item is viewed
						CPoint	org(rt.left, rt.bottom);
						org.y -= m_ViewPosIconSize.cy;
						DrawIconEx(pDC->m_hDC, org.x, org.y, m_ViewPosIcon, 
							0, 0, 0, 0, DI_NORMAL);	// draw viewed item icon
					}
					pDC->ExcludeClipRect(rt);	// mask off thumbnail
					if (item.m_Selected) {
						pDC->FillSolidRect(rs, GetSysColor(COLOR_HIGHLIGHT));
						pDC->ExcludeClipRect(rs);	// mask off selection border
					}
				}
			}
		}
		mem.SelectObject(PrevBmp);	// restore previous bitmap
	}
	COLORREF	color = GetSysColor(COLOR_3DFACE);
	pDC->FillSolidRect(rcb, color);	// fill remaining region with background
}

/////////////////////////////////////////////////////////////////////////////
// CHistoryView diagnostics

#ifdef _DEBUG
void CHistoryView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CHistoryView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CHistoryView message map

BEGIN_MESSAGE_MAP(CHistoryView, CScrollView)
	//{{AFX_MSG_MAP(CHistoryView)
	ON_WM_SIZE()
	ON_WM_MOUSEACTIVATE()
	ON_WM_LBUTTONDOWN()
	ON_WM_CREATE()
	ON_WM_CONTEXTMENU()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHistoryView message handlers

BOOL CHistoryView::PreCreateWindow(CREATESTRUCT& cs) 
{
	// override default window class styles CS_HREDRAW and CS_VREDRAW
	// otherwise resizing frame redraws entire view, causing flicker
	cs.lpszClass = AfxRegisterWndClass(	// create our own window class
		CS_DBLCLKS,						// request double-clicks
		theApp.LoadStandardCursor(IDC_ARROW),	// standard cursor
		NULL,									// no background brush
		theApp.LoadIcon(IDR_MAINFRAME));		// app's icon
    ASSERT(cs.lpszClass);
	return CScrollView::PreCreateWindow(cs);
}

int CHistoryView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CScrollView::OnCreate(lpCreateStruct) == -1)
		return -1;

	SetScrollSizes(MM_TEXT, CSize(0, 0));
	m_ScrollTimer.Create(m_hWnd, TID_SCROLL, SCROLL_DELAY, FALSE);	// stopped
	m_ViewPosIcon = (HICON)LoadImage(theApp.m_hInstance, 
		MAKEINTRESOURCE(IDI_HIST_VIEW_POS), IMAGE_ICON, 0, 0, 0);
	ASSERT(m_ViewPosIcon != NULL);
	theApp.GetIconSize(m_ViewPosIcon, m_ViewPosIconSize);
	m_SlideShowMgr.SetView(this, TID_SLIDE_SHOW);

	return 0;
}

void CHistoryView::OnSize(UINT nType, int cx, int cy) 
{
	CScrollView::OnSize(nType, cx, cy);
	UpdateLayout();
}

int CHistoryView::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	// Because the parent of this view is a resizable control bar instead of a
	// frame window, the view MUST override OnMouseActivate, otherwise a mouse
	// click within the view causes a debug assertion, and activation problems
	// occur in release.  The OnMouseActivate override must NOT call the base
	// class, it should simply return MA_ACTIVATE.  Thanks to Naama Goraly for
	// discovering this. Thanks also to M. Shams Mukhtar for figuring out how
	// to host a view on Cristi Posea's resizable control bar without creating
	// a dummy parent frame.
	//
	return MA_ACTIVATE;	// don't call base class, prevents assertion
}

void CHistoryView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	SetFocus();	// else we won't get key down messages
	int	pos = FindItem(point);
	if (pos >= 0) {
		if (nFlags & MK_SHIFT) {	// if shift key down
			SelectSpan(pos);	// select contiguous group
		} else if (nFlags & MK_CONTROL) {	// if control key down
			ToggleSelect(pos);	// toggle item's selection
		} else if (!IsSelected(pos)) {	// if item isn't already selected
			SetSelectionMark(pos);	// start a new selection
		}
		if (m_DragState == DTS_NONE) {
			m_DragState = DTS_TRACK;
			m_DragOrigin = point;
			SetCapture();
			m_DragPos = pos;
		}
	} else	// clicked on background
		Deselect();
	CScrollView::OnLButtonDown(nFlags, point);
}

void CHistoryView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (m_DragState != DTS_NONE) {
		int	DragState = m_DragState;	// copy state; EndDrag resets it
		EndDrag();
		if (DragState == DTS_DRAG) {	// if we were dragging
			int	target = DropHitTest(point);
			switch (target) {
			case DT_RECORD:
				{
					CPoint	pt(point);
					ClientToScreen(&pt);
					CRecordDlg&	RecDlg = theApp.GetMain()->GetRecordDlg();
					int	idx = RecDlg.SnapHitTest(pt);
					AddRecordPoints(idx, FALSE);	// insert into existing script
				}
				break;
			case DT_HISTORY:
				{
					int	DstPos = GetInsertPos(point);
					if (DstPos != m_DragPos) {	// if valid move
						m_UndoArg = DstPos;
						m_View->NotifyUndoableEdit(CFV::UCODE_HISTORY_MOVE);
						MoveSelectedItems(DstPos);
					}
				}
				break;
			case DT_VIEW:
				RenderItem(m_DragPos, RF_UNDOABLE | RF_ALLOW_REMAP);
				break;
			}
		} else {
			if (!(nFlags & (MK_SHIFT | MK_CONTROL))) {	// if no modifier keys
				int	pos = FindItem(point);
				if (pos >= 0)
					SetSelectionMark(pos);	// start a new selection
			}
		}
	}
	CScrollView::OnLButtonUp(nFlags, point);
}

void CHistoryView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	int	pos = FindItem(point);
	if (pos >= 0)
		RenderItem(pos, RF_UNDOABLE | RF_ALLOW_REMAP);
	CScrollView::OnLButtonDblClk(nFlags, point);
}

void CHistoryView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	int	pos = FindItem(point);
	if (pos >= 0) {
		if (!IsSelected(pos))
			SetSelectionMark(pos);
	}
	CScrollView::OnRButtonDown(nFlags, point);
}

void CHistoryView::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	CRect	rc;
	GetClientRect(rc);
	CPoint	pt(point);
	ScreenToClient(&pt);
	if (rc.PtInRect(pt)) {	// if within view
		m_ContextPos = FindItem(pt);
		if (m_ContextPos < 0) {
			if (HaveSelection())
				m_ContextPos = GetSelectionMark();
			else
				m_ContextPos = GetItemCount();
		}
		CMenu	menu, *mp;
		menu.LoadMenu(IDR_HISTORY);
		mp = menu.GetSubMenu(0);
		mp->TrackPopupMenu(0, point.x, point.y, theApp.GetMain());
	} else	// assume point is within a scroll bar
		CScrollView::OnContextMenu(pWnd, point);	// show scrolling menu
}

void CHistoryView::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_DragState == DTS_TRACK	// if tracking and threshold exceeded
	&& (abs(m_DragOrigin.x - point.x) > DRAG_THRESHOLD
	|| abs(m_DragOrigin.y - point.y) > DRAG_THRESHOLD)) {
		m_DragState = DTS_DRAG;	// enter drag mode
	}
	if (m_DragState == DTS_DRAG) {	// if dragging
		int	target = DropHitTest(point);
		if (target) {
			int	curid = m_SelCount > 1 ? IDC_DRAG_MULTI : IDC_DRAG_SINGLE;
			SetCursor(theApp.LoadCursor(curid));
		} else
			SetCursor(LoadCursor(NULL, IDC_NO));
		m_ScrollDelta = 0;	// default action is no scrolling
		switch (target) {
		case DT_HISTORY:
			if (GetItemCount() > GetCountPerPage()) {
				CRect	r;
				GetClientRect(r);
				int	ScrollZoneHeight = m_ThumbSize.cy / 2 + THUMB_MARGIN; 
				r.top += ScrollZoneHeight;
				r.bottom -= ScrollZoneHeight;
				if (point.y < r.top	// if cursor is above top boundary
				&& GetScrollPosition().y)	// and scroll position not at top
					m_ScrollDelta = point.y - r.top;	// start scrolling up
				else if (point.y > r.bottom)	// if cursor is below bottom boundary
					m_ScrollDelta = point.y - r.bottom;	// start scrolling down
			}
			UpdateDropPos(point);
			theApp.GetMain()->GetRecordDlg().EndDrag();
			break;
		case DT_RECORD:
			{
				EraseDropPos();
				CPoint	pt(point);
				ClientToScreen(&pt);
				theApp.GetMain()->GetRecordDlg().DragMove(pt);
			}
			break;
		default:
			EraseDropPos();
			theApp.GetMain()->GetRecordDlg().EndDrag();
		}
		m_ScrollTimer.Run(m_ScrollDelta != 0);	// run scroll timer if needed
	}
	CScrollView::OnMouseMove(nFlags, point);
}

BOOL CHistoryView::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYDOWN) {
		switch (pMsg->wParam) {
		case VK_RETURN:
			Render();	// render selected item(s)
			return(TRUE);
		case VK_ESCAPE:
			if (m_DragState != DTS_NONE) {	// if dragging
				EndDrag();
				return(TRUE);
			}
			break;
		case VK_UP:
		case VK_LEFT:
			Goto(CHistoryView::LC_UP);
			return(TRUE);
		case VK_DOWN:
		case VK_RIGHT:
			Goto(CHistoryView::LC_DOWN);
			return(TRUE);
		case VK_PRIOR:
			Goto(CHistoryView::LC_PAGE_UP);
			return(TRUE);
		case VK_NEXT:
			Goto(CHistoryView::LC_PAGE_DOWN);
			return(TRUE);
		case VK_HOME:
			Goto(CHistoryView::LC_TOP);
			return(TRUE);
		case VK_END:
			Goto(CHistoryView::LC_BOTTOM);
			return(TRUE);
		}
	}
	return CScrollView::PreTranslateMessage(pMsg);
}

BOOL CHistoryView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	if (nFlags & MK_CONTROL) {
		const MSG	*pMsg = GetCurrentMessage();
		return BOOL(m_View->SendMessage(pMsg->message, pMsg->wParam, pMsg->lParam));
	}
	return CScrollView::OnMouseWheel(nFlags, zDelta, pt);
}

void CHistoryView::OnTimer(W64UINT nIDEvent) 
{
	switch (nIDEvent) {
	case TID_SCROLL:
		if (m_ScrollDelta) {
			int	DropPos = m_DropPos;	// copy position; EraseDropPos resets it
			EraseDropPos();	// must erase drop indicator BEFORE scrolling
			CPoint	sp = GetScrollPosition();
			ScrollToPosition(sp + CPoint(0, m_ScrollDelta));
			if (DropPos >= 0) {	// if drop position should be visible
				CPoint	cp;
				GetCursorPos(&cp);
				ScreenToClient(&cp);
				UpdateDropPos(cp);	// draw new drop indicator near cursor
			}
			if (GetScrollPosition() == sp)	// if scroll position didn't change
				m_ScrollDelta = 0;	// assume limit reached; stop scrolling
		}
		break;
	case TID_SLIDE_SHOW:
		m_SlideShowMgr.ShowNextSlide();
		break;
	}
	CScrollView::OnTimer(nIDEvent);
}
