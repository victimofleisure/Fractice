// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		25jan09	implement dragging multiple selection

        gradient dialog
 
*/

// GradientDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "GradientDlg.h"
#include <afxpriv.h>	// for WM_KICKIDLE
#include "RectTrackerEx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGradientDlg dialog

IMPLEMENT_DYNAMIC(CGradientDlg, CColorPickerDlg);

CGradientDlg::CGradientDlg(int ResID, CWnd* pParent)
	: CColorPickerDlg(ResID, pParent)
{
	//{{AFX_DATA_INIT(CGradientDlg)
	m_ColorMode = 0;
	//}}AFX_DATA_INIT
	m_DragState = DTS_NONE;
	m_DragOrigin = CPoint(0, 0);
	m_PickEnable = TRUE;
	m_RotPrevPos = 0;
	m_HueRotAmt = 1.0 / 6;	// sixty degrees
}

void CGradientDlg::SetGradient(const CGradient& Grad)
{
	m_Grad.SetGradient(Grad);
	if (m_hWnd) {
		m_Grad.Invalidate();
		m_Grad.InvalidateMarkers();
	}
}

void CGradientDlg::GetTriplet(GRADIENT_COLOR& Color) const
{
	if (m_ColorMode == GRADIENT_RGB) {
		DRGB	rgb;
		GetColor(rgb);
		Color.rgb = rgb;
	} else {
		DHLS	hls;
		GetColor(hls);
		Color.hls = hls;
	}
}

void CGradientDlg::SetTriplet(const GRADIENT_COLOR& Color)
{
	if (m_ColorMode == GRADIENT_RGB)
		SetColor(Color.rgb);
	else
		SetColor(Color.hls);
}

bool CGradientDlg::IsDragging() const
{
	return(m_DragState == DTS_DRAG || CColorPickerDlg::IsDragging());
}

void CGradientDlg::UpdateColor()
{
	CColorPickerDlg::UpdateColor();	// let base class update color picker
	if (m_Grad.HasSel() && m_PickEnable) {
		GRADIENT_COLOR	c;
		GetTriplet(c);
		m_Grad.SetColor(c);	// update selected points from color picker
	}
}

void CGradientDlg::UpdateGradient()
{
	int	sel = m_Grad.GetCurSel();
	if (sel >= 0) {
		m_PickEnable = FALSE;	// prevent UpdateColor from updating points
		GRADIENT_COLOR	c;
		m_Grad.GetColor(sel, c);
		SetTriplet(c);		// update color picker from selected point
		m_PickEnable = TRUE;
		m_PosEdit.SetVal(m_Grad.GetPos(sel));	// update position too
	}
}

double CGradientDlg::CalcPos(CPoint point) const
{
	CRect	r;
	m_Grad.GetWindowRect(r);
	ScreenToClient(r);
	double	pos = double(point.x - r.left) / r.Width();
	return(CLAMP(pos, 0, 1));
}

int CGradientDlg::SelectPoint(int nFlags, CPoint point)
{
	CPoint	pt(point);
	MapWindowPoints(&m_Grad, &pt, 1);
	int	idx = m_Grad.HitTest(pt);
	if (idx >= 0) {	// if on a marker
		if (nFlags & MK_SHIFT) {	// if shift key down
			m_Grad.SelectSpan(idx);	// select contiguous group
		} else if (nFlags & MK_CONTROL) {	// if control key down
			m_Grad.ToggleSelect(idx);	// toggle marker's selection
		} else if (m_Grad.IsSelected(idx)) {	// if marker is already selected
			m_Grad.UpdateCurSel(idx);	// update current selection
		} else {	// marker isn't selected
			m_Grad.SetCurSel(idx);	// start a new selection
		}
	} else {	// not on a marker
		if (!(nFlags & MK_CONTROL)) {
			CRect	r;
			m_Grad.GetMarkerRect(r);
			r.bottom += MARKER_BOTTOM_MARGIN;
			if (r.PtInRect(pt))	// if within marker area
				m_Grad.Deselect();	// remove selection
		}
	}
	return(idx);
}

void CGradientDlg::ShowInterpolatedColor(CPoint point)
{
	double	pos = CalcPos(point);
	DRGB	c;
	m_Grad.Interpolate(pos, c);
	SetColor(c);
}

bool CGradientDlg::CtrlHitTest(CPoint point)
{
	CRect	rc, r;
	m_Grad.GetClientRect(rc);
	m_Grad.GetMarkerRect(r);
	r.UnionRect(r, rc);
	m_Grad.ClientToScreen(r);
	return(r.PtInRect(point) != 0);
}

int CGradientDlg::FindMarker(CPoint point)
{
	CPoint	pt(point);
	MapWindowPoints(&m_Grad, &pt, 1);
	return(m_Grad.HitTest(pt));
}

void CGradientDlg::DoDataExchange(CDataExchange* pDX)
{
	CColorPickerDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGradientDlg)
	DDX_Control(pDX, IDC_GRAD_SPACE_EVENLY_CHK, m_SpaceEvenlyChk);
	DDX_Control(pDX, IDC_GRAD_POS_EDIT, m_PosEdit);
	DDX_Control(pDX, IDC_GRAD_GRADIENT, m_Grad);
	DDX_Radio(pDX, IDC_GRAD_COLOR_MODE_0, m_ColorMode);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CGradientDlg, CColorPickerDlg)
	//{{AFX_MSG_MAP(CGradientDlg)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_GRAD_INSERT, OnInsert)
	ON_BN_CLICKED(IDC_GRAD_DELETE, OnDelete)
	ON_COMMAND(ID_GRAD_INSERT_AT, OnInsertAt)
	ON_WM_LBUTTONDOWN()
	ON_BN_CLICKED(IDC_GRAD_COLOR_MODE_0, OnColorMode)
	ON_EN_KILLFOCUS(IDC_GRAD_POS_EDIT, OnKillfocusPosEdit)
	ON_BN_CLICKED(IDC_GRAD_SPACE_EVENLY_CHK, OnSpaceEvenly)
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONDOWN()
	ON_WM_PARENTNOTIFY()
	ON_UPDATE_COMMAND_UI(IDC_GRAD_DELETE, OnUpdateDelete)
	ON_COMMAND(ID_GRAD_REVERSE_CYCLE, OnReverseCycle)
	ON_COMMAND(ID_GRAD_INVERT_COLOR, OnInvertColor)
	ON_COMMAND(ID_GRAD_ROTATE_HUE, OnRotateHue)
	ON_COMMAND(ID_GRAD_SELECT_ALL, OnSelectAll)
	ON_BN_CLICKED(IDC_GRAD_COLOR_MODE_1, OnColorMode)
	ON_COMMAND(ID_GRAD_DELETE, OnDelete)
	//}}AFX_MSG_MAP
	ON_NOTIFY(GCN_GRADIENTCHANGE, IDC_GRAD_GRADIENT, OnGradientChange)
	ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGradientDlg message handlers

BOOL CGradientDlg::OnInitDialog() 
{
	m_ColorMode = m_Grad.GetColorMode();	// order matters
	CColorPickerDlg::OnInitDialog();
	m_PosEdit.SetRange(0, 1);
	if (GetCount())
		m_Grad.SetCurSel(0);
	m_SpaceEvenlyChk.SetCheck(m_Grad.GetSpaceEvenly());
	m_Grad.ModifyStyleEx(WS_EX_NOPARENTNOTIFY, 0, 0);	// for eyedropper
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CGradientDlg::OnOK() 
{
	CColorPickerDlg::OnOK();
	m_Grad.SetSpaceEvenly(m_SpaceEvenlyChk.GetCheck() != 0);
}

void CGradientDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	m_ShadePicker.UpdateMark();
	m_Grad.InvalidateMarkers(NULL, FALSE);	// no need to erase background
}

LRESULT CGradientDlg::OnKickIdle(WPARAM wParam, LPARAM lParam)
{
    UpdateDialogControls(this, TRUE);
    return FALSE;
}

void CGradientDlg::OnGradientChange(NMHDR* pNMHDR, LRESULT* pResult)
{
	if (m_DragState == DTS_NONE)
		UpdateGradient();
}

void CGradientDlg::OnInsert() 
{
	CGradientPt	pt;
	GetTriplet(pt.m_Color);
	if (m_Grad.HasSel())
		pt.m_Pos = -1;
	else
		pt.m_Pos = m_PosEdit.GetVal();
	m_Grad.Insert(pt);
}

void CGradientDlg::OnInsertAt()
{
	CGradientPt	pt;
	GetTriplet(pt.m_Color);
	pt.m_Pos = CalcPos(m_ContextPt);
	m_Grad.Insert(pt);
}

void CGradientDlg::OnDelete() 
{
	m_Grad.Delete();
}

void CGradientDlg::OnUpdateDelete(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_Grad.HasSel());
}

void CGradientDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CColorPickerDlg::OnLButtonDown(nFlags, point);	// do base class first
	if (!CColorPickerDlg::IsDragging()) {	// if base class didn't capture
		int	idx = SelectPoint(nFlags, point);
		if (idx >= 0) {
			m_DragState = DTS_TRACK;
			m_DragOrigin = point;
			SetCapture();
		} else {
			CRectTrackerEx	tracker;
			bool	MultiSel = (GetAsyncKeyState(VK_CONTROL) & GKS_DOWN) != 0;
			if (tracker.TrackRubberBand(this, point)) {	// if rubber band ok
				CRect	rt = tracker.m_rect;
				rt.NormalizeRect();	// tracker rect can be inverted
				MapWindowPoints(&m_Grad, rt);	// map tracker rect to gradient
				bool	FirstPt = TRUE;
				int	pts = GetCount();
				for (int i = 0; i < pts; i++) {	// for each gradient point
					CRect	rm;
					m_Grad.GetMarkerRect(i, rm);
					if (rt.PtInRect(rm.TopLeft())	// if marker within tracker rect
					&& rt.PtInRect(rm.BottomRight())) {
						if (FirstPt && !MultiSel)	// if single selection only
							m_Grad.Deselect();			// deselect first
						m_Grad.Select(i, TRUE);	// select point
						if (FirstPt) {
							FirstPt = FALSE;
							UpdateGradient();	// update color picker
						}
					}
				}
			}
		}
	}
}

void CGradientDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	switch (m_DragState) {
	case DTS_DRAG:
		m_DragState = DTS_NONE;	// so view gets updated
		if (m_SpaceEvenlyChk.GetCheck())
			m_Grad.SetSpaceEvenly(TRUE);	// restore spacing
		UpdateGradient();
		break;
	case DTS_ROTATE:
		m_DragState = DTS_NONE;	// so view gets updated
		UpdateGradient();
		break;
	default:
		if (!(nFlags & (MK_SHIFT | MK_CONTROL))) {	// if no modifier keys
			int	idx = FindMarker(point);
			if (idx >= 0)	// if on a marker
				m_Grad.SetCurSel(idx);	// start a new selection
		}
		break;
	}
	m_DragState = DTS_NONE;
	ReleaseCapture();
	CColorPickerDlg::OnLButtonUp(nFlags, point);
}

void CGradientDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	switch (m_DragState) {
	case DTS_TRACK:
		if (abs(m_DragOrigin.x - point.x) > DRAG_THRESHOLD) {
			m_DragState = DTS_DRAG;
			m_Grad.SetSpaceEvenly(FALSE);	// no spacing during drag
		}
		break;
	case DTS_DRAG:
		{
			int	sel = m_Grad.GetCurSel();
			if (sel >= 0) {
				double	pos = CalcPos(point);
				if (pos != m_Grad.GetPos(sel)) {	// if position changed
					m_Grad.MoveSelection(pos);
					m_PosEdit.SetVal(m_Grad.GetPos(sel));
				}
			}
		}
		break;
	case DTS_EYEDROPPER:
		ShowInterpolatedColor(point);
		break;
	case DTS_ROTATE:
		{
			double	pos = CalcPos(point);
			if (pos != m_RotPrevPos) {
				m_Grad.RotateGradient(pos - m_RotPrevPos);
				m_RotPrevPos = pos;
			}
		}
		break;
	}
	CColorPickerDlg::OnMouseMove(nFlags, point);
}

void CGradientDlg::OnParentNotify(UINT message, LPARAM lParam)
{
	if (message == WM_LBUTTONDOWN) {
		CPoint	point(LOWORD(lParam), HIWORD(lParam));
		CRect	r;
		m_Grad.GetWindowRect(r);
		ScreenToClient(r);
		r.DeflateRect(1, 1);	// don't trigger on border
		if (r.PtInRect(point)) {
			int	cursor;
			if (GetAsyncKeyState(VK_CONTROL) & GKS_DOWN) {
				m_Grad.Deselect();
				m_DragState = DTS_ROTATE;
				m_RotPrevPos = CalcPos(point);
				cursor = IDC_TOOL_HAND_OPEN;
			} else {
				m_Grad.Deselect();
				m_DragState = DTS_EYEDROPPER;
				ShowInterpolatedColor(point);
				cursor = IDC_EYEDROPPER;
			}
			SetCursor(AfxGetApp()->LoadCursor(cursor));
			SetCapture();
		}
	}
}

void CGradientDlg::OnRButtonDown(UINT nFlags, CPoint point) 
{
	int	idx = FindMarker(point);
	if (idx >= 0) {	// if on a marker
		if (!m_Grad.IsSelected(idx))	// if marker not selected
			m_Grad.SetCurSel(idx);	// select it
	}
	CColorPickerDlg::OnRButtonDown(nFlags, point);
}

void CGradientDlg::OnColorMode() 
{
	UpdateData(TRUE);
	m_Grad.SetColorMode(m_ColorMode);
}

void CGradientDlg::OnKillfocusPosEdit() 
{
	int	sel = m_Grad.GetCurSel();
	if (sel >= 0)
		m_Grad.MoveSelection(m_PosEdit.GetVal());
}

void CGradientDlg::OnSpaceEvenly() 
{
	m_Grad.SetSpaceEvenly(m_SpaceEvenlyChk.GetCheck() != 0);
}

inline void CGradientDlg::EnableMenuItem(CMenu& Menu, UINT CmdID, bool Enable)
{
	Menu.EnableMenuItem(MF_BYCOMMAND | CmdID, Enable ? MF_ENABLED : MF_GRAYED);
}

void CGradientDlg::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	if (CtrlHitTest(point)) {	// if within control or its marker area
		CMenu	menu, *mp;
		menu.LoadMenu(IDR_GRADIENT);
		mp = menu.GetSubMenu(0);
		CPoint	pt(point);
		m_Grad.ScreenToClient(&pt);
		bool	HaveSel = m_Grad.GetSelCount() > 0;
		bool	IsWndEnab = m_Grad.IsWindowEnabled() != 0;
		EnableMenuItem(*mp, ID_GRAD_INSERT_AT, IsWndEnab);
		EnableMenuItem(*mp, ID_GRAD_DELETE, HaveSel);
		EnableMenuItem(*mp, ID_GRAD_INVERT_COLOR, HaveSel);
		EnableMenuItem(*mp, ID_GRAD_ROTATE_HUE, HaveSel);
		EnableMenuItem(*mp, ID_GRAD_REVERSE_CYCLE,
			HaveSel && m_Grad.GetColorMode() == GRADIENT_HLS);
		EnableMenuItem(*mp, ID_GRAD_SELECT_ALL, IsWndEnab);
		m_ContextPt = point;
		ScreenToClient(&m_ContextPt);
		mp->TrackPopupMenu(0, point.x, point.y, this);
	} else
		CColorPickerDlg::OnContextMenu(pWnd, point);	// delegate to base class
}

void CGradientDlg::OnReverseCycle()
{
	int	pts = m_Grad.GetCount();
	for (int i = 0; i < pts; i++) {
		if (m_Grad.IsSelected(i)) {
			GRADIENT_COLOR	c;
			m_Grad.GetColor(i, c);
			if (c.hls.m_h >= 0)
				c.hls.m_h -= 1;
			else
				c.hls.m_h += 1;
			m_Grad.SetColor(i, c);
		}
	}
	UpdateGradient();
}

void CGradientDlg::OnInvertColor()
{
	int	pts = m_Grad.GetCount();
	for (int i = 0; i < pts; i++) {
		if (m_Grad.IsSelected(i))
			m_Grad.Invert(i);
	}
	UpdateGradient();
}

void CGradientDlg::OnRotateHue()
{
	int	pts = m_Grad.GetCount();
	for (int i = 0; i < pts; i++) {
		if (m_Grad.IsSelected(i))
			m_Grad.RotateHue(i, m_HueRotAmt);
	}
	UpdateGradient();
}

void CGradientDlg::OnSelectAll()
{
	m_Grad.SelectAll();
}
