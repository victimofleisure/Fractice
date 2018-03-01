// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
		00		17oct07	initial version
		01		08nov07	move 360 degree normalization into DColor object
		02		05mar08	add IsDragging, send notification instead of app msg
		03		09mar08	wrap out-of-range hues
		04		24dec08	support disabled state

		color picker control
 
*/

// ColorPickerCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "ColorPickerCtrl.h"
#include "ShadePickerCtrl.h"
#include "hls.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CColorPickerCtrl

IMPLEMENT_DYNAMIC(CColorPickerCtrl, CStatic);

CColorPickerCtrl::CColorPickerCtrl()
{
	m_Buddy = NULL;
	m_Mark = CPoint(0, 0);
	m_Dragging = FALSE;
	m_ShowMark = TRUE;
}

CColorPickerCtrl::~CColorPickerCtrl()
{
}

void CColorPickerCtrl::PtToHLS(const CRect& r, const CPoint& pt, DHLS& hls)
{
	hls.m_h = double(pt.x) / r.Width();
	hls.m_s = 1 - double(pt.y) / r.Height();
}

void CColorPickerCtrl::HLSToPt(const CRect& r, const DHLS& hls, CPoint& pt)
{
	double	h = fmod(hls.m_h, 1);
	if (h < 0)
		h = h + 1;
	pt.x = round(h * r.Width());
	pt.y = round((1 - hls.m_s) * r.Height());
}

void CColorPickerCtrl::UpdateBuddy()
{
	if (m_Buddy != NULL) {
		CColorPickerCtrl	*bud = m_Buddy->GetBuddy();
		m_Buddy->SetBuddy(NULL);	// avoid infinite recursion
		m_Buddy->SetColor(m_hls);
		m_Buddy->SetBuddy(bud);		// restore previous buddy (presumably us)
	}
}

void CColorPickerCtrl::GetMarkRect(CRect& r)
{
	r = CRect(m_Mark, m_Mark);
	r.InflateRect(CSize(MARK_SIZE, MARK_SIZE));
}

void CColorPickerCtrl::ShowMark(bool Enable)
{
	if (Enable == m_ShowMark)
		return;	// nothing to do
	m_ShowMark = Enable;	// order matters
	CRect	mr;
	GetMarkRect(mr);
	RedrawWindow(mr);
}

void CColorPickerCtrl::UpdateMark()
{
	CRect	cr;
	GetClientRect(cr);
	HLSToPt(cr, m_hls, m_Mark);
}

void CColorPickerCtrl::SetMark(CPoint pt)
{
	CRect	cr;
	GetClientRect(cr);
	pt.x = CLAMP(pt.x, cr.left, cr.right);
	pt.y = CLAMP(pt.y, cr.top, cr.bottom);
	PtToHLS(cr, pt, m_hls);
	m_Mark = pt;
	hls2rgb(m_hls, m_rgb);
	UpdateBuddy();
	NotifyParent();
}

void CColorPickerCtrl::NotifyParent()
{
	NMHDR	nmh;
	nmh.hwndFrom = m_hWnd;
	nmh.idFrom = GetDlgCtrlID();
	nmh.code = CPN_COLORPICK;
	GetParent()->SendMessage(WM_NOTIFY, nmh.idFrom, (LPARAM)&nmh);
}

void CColorPickerCtrl::UpdateColor()
{
	CRect	cr;
	GetClientRect(cr);
	CPoint	pt;
	HLSToPt(cr, m_hls, pt);
	if (pt != m_Mark) {
		bool	prevshow = m_ShowMark;
		ShowMark(FALSE);
		m_Mark = pt;
		ShowMark(prevshow);
	}
	UpdateBuddy();
}

void CColorPickerCtrl::SetColor(const DRGB& Color)
{
	rgb2hls(Color, m_hls);
	m_rgb = Color;
	if (m_hWnd)
		UpdateColor();
}

void CColorPickerCtrl::SetColor(const DHLS& Color)
{
	hls2rgb(Color, m_rgb);
	m_hls = Color;
	if (m_hWnd)
		UpdateColor();
}

void CColorPickerCtrl::SetColor(COLORREF Color)
{
	DRGB	rgb(Color);
	SetColor(rgb);
}

BEGIN_MESSAGE_MAP(CColorPickerCtrl, CStatic)
	//{{AFX_MSG_MAP(CColorPickerCtrl)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_SIZE()
	ON_WM_ENABLE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CColorPickerCtrl message handlers

void CColorPickerCtrl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	if (!IsWindowEnabled())	// if disabled
		return;
	CRect	pr(dc.m_ps.rcPaint);	// rectangle that needs painting
	CRect	cr;
	GetClientRect(cr);
	DHLS	hls;
	hls.m_l = .5;	// middle lightness
	DRGB	rgb;
	for (int x = pr.left; x < pr.right; x++) {
		for (int y = pr.top; y < pr.bottom; y++) {
			PtToHLS(cr, CPoint(x, y), hls);
			hls2rgb(hls, rgb);
			dc.SetPixelV(x, y, rgb);
		}
	}
	if (m_ShowMark) {
		CRect	mr;
		GetMarkRect(mr);
		CRect	inter;
		if (inter.IntersectRect(mr, pr)) {	// if marker intersects paint rect
			dc.IntersectClipRect(cr);	// prevent drawing outside our client rect
			dc.SelectObject(GetStockObject(NULL_BRUSH));	// outline only
			dc.Ellipse(mr);	// draw marker
		}
	}
}

void CColorPickerCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	SetCapture();
	m_Dragging = TRUE;
	ShowMark(FALSE);	// order matters
	SetMark(point);
	CStatic::OnLButtonDown(nFlags, point);
}

void CColorPickerCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (m_Dragging) {
		ReleaseCapture();
		m_Dragging = FALSE;
		SetMark(point);
		ShowMark(TRUE);
	}
	CStatic::OnLButtonUp(nFlags, point);
}

void CColorPickerCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_Dragging)
		SetMark(point);
	CStatic::OnMouseMove(nFlags, point);
}

void CColorPickerCtrl::OnSize(UINT nType, int cx, int cy) 
{
	CStatic::OnSize(nType, cx, cy);
	UpdateMark();
}

void CColorPickerCtrl::OnEnable(BOOL bEnable)
{
	CStatic::OnEnable(bEnable);
	Invalidate();	// ensure a repaint
}
