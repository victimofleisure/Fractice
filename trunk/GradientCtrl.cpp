// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		25jan09	add MoveSelection
		02		06jan10	W64: cast SelIdx array size to 32-bit

        gradient control
 
*/

// GradientCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "GradientCtrl.h"
#include "SortArray.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGradientCtrl

IMPLEMENT_DYNAMIC(CGradientCtrl, CStatic);

CGradientCtrl::CGradientCtrl()
{
	m_CurSel = -1;
	m_SelCount = 0;
	m_ShowMarkers = TRUE;
}

CGradientCtrl::~CGradientCtrl()
{
}

void CGradientCtrl::DumpPts() const
{
	int	pts = GetCount();
	_tprintf(_T("count=%d cursel=%d sels=%d\n"), pts, m_CurSel, m_SelCount);
	for (int i = 0; i < pts; i++) {
		GRADIENT_COLOR	c;
		GetColor(i, c);
		_tprintf(_T("%d. sel=%d pos=%.3f r=%.3f g=%.3f b=%.3f\n"),
			i, m_IsSel[i], m_Grad[i].m_Pos, c.rgb.m_r, c.rgb.m_g, c.rgb.m_b);
	}
}

void CGradientCtrl::SetGradient(const CGradient& Grad)
{
	m_CurSel = -1;
	m_SelCount = 0;
	m_Grad.Copy(Grad);
	m_IsSel.SetSize(GetCount());
}

void CGradientCtrl::SetPoint(int PtIdx, const CGradientPt& Pt)
{
	m_Grad[PtIdx] = Pt;
	Invalidate();
}

void CGradientCtrl::SetColor(int PtIdx, const GRADIENT_COLOR& Color)
{
	m_Grad[PtIdx].m_Color = Color;
	Invalidate();
}

void CGradientCtrl::SetColor(const GRADIENT_COLOR& Color)
{
	int	pts = GetCount();
	for (int i = 0; i < pts; i++) {
		if (m_IsSel[i])
			m_Grad[i].m_Color = Color;
	}
	Invalidate();
}

void CGradientCtrl::SetPos(int PtIdx, double Pos)
{
	CGradientPt	pt = m_Grad[PtIdx];
	if (Pos == pt.m_Pos)
		return;	// nothing to do
	CRect	r;
	GetMarkerRect(PtIdx, r);
	m_Grad.RemoveAt(PtIdx);
	m_IsSel.RemoveAt(PtIdx);
	int	idx;
	if (Pos > pt.m_Pos)
		idx = m_Grad.FindPos(Pos, PtIdx);
	else {	// search backwards
		int	pts = m_Grad.GetSize();
		idx = m_Grad.ReverseFindPos(Pos, PtIdx < pts ? PtIdx : pts - 1);
	}
	pt.m_Pos = Pos;
	m_Grad.InsertAt(idx, pt);
	BYTE	sel = TRUE;
	m_IsSel.InsertAt(idx, sel);
	if (m_CurSel == PtIdx)
		m_CurSel = idx;
	if (m_Grad.m_SpaceEvenly) {
		m_Grad.SpaceEvenly();
		InvalidateMarkers();
	} else {
		InvalidateMarkers(&r);	// invalidate previous position
		GetMarkerRect(idx, r);
		InvalidateMarkers(&r);	// invalidate new position
	}
	Invalidate();
	NotifyParent();
}

void CGradientCtrl::SetColorMode(int Mode)
{
	if (Mode == m_Grad.m_ColorMode)
		return;	// nothing to do
	int	pts = GetCount();
	if (Mode == GRADIENT_RGB) {
		for (int i = 0; i < pts; i++) {
			hls2rgb(m_Grad[i].m_Color.hls, m_Grad[i].m_Color.rgb);
		}
	} else {
		for (int i = 0; i < pts; i++) {
			rgb2hls(m_Grad[i].m_Color.rgb, m_Grad[i].m_Color.hls);
		}
	}
	m_Grad.m_ColorMode = Mode;
	Invalidate();
	NotifyParent();
}

void CGradientCtrl::SetSpaceEvenly(bool Enable)
{
	if (Enable == m_Grad.m_SpaceEvenly)
		return;	// nothing to do
	m_Grad.m_SpaceEvenly = Enable;
	UpdateGradient();
}

void CGradientCtrl::UpdateGradient()
{
	if (m_Grad.m_SpaceEvenly)
		m_Grad.SpaceEvenly();
	InvalidateMarkers();
	Invalidate();
	NotifyParent();
}

void CGradientCtrl::GetMarkerRect(int PtIdx, CRect& r) const
{
	GetClientRect(r);
	if (PtIdx >= 0) {
		int	x = round(m_Grad[PtIdx].m_Pos * r.Width());
		r.left = x - MARKER_SIZE;
		r.right = x + MARKER_SIZE + 1;
	} else {	// return entire marker area
		r.left -= MARKER_SIZE;
		r.right += MARKER_SIZE + 1;
	}
	r.top = r.bottom + MARKER_OFFSET;
	r.bottom = r.top + MARKER_SIZE + 1;
}

void CGradientCtrl::DrawMarker(int PtIdx, CDC *pDC)
{
	static const COLORREF SelColor = RGB(255, 255, 255);
	CRect	rc;
	GetClientRect(rc);
	HGDIOBJ	PrevBrush = pDC->SelectObject(GetStockObject(DC_BRUSH));
	CPoint	pt;
	pt.x = round(m_Grad[PtIdx].m_Pos / 1.0 * rc.Width());
	pt.y = rc.bottom + MARKER_OFFSET;
	SetDCBrushColor(*pDC, m_IsSel[PtIdx] ? 
		SelColor : GetSysColor(COLOR_HIGHLIGHT));
	CPoint	pta[3];
	pta[0] = pt;
	pta[1] = CPoint(pt.x + MARKER_SIZE, pt.y + MARKER_SIZE);
	pta[2] = CPoint(pt.x - MARKER_SIZE, pt.y + MARKER_SIZE);
	pDC->Polygon(pta, 3);
	SelectObject(pDC->m_hDC, PrevBrush);
}

void CGradientCtrl::InvalidateMarkers(LPCRECT lpRect, bool Erase)
{
	CRect	rema;
	if (lpRect == NULL) {	// if no rectangle was passed
		GetMarkerRect(rema);	// invalidate entire marker area
		lpRect = &rema;
	}
	CClientDC	dc(this);
	if (Erase)
		dc.FillSolidRect(lpRect, GetSysColor(COLOR_3DFACE));
	if (m_ShowMarkers) {
		int	pts = GetCount();
		for (UINT j = 0; j < 2; j++) {
			for (int i = 0; i < pts; i++) {
				if (m_IsSel[i] == j) {
					CRect	r, rt;
					GetMarkerRect(i, r);
					if (rt.IntersectRect(r, lpRect))
						DrawMarker(i, &dc);
				}
			}
		}
	}
}

void CGradientCtrl::Select(int PtIdx, bool Enable, CDC *pDC)
{
	if (IsSelected(PtIdx) == Enable)
		return;	// nothing to do
	m_IsSel[PtIdx] = Enable;
	if (Enable)
		m_SelCount++;
	else
		m_SelCount--;
	m_CurSel = m_SelCount > 0 ? PtIdx : -1;
	DrawMarker(PtIdx, pDC);
}

void CGradientCtrl::Deselect(CDC *pDC)
{
	int	pts = GetCount();
	for (int i = 0; i < pts; i++) {
		if (m_IsSel[i]) {
			m_IsSel[i] = FALSE;
			DrawMarker(i, pDC);
		}
	}
	m_CurSel = -1;
	m_SelCount = 0;
}

void CGradientCtrl::Select(int PtIdx, bool Enable)
{
	CClientDC	dc(this);
	Select(PtIdx, Enable, &dc);
}

void CGradientCtrl::Deselect()
{
	CClientDC	dc(this);
	Deselect(&dc);
}

void CGradientCtrl::SetCurSel(int PtIdx)
{
	CClientDC	dc(this);
	Deselect(&dc);
	if (PtIdx >= 0)
		Select(PtIdx, TRUE, &dc);
	NotifyParent();
}

void CGradientCtrl::UpdateCurSel(int PtIdx)
{
	if (IsSelected(PtIdx)) {
		m_CurSel = PtIdx;
		NotifyParent();
	} else
		SetCurSel(PtIdx);
}

void CGradientCtrl::ToggleSelect(int PtIdx)
{
	Select(PtIdx, !IsSelected(PtIdx));
	NotifyParent();
}

void CGradientCtrl::SelectSpan(int PtIdx)
{
	if (m_CurSel >= 0) {
		int	start, end;
		if (m_CurSel < PtIdx) {
			start = m_CurSel;
			end = PtIdx;
		} else {
			start = PtIdx;
			end = m_CurSel;
		}
		int	sel = m_CurSel;
		int	pts = GetCount();
		CClientDC	dc(this);
		for (int i = 0; i < pts; i++)
			Select(i, i >= start && i <= end, &dc);
		m_CurSel = sel;
	} else
		SetCurSel(PtIdx);
}

void CGradientCtrl::SelectAll()
{
	int	pts = GetCount();
	for (int i = 0; i < pts; i++)
		Select(i, TRUE);
}

void CGradientCtrl::GetSelection(CDWordArray& SelIdx) const
{
	int pts = GetCount();
	int	sels = GetSelCount();
	SelIdx.SetSize(sels);
	int	j = 0;
	for (int i = 0; i < pts; i++) {
		if (IsSelected(i))
			SelIdx[j++] = i;
	}
}

void CGradientCtrl::SetSelection(const CDWordArray& SelIdx)
{
	Deselect();
	int	sels = INT64TO32(SelIdx.GetSize());
	for (int i = 0; i < sels; i++)
		Select(SelIdx[i], TRUE);
}

void CGradientCtrl::SortByPos()
{
	CSortArray<CSortGradPt, CSortGradPt&>	sa;
	int	pts = GetCount();
	sa.SetSize(pts);
	int	i;
	for (i = 0; i < pts; i++) {	// copy points into sortable array
		CSortGradPt&	spt = sa[i];
		CGradientPt&	pt = spt;
		pt = m_Grad[i];
		spt.m_Idx = i;
		spt.m_IsSel = m_IsSel[i];	// save selection flag
		spt.m_IsCurSel = (i == m_CurSel);	// save current selection
	}
	sa.Sort();	// sort points by position
	for (i = 0; i < pts; i++) {	// copy sortable array into points
		CSortGradPt&	spt = sa[i];
		m_Grad[i] = spt;
		m_IsSel[i] = spt.m_IsSel;	// restore selection flag
		if (spt.m_IsCurSel)	// restore current selection
			m_CurSel = i;
	}
}

void CGradientCtrl::MoveSelection(double Pos)
{
	if (m_CurSel < 0)
		return;
	CDWordArray	SelIdx;
	GetSelection(SelIdx);
	int	sels = INT64TO32(SelIdx.GetSize());
	int	i;
	for (i = 0; i < sels; i++) {	// search selection for selection mark
		if (static_cast<int>(SelIdx[i]) == m_CurSel)
			break;
	}
	if (i >= sels)	// if selection mark wasn't found
		m_CurSel = SelIdx[0];	// move mark to first selected point
	if (sels > 1) {	// if multiple selection
		double	offset = Pos - GetPos(m_CurSel);
		// constrain motion so selected points stay within gradient
		if (offset < 0) {
			double	r = -GetPos(SelIdx[0]);
			offset = max(offset, r);
		} else {
			double	r = 1 - GetPos(SelIdx[sels - 1]);
			offset = min(offset, r);
		}
		if (offset) {
			for (int i = 0; i < sels; i++)	// offset point positions
				m_Grad[SelIdx[i]].m_Pos += offset;
			SortByPos();
			Invalidate();
			NotifyParent();
			InvalidateMarkers();
		}
	} else	// single selection
		SetPos(m_CurSel, Pos);	// update position
}

void CGradientCtrl::Insert(CGradientPt& pt)
{
	int	pts = GetCount();
	if (pts > 1) {
		if (pt.m_Pos < 0) {	// if caller wants us to position point
			if (m_CurSel >= 0) {	// if valid selection
				// position new point between selected point and its neighbor
				int	sel = m_CurSel;
				int	ctr = sel < pts - 1 ? sel + 1 : sel - 1;
				pt.m_Pos = (m_Grad[sel].m_Pos + m_Grad[ctr].m_Pos) / 2;
			} else
				pt.m_Pos = m_Grad[0].m_Pos ? 0 : 1;	// position at start or end
		}
	} else
		pt.m_Pos = pts > 0 ? 1 : 0;
	CClientDC	dc(this);
	Deselect(&dc);
	int	idx = m_Grad.FindPos(pt.m_Pos);
	m_Grad.InsertAt(idx, pt);
	BYTE	sel = FALSE;
	m_IsSel.InsertAt(idx, sel);
	Select(idx, TRUE, &dc);
	UpdateGradient();
}

void CGradientCtrl::Delete()
{
	int	i = 0;
	while (i < GetCount()) {
		if (m_IsSel[i]) {
			m_Grad.RemoveAt(i);
			m_IsSel.RemoveAt(i);
		} else
			i++;
	}
	m_SelCount = 0;
	m_CurSel = -1;
	UpdateGradient();
}

int CGradientCtrl::HitTest(CPoint pt) const
{
	int	pts = GetCount();
	for (int i = 0; i < pts; i++) {
		CRect	r;
		GetMarkerRect(i, r);
		if (r.PtInRect(pt))
			return(i);
	}
	return(-1);
}

void CGradientCtrl::NotifyParent()
{
	NMHDR	nmh;
	nmh.hwndFrom = m_hWnd;
	nmh.idFrom = GetDlgCtrlID();
	nmh.code = GCN_GRADIENTCHANGE;
	GetParent()->SendMessage(WM_NOTIFY, nmh.idFrom, (LPARAM)&nmh);
}

void CGradientCtrl::Interpolate(double Pos, DRGB& Color) const
{
	CGradientIter	iter(m_Grad);
	iter.Interpolate(Pos, Color);
}

void CGradientCtrl::Invert(int PtIdx)
{
	m_Grad[PtIdx].Invert(m_Grad.m_ColorMode == GRADIENT_HLS);
	Invalidate();
}

void CGradientCtrl::RotateHue(int PtIdx, double Rotation)
{
	m_Grad[PtIdx].RotateHue(m_Grad.m_ColorMode == GRADIENT_HLS, Rotation);
	Invalidate();
}

void CGradientCtrl::ShowMarkers(bool Enable)
{
	if (Enable == m_ShowMarkers)
		return;
	m_ShowMarkers = Enable;
	InvalidateMarkers();
}

void CGradientCtrl::RotateGradient(double Offset)
{
	if (Offset < 0)
		Offset = fmod(Offset, 1) + 1;
	int	pts = GetCount();
	for (int i = 0; i < pts; i++)
		m_Grad[i].m_Pos = fmod(m_Grad[i].m_Pos + Offset, 1);
	SortByPos();
	Invalidate();
	InvalidateMarkers();
}

BEGIN_MESSAGE_MAP(CGradientCtrl, CStatic)
	//{{AFX_MSG_MAP(CGradientCtrl)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_ENABLE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGradientCtrl message handlers

void CGradientCtrl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	CRect	rc, rcb;
	GetClientRect(rc);
	dc.GetClipBox(rcb);
	rcb.IntersectRect(rcb, rc);	// limit clip box to client area
	int	pts = GetCount();
	if (pts > 0 && IsWindowEnabled()) {
		DRGB	c;
		CGradientIter	iter(m_Grad);
		for (int x = rcb.left; x < rcb.right; x++) {
			double	pos = double(x) / rc.Width();
			iter.Interpolate(pos, c);
			dc.FillSolidRect(x, rc.top, 1, rc.Height(), c);
		}
	} else
		dc.FillSolidRect(rcb, GetSysColor(COLOR_3DFACE));
}

BOOL CGradientCtrl::OnEraseBkgnd(CDC* pDC) 
{
	return TRUE;
}

void CGradientCtrl::OnEnable(BOOL bEnable)
{
	ShowMarkers(bEnable != 0);
	CStatic::OnEnable(bEnable);
	Invalidate();	// ensure a repaint
}
