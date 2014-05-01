// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
		00		17oct07	initial version
		01		05mar08	add IsDragging, send notification instead of app msg
		02		09mar08	in DragTest, fix top and bottom cases
		03		24dec08	support disabled state

		shade picker control, used as a buddy for CColorPickerCtrl
 
*/

// ShadePickerCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "ShadePickerCtrl.h"
#include "ColorPickerCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CShadePickerCtrl

IMPLEMENT_DYNAMIC(CShadePickerCtrl, CStatic);

CShadePickerCtrl::CShadePickerCtrl()
{
	m_Buddy = NULL;
	m_Mark = 0;
	m_Dragging = FALSE;
	m_ShowMark = TRUE;
}

CShadePickerCtrl::~CShadePickerCtrl()
{
}

void CShadePickerCtrl::PtToHLS(const CRect& r, const CPoint& pt, DHLS& hls)
{
	hls.m_l = 1 - double(pt.y) / r.Height();
}

void CShadePickerCtrl::HLSToPt(const CRect& r, const DHLS& hls, CPoint& pt)
{
	pt.y = round((1 - hls.m_l) * r.Height());
}

void CShadePickerCtrl::UpdateBuddy()
{
	if (m_Buddy != NULL) {
		CShadePickerCtrl	*bud = m_Buddy->GetBuddy();
		m_Buddy->SetBuddy(NULL);	// avoid infinite recursion
		m_Buddy->SetColor(m_hls);
		m_Buddy->SetBuddy(bud);		// restore previous buddy (presumably us)
	}
}

void CShadePickerCtrl::GetMarkRect(CRect& r)
{
	CRect	cr;
	GetClientRect(cr);
	int	x = cr.right + MARK_OFFSET;
	r = CRect(x, m_Mark - MARK_SIZE, x + MARK_SIZE, m_Mark + MARK_SIZE);
}

void CShadePickerCtrl::ShowMark(bool Enable)
{
	if (Enable == m_ShowMark)
		return;	// nothing to do
	m_ShowMark = Enable;	// order matters
	DrawMark(Enable);
}

void CShadePickerCtrl::SetMark(CPoint pt)
{
	CRect	cr;
	GetClientRect(cr);
	pt.y = CLAMP(pt.y, cr.top, cr.bottom);
	PtToHLS(cr, pt, m_hls);
	DrawMark(FALSE);
	m_Mark = pt.y;
	DrawMark(TRUE);
	UpdateBuddy();
	NotifyParent();
}

void CShadePickerCtrl::NotifyParent()
{
	NMHDR	nmh;
	nmh.hwndFrom = m_hWnd;
	nmh.idFrom = GetDlgCtrlID();
	nmh.code = CPN_COLORPICK;
	GetParent()->SendMessage(WM_NOTIFY, nmh.idFrom, (LPARAM)&nmh);
}

void CShadePickerCtrl::UpdateColor()
{
	CRect	cr;
	GetClientRect(cr);
	CPoint	pt;
	HLSToPt(cr, m_hls, pt);
	if (pt.y != m_Mark) {
		DrawMark(FALSE);
		m_Mark = pt.y;
		DrawMark(TRUE);
	}
	Invalidate();
	UpdateBuddy();
}

void CShadePickerCtrl::SetColor(const DHLS& Color)
{
	m_hls = Color;
	if (m_hWnd)
		UpdateColor();
}

void CShadePickerCtrl::DrawMark(bool Show, CDC *pDC)
{
	HGDIOBJ	PrevBrush;
	if (Show)
		PrevBrush = SelectObject(pDC->m_hDC, GetStockObject(BLACK_BRUSH));
	else
		PrevBrush = SelectObject(pDC->m_hDC, GetSysColorBrush(COLOR_3DFACE));
	HGDIOBJ	PrevPen = SelectObject(pDC->m_hDC, GetStockObject(NULL_PEN));
	CRect	cr;
	GetClientRect(cr);
	CPoint	pt(cr.right + MARK_OFFSET, m_Mark);
	CPoint	pta[3];
	pta[0] = pt;
	pta[1] = CPoint(pt.x + MARK_SIZE, pt.y - MARK_SIZE);
	pta[2] = CPoint(pt.x + MARK_SIZE, pt.y + MARK_SIZE);
	pDC->Polygon(pta, 3);
	SelectObject(pDC->m_hDC, PrevBrush);
	SelectObject(pDC->m_hDC, PrevPen);
}

void CShadePickerCtrl::DrawMark(bool Show)
{
	CClientDC	dc(this);
	DrawMark(Show, &dc);
}

void CShadePickerCtrl::UpdateMark()
{
	CClientDC	dc(this);
	DrawMark(m_ShowMark, &dc);
}

void CShadePickerCtrl::DragTest(CPoint point)
{
	if (!IsWindowEnabled())
		return;
	CRect	cr;
	GetClientRect(cr);
	CRect	mr(cr);
	mr.left = cr.right;
	mr.right += MARK_OFFSET + MARK_SIZE + 1;
	mr.top -= MARK_SIZE;
	mr.bottom += MARK_SIZE + 1;
	if (cr.PtInRect(point) || mr.PtInRect(point))
		OnLButtonDown(0, point);
}

BEGIN_MESSAGE_MAP(CShadePickerCtrl, CStatic)
	//{{AFX_MSG_MAP(CShadePickerCtrl)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_ENABLE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CShadePickerCtrl message handlers

void CShadePickerCtrl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	if (!IsWindowEnabled())	// if disabled
		return;
	CRect	pr(dc.m_ps.rcPaint);	// rectangle that needs painting
	CRect	cr;
	GetClientRect(cr);
	DRGB	rgb;
	for (int x = pr.left; x < pr.right; x++) {
		for (int y = pr.top; y < pr.bottom; y++) {
			PtToHLS(cr, CPoint(x, y), m_hls);
			hls2rgb(m_hls, rgb);
			dc.SetPixelV(x, y, rgb);
		}
	}
}

void CShadePickerCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	SetCapture();
	m_Dragging = TRUE;
	SetMark(point);
	CStatic::OnLButtonDown(nFlags, point);
}

void CShadePickerCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (m_Dragging) {
		ReleaseCapture();
		m_Dragging = FALSE;
		SetMark(point);
	}
	CStatic::OnLButtonUp(nFlags, point);
}

void CShadePickerCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_Dragging)
		SetMark(point);
	CStatic::OnMouseMove(nFlags, point);
}

void CShadePickerCtrl::OnEnable(BOOL bEnable)
{
	ShowMark(bEnable != 0);
	CStatic::OnEnable(bEnable);
	Invalidate();	// ensure a repaint
}
