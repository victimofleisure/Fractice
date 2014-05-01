// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		11jan09	make queue job checkbox persistent
		02		11feb09	add color cycling
		03		05jul09	change minimum frame count to two
		04		11jul09	allow unlimited record points
		05		03dec09	highlight drop position, add scrolling
		06		17dec09	add select images dialog
		07		22dec09	allow negative color cycling rate
		08		06jan10	W64: make OnTimer 64-bit compatible
		09		06jan10	W64: cast selection array size to 32-bit

        record dialog
 
*/

// RecordDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Fractice.h"
#include "RecordDlg.h"
#include "MainFrm.h"
#include "FracticeDoc.h"
#include "FracticeView.h"
#include "SelectImagesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRecordDlg dialog

IMPLEMENT_DYNAMIC(CRecordDlg, CToolDlg);

#define	RK_DURATION				_T("RecDuration")
#define	RK_DURATION_UNIT		_T("RecDurationUnit")
#define	RK_FRAME_COUNT			_T("RecFrameCount")
#define	RK_FRAME_RATE			_T("RecFrameRate")
#define	RK_QUEUE_JOB			_T("RecQueueJob")

const CCtrlResize::CTRL_LIST CRecordDlg::m_CtrlList[] = {
	{IDC_REC_PARAMS_CTRL,		BIND_TOP | BIND_BOTTOM},
	{IDC_REC_SNAP_LIST,			BIND_ALL},
	{IDC_REC_X_CAP,				BIND_BOTTOM},
	{IDC_REC_X_EDIT,			BIND_BOTTOM},
	{IDC_REC_Y_CAP,				BIND_BOTTOM},
	{IDC_REC_Y_EDIT,			BIND_BOTTOM},
	{IDC_REC_Z_CAP,				BIND_BOTTOM},
	{IDC_REC_Z_EDIT,			BIND_BOTTOM},
	{IDC_REC_CYCLE_COLORS_CHK,	BIND_BOTTOM},
	{IDC_REC_CYCLING_RATE_CAP,	BIND_BOTTOM},
	{IDC_REC_CYCLING_RATE_EDIT,	BIND_BOTTOM},
	{IDC_REC_FRAME_RANGE,		BIND_BOTTOM},
	{IDC_REC_FRAME_RANGE2,		BIND_BOTTOM},
	{IDC_REC_START_FRAME_CAP,	BIND_BOTTOM},
	{IDC_REC_END_FRAME_CAP,		BIND_BOTTOM},
	{IDC_REC_START_FRAME_EDIT,	BIND_BOTTOM},
	{IDC_REC_END_FRAME_EDIT,	BIND_BOTTOM},
	{IDC_REC_TO_BITMAPS,		BIND_BOTTOM},
	{IDC_REC_QUEUE_JOB,			BIND_BOTTOM},
	{IDC_REC_INSERT,			BIND_BOTTOM},
	{IDC_REC_DELETE,			BIND_BOTTOM},
	{IDOK,						BIND_BOTTOM},
	{IDCANCEL,					BIND_BOTTOM},
	{0, 0}	// list terminator
};

CRecordDlg::CRecordDlg(CWnd* pParent /*=NULL*/)
	: CToolDlg(IDD, IDR_MAINFRAME, _T("RecordDlg"), pParent)
{
	//{{AFX_DATA_INIT(CRecordDlg)
	m_FrameRange = 0;
	m_QueueJob = FALSE;
	//}}AFX_DATA_INIT
	m_View = NULL;
	m_CtxMenuSnapIdx = -1;
	ReadRegVars();
	m_CtrlsEnabled = TRUE;
	m_InitRect.SetRectEmpty();
	m_Dragging = FALSE;
	m_QuietListUpdate = FALSE;
	m_ThumbSize = CSize(0, 0);
	m_DropPos = -1;
	m_ScrollDelta = 0;
}

CRecordDlg::~CRecordDlg()
{
	WriteRegVars();
}

void CRecordDlg::ReadRegVars()
{
	m_DefFrameRate	= theApp.RdRegFloat(RK_FRAME_RATE, DEF_FRAME_RATE);
	m_DefDuration	= theApp.RdRegInt(RK_DURATION, DEF_DURATION);
	m_DefFrameCount = theApp.RdRegInt(RK_FRAME_COUNT, DEF_FRAME_COUNT);
	m_DurationUnit	= theApp.RdRegInt(RK_DURATION_UNIT, 0);
	m_QueueJob		= theApp.RdRegInt(RK_QUEUE_JOB, 0);
}

void CRecordDlg::WriteRegVars()
{
	theApp.WrRegFloat(RK_FRAME_RATE, m_DefFrameRate);
	theApp.WrRegInt(RK_DURATION, m_DefDuration);
	theApp.WrRegInt(RK_FRAME_COUNT, m_DefFrameCount);
	theApp.WrRegInt(RK_DURATION_UNIT, m_DurationUnit);
	theApp.WrRegInt(RK_QUEUE_JOB, m_QueueJob);
}

void CRecordDlg::GetDefaults(CRecordInfo& Info) const
{
	Info.m_Duration = m_DefDuration;
	Info.m_FrameCount = m_DefFrameCount;
	Info.m_FrameRate = m_DefFrameRate;
}

void CRecordDlg::SetDefaults(const CRecordInfo& Info)
{
	m_DefDuration = Info.m_Duration;
	m_DefFrameCount = Info.m_FrameCount;
	m_DefFrameRate = Info.m_FrameRate;
}

void CRecordDlg::SetView(CFracticeView *View)
{
	if (View == m_View)
		return;	// nothing to do
	if (m_View != NULL) {	// if we're attached to a view
		ReadCtrls();
		m_View->SetRecordInfo(m_Info);	// store our state to view
	}
	bool	Enable = View != NULL;
	// delay enabling/disabling child controls to avoid flicker
	if (!Enable || m_View == NULL)	// if enable changed
		PostMessage(UWM_ENABLECTRLS, Enable);	// use post for delay
	m_View = View;	// order matters; InitCtrls uses m_View
	if (Enable) {
		if (View->GetImageSize().cx) {	// if view is fully initialized
			View->GetRecordInfo(m_Info);	// load our state from view
			InitCtrls();
		}
	} else {	// no view
		CRecordInfo	def;
		m_Info = def;	// set default info
		InitCtrls();
	}
}

void CRecordDlg::GetInfo(CRecordInfo& Info)
{
	ReadCtrls();
	Info = m_Info;
}

void CRecordDlg::SetInfo(const CRecordInfo& Info)
{
	m_Info = Info;
	if (!m_Info.m_FrameCount)
		GetDefaults(m_Info);
	InitCtrls();
}

void CRecordDlg::SecsToTime(int Secs, CString& Time)
{
	Time.Format(_T("%d:%02d:%02d"), Secs / 3600, Secs % 3600 / 60, Secs % 60);
}

int CRecordDlg::TimeToSecs(LPCTSTR Time)
{
	static const int PLACES = 3;	// hours, minutes, seconds
	int	ip[PLACES], op[PLACES];	// input and output place arrays
	ZeroMemory(op, sizeof(op));
	int	ps = _stscanf(Time, _T("%d%*[: ]%d%*[: ]%d"), &ip[0], &ip[1], &ip[2]);
	if (ps >= 0)
		CopyMemory(&op[PLACES - ps], ip, ps * sizeof(int));
	return(op[0] * 3600 + op[1] * 60 + op[2]);
}

bool CRecordDlg::UpdateUI()
{
	CString	s;
	if (!UpdateData(TRUE))	// retrieve data from radio buttons
		return(FALSE);
	// if duration or frame rate changed, update duration controls
	float	FrameRate = static_cast<float>(m_FrameRateEdit.GetVal());
	if (m_DurationEdit.GetModify() || FrameRate != m_FrameRate) {
		m_DurationEdit.GetWindowText(s);
		m_FrameRate = FrameRate;
		if (m_DurationUnit == DU_SECONDS) {	// if unit is seconds
			m_Duration = TimeToSecs(s);	// assume s contains hh:mm:ss duration
			m_FrameCount = SecsToFrames(m_Duration);	// convert to frames
		} else {	// unit is frames
			m_FrameCount = _ttoi(s);	// assume s contains integer frame count
			m_Duration = FramesToSecs(m_FrameCount);	// convert to seconds
		}
	}
	if (m_DurationUnit == DU_SECONDS)	// if unit is seconds
		SecsToTime(m_Duration, s);	// display hh:mm:ss duration
	else	// unit is frames
		s.Format(_T("%d"), m_FrameCount);	// display integer frame count
	m_DurationEdit.SetWindowText(s);
	if (!m_FrameRange) {
		m_StartFrameEdit.SetVal(0);
		m_EndFrameEdit.SetVal(max(0, m_FrameCount - 1));
	}
	m_StartFrameEdit.EnableWindow(m_CtrlsEnabled && m_FrameRange);
	m_EndFrameEdit.EnableWindow(m_CtrlsEnabled && m_FrameRange);
	m_CyclingRateEdit.EnableWindow(m_CtrlsEnabled && m_CycleColorsChk.GetCheck());
	return(TRUE);
}

inline void CRecordDlg::SetNum(CWnd& Wnd, const BigNum& Num)
{
	Wnd.SetWindowText(Num.ToStr(CFracticeView::COORDS_PRECISION));
}

void CRecordDlg::SetCoords(const CSnapshot *pSnap)
{
	if (pSnap != NULL) {
		SetNum(m_XEdit, pSnap->m_Origin.x);
		SetNum(m_YEdit, pSnap->m_Origin.y);
		SetNum(m_ZEdit, pSnap->m_Zoom);
	} else {
		CString	s;
		m_XEdit.SetWindowText(s);
		m_YEdit.SetWindowText(s);
		m_ZEdit.SetWindowText(s);
	}
}

void CRecordDlg::InitSnapList()
{
	m_SnapList.DeleteAllItems();	// remove any existing selection
	if (m_View != NULL && m_View->GetThumbSize() != m_ThumbSize) {
		CSize	ThumbSize = m_View->GetThumbSize();
		m_ThumbSize = ThumbSize;
		m_ImgList.DeleteImageList();
		m_ImgList.Create(ThumbSize.cx, ThumbSize.cy, ILC_COLORDDB, 0, 0);
		m_SnapList.SetImageList(&m_ImgList, LVSIL_NORMAL);
		// set spacing between icons; use smallest value that doesn't
		// cause unexpected behavior when navigating via cursor keys
		static const int MIN_GUTTER = 16;	// seems to be ok
		LOGFONT	font;
		m_SnapList.GetFont()->GetLogFont(&font);
		ThumbSize.cx += MIN_GUTTER;
		ThumbSize.cy += MIN_GUTTER + abs(font.lfHeight);	// add font height
		m_SnapList.SetIconSpacing(ThumbSize.cx, ThumbSize.cy);
	}
	int	snaps = GetSnapCount();
	m_SnapList.SetItemCountEx(snaps, 0);
	m_ImgList.SetImageCount(snaps);
	m_SnapImgIdx.SetSize(snaps);
	if (snaps) {
		for (int i = 0; i < snaps; i++) {
			m_ImgList.Replace(i, m_Info.m_Snap[i].m_Thumb, NULL);
			m_SnapImgIdx[i] = i;
		}
		const CSnapshot&	FirstSnap = m_Info.m_Snap[0];
		SetCoords(&FirstSnap);
		m_ParamsCtrl.SetSnapshot(FirstSnap);	// update control
		m_SnapList.Scroll(CSize(INT_MIN, INT_MIN));	// reset scroll position
	} else {	// no record points
		SetCoords(NULL);
		if (!m_ParamsCtrl.IsEmpty())	// if control isn't already blank
			m_ParamsCtrl.SetEmpty();	// blank it
	}
	m_ParamsCtrl.EnableWindow(m_CtrlsEnabled && snaps);
	m_DeleteBtn.EnableWindow(FALSE);	// no items selected
}

void CRecordDlg::InitCtrls()
{
	InitSnapList();	// init snapshot list and parameters control
	// set duration and frame rate
	m_FrameRateEdit.SetVal(m_Info.m_FrameRate);
	m_FrameRate = m_Info.m_FrameRate;
	if (m_DurationUnit == DU_SECONDS) {	// if unit is seconds
		m_Duration = m_Info.m_Duration;
		m_FrameCount = SecsToFrames(m_Info.m_Duration);	// convert to frames
	} else {	// unit is frames
		m_Duration = FramesToSecs(m_Info.m_FrameCount);	// convert to seconds
		m_FrameCount = m_Info.m_FrameCount;
	}
	// set remaining controls and update dialog data
	UINT	start = m_Info.m_StartFrame;
	UINT	end = m_Info.m_EndFrame;
	m_StartFrameEdit.SetVal(start);
	m_EndFrameEdit.SetVal(end);
	m_FrameRange = (start > 0 || (end > 0 && end < m_Info.m_FrameCount - 1));
	m_ToBitmapsChk.SetCheck(m_Info.m_ToBitmaps);
	m_CycleColorsChk.SetCheck(m_Info.m_CycleColors);
	m_CyclingRateEdit.SetVal(m_Info.m_ColorCycleRate);
	UpdateData(FALSE);	// init controls
	UpdateUI();
}

void CRecordDlg::CopyParams(CSnapshot& Dst, const CSnapshot& Src)
{
	BigPoint	Origin(Dst.m_Origin);
	BigNum	Zoom(Dst.m_Zoom);
	CBitmapPtr	Thumb(Dst.m_Thumb);
	Dst = Src;	// copy parameters and everything else too
	Dst.m_Origin = Origin;	// restore origin, zoom, and thumbnail
	Dst.m_Zoom = Zoom;
	Dst.m_Thumb = Thumb;
}

void CRecordDlg::ReadCtrls()
{
	CString	s;
	m_DurationEdit.GetWindowText(s);
	m_Info.m_Duration = m_Duration;
	m_Info.m_FrameCount = m_FrameCount;
	m_Info.m_StartFrame = m_StartFrameEdit.GetIntVal();
	m_Info.m_EndFrame = m_EndFrameEdit.GetIntVal();
	m_Info.m_FrameRate = static_cast<float>(m_FrameRateEdit.GetVal());
	m_Info.m_ToBitmaps = m_ToBitmapsChk.GetCheck() != 0;
	m_Info.m_CycleColors = m_CycleColorsChk.GetCheck() != 0;
	m_Info.m_ColorCycleRate = m_CyclingRateEdit.GetVal();
	if (GetSnapCount()) {
		CSnapshot	snap;
		m_ParamsCtrl.GetSnapshot(snap);	// retrieve parameters from control
		CopyParams(m_Info.m_Snap[0], snap);	// copy parameters to first snapshot
	}
}

void CRecordDlg::EnableCtrls(bool Enable)
{
	if (Enable == m_CtrlsEnabled)
		return;	// nothing to do
	theApp.EnableChildWindows(*this, Enable);
	// parameters control is also disabled by no record points
	m_ParamsCtrl.EnableWindow(Enable && GetSnapCount() > 0);
	// start and end frame are also disabled by frame range == all
	m_StartFrameEdit.EnableWindow(Enable && m_FrameRange);
	m_EndFrameEdit.EnableWindow(Enable && m_FrameRange);
	m_CtrlsEnabled = Enable;
}

void CRecordDlg::Show()
{
	if (IsIconic())
		ShowWindow(SW_RESTORE);
	else {
		if (IsWindowVisible())
			SetFocus();
		else
			ShowWindow(SW_SHOW);
	}
}

int CRecordDlg::SnapHitTest(CPoint point) const
{
	m_SnapList.ScreenToClient(&point);
	return(m_SnapList.HitTest(point));
}

void CRecordDlg::Select(int SnapIdx, bool Enable)
{
	m_SnapList.SetItemState(SnapIdx, Enable ? LVIS_SELECTED : 0, LVIS_SELECTED);
}

void CRecordDlg::SelectAll(bool Enable)
{
	int	snaps = GetSnapCount();
	for (int i = 0; i < snaps; i++)
		Select(i, Enable);
}

void CRecordDlg::GetSelection(CDWordArray& Selection)
{
	POSITION	pos = m_SnapList.GetFirstSelectedItemPosition();
	while (pos != NULL)
		Selection.Add(m_SnapList.GetNextSelectedItem(pos));
}

void CRecordDlg::InsertSnap(int SnapIdx, CSnapshot& Snap)
{
	ASSERT(SnapIdx >= 0 && SnapIdx <= GetSnapCount());
	if (!m_QuietListUpdate) {
		if (!GetSnapCount()) {
			m_ParamsCtrl.SetSnapshot(Snap);	// update parameters control
			m_ParamsCtrl.EnableWindow(TRUE);
		}
		SetCoords(&Snap);
	}
	m_Info.m_Snap.InsertAt(SnapIdx, Snap);
	CSnapshot&	NewSnap = m_Info.m_Snap[SnapIdx];
	NewSnap.m_Image.SetEmpty();	// unlink from image and escape times
	NewSnap.m_EscTimes.SetEmpty();
	int	ImgIdx = m_ImgList.Add(Snap.m_Thumb, COLORREF(0));
	m_SnapImgIdx.InsertAt(SnapIdx, ImgIdx);
	int	snaps = GetSnapCount();
	m_SnapList.SetItemCountEx(snaps, 0);
}

void CRecordDlg::DeleteSnap(int SnapIdx)
{
	ASSERT(SnapIdx >= 0 && SnapIdx < GetSnapCount());
	m_Info.m_Snap.RemoveAt(SnapIdx);	// delete from snapshot array
	int	ImgIdx = m_SnapImgIdx[SnapIdx];
	m_ImgList.Remove(ImgIdx);	// delete from image list
	m_SnapImgIdx.RemoveAt(SnapIdx);	// delete from image index
	int	snaps = GetSnapCount();
	m_SnapList.SetItemCountEx(snaps, 0);	// update list control
	for (int i = 0; i < snaps; i++) {	// fix any image indices above deletion
		if (int(m_SnapImgIdx[i]) > ImgIdx)
			m_SnapImgIdx[i]--;
	}
	if (!m_QuietListUpdate) {
		if (GetSnapCount()) {
			int	pos = m_SnapList.GetSelectionMark();
			SetCoords(pos >= 0 ? &m_Info.m_Snap[pos] : NULL);
		} else {
			SetCoords(NULL);
			m_ParamsCtrl.SetEmpty();
			m_ParamsCtrl.EnableWindow(FALSE);
		}
	}
}

void CRecordDlg::InsertSnaps(int SnapIdx, CSnapshotArray& SnapList)
{
	SelectAll(FALSE);	// deselect
	int	InsPos = SnapIdx >= 0 ? SnapIdx : GetSnapCount();
	bool	DiffWarn = TRUE;
	bool	UpdateParams = !m_QuietListUpdate;
	CSnapshot	ParmSnap;
	m_ParamsCtrl.GetSnapshot(ParmSnap);
	int	snaps = SnapList.GetSize();
	for (int i = 0; i < snaps; i++) {
		CSnapshot&	snap = SnapList[i];
		InsertSnap(InsPos, snap);
		Select(InsPos, TRUE);
		if (UpdateParams) {
			ParmSnap.m_Origin = snap.m_Origin;	// compare excluding coordinates
			ParmSnap.m_Zoom = snap.m_Zoom;
			if (snap != ParmSnap) {	// if new parameters differ
				if (!ParmSnap.IsEmpty()) {	// if old parameters aren't empty
					if (DiffWarn) {
						DiffWarn = FALSE;	// only one warning
						m_SnapList.EnsureVisible(InsPos, TRUE);
						UINT	style = MB_YESNO | MB_ICONQUESTION;
						if (AfxMessageBox(IDS_REC_PARAMS_DIFFER, style) == IDNO) {
							UpdateParams = FALSE;	// user likes old parameters
							continue;	// skip parameter update for rest of loop
						}
					}
				}
				ParmSnap = snap;
			}
		}
		InsPos++;
	}
	m_SnapList.EnsureVisible(SnapIdx, TRUE);
	if (UpdateParams)
		m_ParamsCtrl.SetSnapshot(ParmSnap);
}

bool CRecordDlg::InsertSnaps()
{
	if (m_View == NULL)
		return(FALSE);
	CHistoryView	*hist = m_View->GetHistoryView();
	CImageList	ImgList;
	ImgList.Create(m_ThumbSize.cx, m_ThumbSize.cy, ILC_COLORDDB, 0, 0);
	int	items = hist->GetItemCount();
	int	i;
	for (i = 0; i < items; i++)
		ImgList.Add(hist->GetItem(i)->m_Thumb, COLORREF(0));
	CString	Caption((LPCTSTR)IDS_REC_SELECT_POINTS);
	CSelectImagesDlg	dlg(ImgList, Caption, _T("RecInsSnapsDlg"));
	if (dlg.DoModal() != IDOK)
		return(FALSE);
	CDWordArray	SelIdx;
	dlg.GetSelection(SelIdx);
	CSnapshotArray	SelSnap;
	int	sels = INT64TO32(SelIdx.GetSize());
	SelSnap.SetSize(sels);
	for (i = 0; i < sels; i++)
		SelSnap[i] = hist->GetItem(SelIdx[i]);
	int	InsPos = m_SnapList.GetSelectedCount() ? 
		m_SnapList.GetSelectionMark() : m_SnapList.GetItemCount();
	InsertSnaps(InsPos, SelSnap);
	return(TRUE);
}

void CRecordDlg::RemoveAllSnaps()
{
	m_Info.m_Snap.RemoveAll();
	m_ImgList.SetImageCount(0);
	m_SnapImgIdx.SetSize(0);
	m_SnapList.DeleteAllItems();
	m_ParamsCtrl.SetEmpty();
}

void CRecordDlg::SetSnaps(CSnapshotArray& SnapList)
{
	RemoveAllSnaps();
	InsertSnaps(0, SnapList);
}

void CRecordDlg::DeleteSelectedItems()
{
	CDWordArray	sel;
	GetSelection(sel);
	int	sels = INT64TO32(sel.GetSize());
	if (sels) {
		SelectAll(FALSE);	// deselect
		// delete in reverse order to keep selection indices valid
		for (int i = sels - 1; i >= 0; i--)
			DeleteSnap(sel[i]);
	}
}

void CRecordDlg::MoveSelectedItems(int DstPos)
{
	CDWordArray	sel;
	GetSelection(sel);
	int	sels = INT64TO32(sel.GetSize());
	if (sels) {
		CSnapshotArray	snap;
		for (int i = 0; i < sels; i++) {
			int	pos = sel[i];
			snap.Add(m_Info.m_Snap[pos]);
			if (pos < DstPos)
				DstPos--;
		}
		m_QuietListUpdate = TRUE;	// suppress list update side effects
		DeleteSelectedItems();
		if (DstPos < 0 || DstPos > GetSnapCount())
			DstPos = GetSnapCount();
		InsertSnaps(DstPos, snap);
		m_QuietListUpdate = FALSE;
	}
}

void CRecordDlg::EndDrag()
{
	m_ScrollTimer.Run(FALSE);
	EraseDropPos();
	if (m_Dragging) {
		ReleaseCapture();
		m_Dragging = FALSE;
	}
}

void CRecordDlg::DragMove(CPoint point)
{
	UpdateDropPos(point);
	CRect	r;
	m_SnapList.GetClientRect(r);
	CPoint	pt(point);
	m_SnapList.ScreenToClient(&pt);
	m_ScrollDelta = 0;	// default action is no scrolling
	if (r.PtInRect(pt)) {	// if point within list control
		int	ScrollZoneHeight = m_ThumbSize.cy / 2; 
		r.top += ScrollZoneHeight;
		r.bottom -= ScrollZoneHeight;
		if (pt.y < r.top)	// if cursor is above top boundary
			m_ScrollDelta = pt.y - r.top;	// start scrolling up
		else if (pt.y > r.bottom)	// if cursor is below bottom boundary
			m_ScrollDelta = pt.y - r.bottom;	// start scrolling down
	}
	m_ScrollTimer.Run(m_ScrollDelta != 0);	// run scroll timer if needed
}

void CRecordDlg::UpdateDropPos(CPoint point)
{
	int	DstPos = SnapHitTest(point);
	if (DstPos != m_DropPos) {
		EraseDropPos();
		if (DstPos >= 0) {	// if over a list item
			m_SnapList.SetItemState(DstPos, LVIS_DROPHILITED, LVIS_DROPHILITED);
			m_DropPos = DstPos;	// save new drop position
		}
	}
}

void CRecordDlg::EraseDropPos()
{
	if (m_DropPos >= 0) {
		m_SnapList.SetItemState(m_DropPos, 0, LVIS_DROPHILITED);
		m_DropPos = -1;
	}
}

void CRecordDlg::UpdateDragCursor(CPoint Point)
{
	if (ChildWindowFromPoint(Point) == &m_SnapList) {
		int	ResID = m_SnapList.GetSelectedCount() > 1 ? 
			IDC_DRAG_MULTI : IDC_DRAG_SINGLE;
		SetCursor(AfxGetApp()->LoadCursor(ResID));
	} else
	 	SetCursor(LoadCursor(NULL, IDC_NO));
}

void CRecordDlg::DoDataExchange(CDataExchange* pDX)
{
	CToolDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRecordDlg)
	DDX_Control(pDX, IDC_REC_DELETE, m_DeleteBtn);
	DDX_Control(pDX, IDC_REC_SNAP_LIST, m_SnapList);
	DDX_Control(pDX, IDC_REC_CYCLING_RATE_EDIT, m_CyclingRateEdit);
	DDX_Control(pDX, IDC_REC_CYCLE_COLORS_CHK, m_CycleColorsChk);
	DDX_Control(pDX, IDC_REC_PARAMS_CTRL, m_ParamsCtrl);
	DDX_Control(pDX, IDC_REC_TO_BITMAPS, m_ToBitmapsChk);
	DDX_Control(pDX, IDC_REC_START_FRAME_EDIT, m_StartFrameEdit);
	DDX_Control(pDX, IDC_REC_END_FRAME_EDIT, m_EndFrameEdit);
	DDX_Control(pDX, IDC_REC_FRAME_RATE_EDIT, m_FrameRateEdit);
	DDX_Control(pDX, IDC_REC_DURATION_EDIT, m_DurationEdit);
	DDX_Radio(pDX, IDC_REC_DURATION_UNIT, m_DurationUnit);
	DDX_Radio(pDX, IDC_REC_FRAME_RANGE, m_FrameRange);
	DDX_Check(pDX, IDC_REC_QUEUE_JOB, m_QueueJob);
	DDX_Control(pDX, IDC_REC_X_EDIT, m_XEdit);
	DDX_Control(pDX, IDC_REC_Z_EDIT, m_ZEdit);
	DDX_Control(pDX, IDC_REC_Y_EDIT, m_YEdit);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CRecordDlg, CToolDlg)
	//{{AFX_MSG_MAP(CRecordDlg)
	ON_BN_CLICKED(IDC_REC_DURATION_UNIT, OnDurationUnit)
	ON_EN_KILLFOCUS(IDC_REC_DURATION_EDIT, OnKillfocusDuration)
	ON_NOTIFY(NEN_CHANGED, IDC_REC_FRAME_RATE_EDIT, OnChangedFrameRate)
	ON_BN_CLICKED(IDC_REC_FRAME_RANGE2, OnFrameRange)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_SNAP_INSERT, OnSnapInsert)
	ON_COMMAND(ID_SNAP_DELETE, OnSnapDelete)
	ON_COMMAND(ID_SNAP_PROPERTIES, OnSnapProperties)
	ON_COMMAND(ID_SNAP_SELECT_ALL, OnSelectAll)
	ON_UPDATE_COMMAND_UI(ID_SNAP_DELETE, OnUpdateSnapDelete)
	ON_UPDATE_COMMAND_UI(ID_SNAP_INSERT, OnUpdateSnapInsert)
	ON_UPDATE_COMMAND_UI(ID_SNAP_SELECT_ALL, OnUpdateSelectAll)
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_NOTIFY(LVN_GETDISPINFO, IDC_REC_SNAP_LIST, OnGetdispinfoSnapList)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_REC_SNAP_LIST, OnItemchangedSnapList)
	ON_NOTIFY(LVN_BEGINDRAG, IDC_REC_SNAP_LIST, OnBegindragSnapList)
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	ON_UPDATE_COMMAND_UI(ID_SNAP_PROPERTIES, OnUpdateSnapDelete)
	ON_BN_CLICKED(IDC_REC_DURATION_UNIT2, OnDurationUnit)
	ON_BN_CLICKED(IDC_REC_FRAME_RANGE, OnFrameRange)
	ON_BN_CLICKED(IDC_REC_CYCLE_COLORS_CHK, OnFrameRange)
	ON_BN_CLICKED(IDC_REC_INSERT, OnSnapInsert)
	ON_BN_CLICKED(IDC_REC_DELETE, OnSnapDelete)
	//}}AFX_MSG_MAP
	ON_MESSAGE(UWM_ENABLECTRLS, OnEnableCtrls)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRecordDlg message handlers

BOOL CRecordDlg::OnInitDialog() 
{
	CToolDlg::OnInitDialog();

	// init resizing stuff
	SetIcon(theApp.LoadIcon(IDR_MAINFRAME), FALSE);
	GetWindowRect(m_InitRect);
	m_Resize.AddControlList(this, m_CtrlList);
	// init numeric edit controls
	m_FrameRateEdit.SetPrecision(2);
	m_FrameRateEdit.SetRange(0, SHRT_MAX);
	m_StartFrameEdit.SetPrecision(0);
	m_EndFrameEdit.SetPrecision(0);
	m_ParamsCtrl.Init();
	m_CyclingRateEdit.SetRange(SHRT_MIN, SHRT_MAX);
	// init scroll timer
	m_ScrollTimer.Create(m_hWnd, SCROLL_TIMER, SCROLL_DELAY, FALSE);	// stopped

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CRecordDlg::OnOK()
{
	if (!UpdateUI())
		return;
	if (GetSnapCount() < MIN_POINTS) {
		AfxMessageBox(IDS_REC_TOO_FEW_POINTS);
		return;
	}
	if (m_FrameRate <= 0) {
		AfxMessageBox(IDS_REC_BAD_FRAME_RATE);
		GotoDlgCtrl(&m_FrameRateEdit);
		return;
	}
	if (m_FrameCount < MIN_FRAMES) {
		AfxMessageBox(IDS_REC_BAD_DURATION);
		GotoDlgCtrl(GetDlgItem(IDC_REC_DURATION_EDIT));
		return;
	}
	if (m_CycleColorsChk.GetCheck() && !m_CyclingRateEdit.GetVal()) {
		AfxMessageBox(IDS_REC_BAD_COLOR_CYCLE_RATE);
		GotoDlgCtrl(&m_CyclingRateEdit);
		return;
	}
	CSnapshotArray&	snap = m_Info.m_Snap;
	if (GetSnapCount() == 2 && snap[0].m_Origin == snap[1].m_Origin
	&& snap[0].m_Zoom == snap[1].m_Zoom) {
		if (AfxMessageBox(IDS_REC_SAME_COORDS, MB_OKCANCEL) == IDCANCEL)
			return;
	}
	if (m_FrameRange) {
		int	Start = m_StartFrameEdit.GetIntVal();
		int	End = m_EndFrameEdit.GetIntVal();
		if (Start < 0 || End >= int(m_FrameCount) || Start > End) {
			AfxMessageBox(IDS_REC_BAD_FRAME_RANGE);
			GotoDlgCtrl(&m_StartFrameEdit);
			return;
		}
	}
	ReadCtrls();
	SetDefaults(m_Info);
	CRecorder&	Recorder = theApp.GetMain()->GetRecorder();
	if (Recorder.Record(m_Info))	// if recording starts successully
		CToolDlg::OnOK();	// hide dialog
}

void CRecordDlg::OnCancel() 
{
	if (!UpdateData())
		return;
	ReadCtrls();
	CToolDlg::OnCancel();
}

LRESULT	CRecordDlg::OnEnableCtrls(WPARAM wParam, LPARAM lParam)
{
	EnableCtrls(wParam != 0);
	m_DeleteBtn.EnableWindow(m_SnapList.GetSelectedCount());
	return 0;
}

void CRecordDlg::OnDurationUnit() 
{
	UpdateUI();
}

void CRecordDlg::OnKillfocusDuration() 
{
	UpdateUI();
}

void CRecordDlg::OnChangedFrameRate(NMHDR* pNMHDR, LRESULT* pResult)
{
	UpdateUI();
	*pResult = 0;
}

void CRecordDlg::OnFrameRange() 
{
	UpdateUI();
}

void CRecordDlg::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	CRect	r;
	m_SnapList.GetClientRect(r);
	CPoint	pt(point);
	m_SnapList.ScreenToClient(&pt);
	if (r.PtInRect(pt)) {	// if within client area
		m_CtxMenuSnapIdx = SnapHitTest(point);
		CMenu	menu, *mp;
		menu.LoadMenu(IDR_RECORD_CTX);
		mp = menu.GetSubMenu(0);
		UpdateMenu(this, mp);
		mp->TrackPopupMenu(0, point.x, point.y, this);
	} else
		CToolDlg::OnContextMenu(pWnd, point);
}

void CRecordDlg::OnSnapInsert() 
{
	InsertSnaps();
}
void CRecordDlg::OnSnapDelete()
{
	DeleteSelectedItems();
}

void CRecordDlg::OnSelectAll()
{
	SelectAll(TRUE);
}

void CRecordDlg::OnSnapProperties()
{
	CDWordArray	sel;
	GetSelection(sel);
	int	sels = INT64TO32(sel.GetSize());
	for (int i = 0; i < sels; i++)
		CFracticeView::ShowProperties(m_Info.m_Snap[sel[i]]);
}

void CRecordDlg::OnUpdateSnapInsert(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_View != NULL);
}

void CRecordDlg::OnUpdateSnapDelete(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_CtxMenuSnapIdx >= 0);
}

void CRecordDlg::OnUpdateSelectAll(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetSnapCount() > 0);
}

void CRecordDlg::OnSize(UINT nType, int cx, int cy) 
{
	CToolDlg::OnSize(nType, cx, cy);
	m_Resize.OnSize();
}

void CRecordDlg::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	if (!m_InitRect.IsRectNull()) {	// wait until rect is initialized
		lpMMI->ptMinTrackSize.x = m_InitRect.Width();
		lpMMI->ptMinTrackSize.y = m_InitRect.Height();
	}
}

void CRecordDlg::OnGetdispinfoSnapList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM	*pItem= &pDispInfo->item;
	if (pItem->mask & LVIF_TEXT) {
        _stprintf(pItem->pszText, _T("%d"), pItem->iItem + 1);
	}
	if (pItem->mask & LVIF_IMAGE) {
		pItem->iImage = m_SnapImgIdx[pItem->iItem];
	}
	*pResult = 0;
}

void CRecordDlg::OnItemchangedSnapList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NMLISTVIEW* pNMLV = (NMLISTVIEW*)pNMHDR;
	if (pNMLV->uChanged & LVIF_STATE) {	// if state changed
		if ((pNMLV->uNewState & LVIS_SELECTED) && pNMLV->iItem >= 0)
			SetCoords(&m_Info.m_Snap[pNMLV->iItem]);
		m_DeleteBtn.EnableWindow(m_SnapList.GetSelectedCount());
	}
	*pResult = 0;
}

void CRecordDlg::OnBegindragSnapList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NMLISTVIEW* pNMLV = (NMLISTVIEW*)pNMHDR;
	m_Dragging = TRUE;
	CPoint	pt(pNMLV->ptAction);
	m_SnapList.MapWindowPoints(this, &pt, 1);
	UpdateDragCursor(pt);
	SetCapture();
	*pResult = 0;
}

void CRecordDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_Dragging) {
		UpdateDragCursor(point);
		CPoint	pt(point);
		ClientToScreen(&pt);
		DragMove(pt);
	}
	CToolDlg::OnMouseMove(nFlags, point);
}

void CRecordDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (m_Dragging) {
		EndDrag();
		if (ChildWindowFromPoint(point) == &m_SnapList) {	// if over list
			CPoint	pt(point);
			ClientToScreen(&pt);
			int	DstPos = SnapHitTest(pt);
			if (DstPos < 0)	// if not over a list item
				DstPos = GetSnapCount();	// append
			MoveSelectedItems(DstPos);
		}
	}
	CToolDlg::OnLButtonUp(nFlags, point);
}

void CRecordDlg::OnTimer(W64UINT nIDEvent) 
{
	if (m_ScrollDelta) {
		m_SnapList.Scroll(CSize(0, m_ScrollDelta));
		CPoint	cp;
		GetCursorPos(&cp);
		UpdateDropPos(cp);
	}
	CToolDlg::OnTimer(nIDEvent);
}

BOOL CRecordDlg::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST) {
		if (m_Dragging) {
			if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
				EndDrag();
			return(TRUE);	// override normal keyboard behavior
		}
	}
	return CToolDlg::PreTranslateMessage(pMsg);
}
