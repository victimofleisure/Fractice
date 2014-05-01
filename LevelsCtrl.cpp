// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		27		06jan10	W64: in OnPaint, cast sample array size to 32-bit

        levels control
 
*/

// LevelsCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "LevelsCtrl.h"
#include "FracticeDoc.h"
#include "FracticeView.h"
#include "MainFrm.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLevelsCtrl

IMPLEMENT_DYNAMIC(CLevelsCtrl, CStatic);

CLevelsCtrl::CLevelsCtrl()
{
	m_ScalingType = ST_LINEAR;
	m_ShowAxis = FALSE;
	m_View = NULL;
}

CLevelsCtrl::~CLevelsCtrl()
{
}

void CLevelsCtrl::CalcLevels()
{
	if (m_View->GetHistogram(m_Sample, m_MaxSamp))
		Invalidate();
	else
		SetEmpty();
}

void CLevelsCtrl::SetEmpty()
{
	m_Sample.RemoveAll();
	Invalidate();
}

void CLevelsCtrl::SetView(CFracticeView *View)
{
	if (View == m_View)
		return;	// nothing to do
	m_View = View;
	if (View != NULL)
		CalcLevels();
	else
		SetEmpty();
}

void CLevelsCtrl::SetScalingType(int Type)
{
	m_ScalingType = Type;
	Invalidate();
}

void CLevelsCtrl::ShowAxis(bool Enable)
{
	m_ShowAxis = Enable;
	Invalidate();
}

UINT CLevelsCtrl::CalcAxisStep(int Samples, int AxisLen, int MinSpacing)
{
	static const int denom[] = {1, 2, 5};
	double	ratio = double(AxisLen) / Samples;
	int	step = 1;
	int	scale = 1;
	int	i = 0;
	while (step * ratio < MinSpacing) {
		i++;
		int	di = i % 3;
		if (!di)
			scale *= 10;
		step = denom[di] * scale;
	}
	return(step);
}

BEGIN_MESSAGE_MAP(CLevelsCtrl, CStatic)
	//{{AFX_MSG_MAP(CLevelsCtrl)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLevelsCtrl message handlers

void CLevelsCtrl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	CRect	rc;
	GetClientRect(rc);
	dc.FillSolidRect(rc, GetSysColor(COLOR_3DFACE));
	int	samps = INT64TO32(m_Sample.GetSize());
	if (samps > 0) {
		dc.IntersectClipRect(rc);	// exclude border from clipping region
		int	w = rc.Width();
		int	h = rc.Height();
		if (m_ShowAxis) {
			static const int	MIN_SPACING = 32;
			int	step = CalcAxisStep(samps, w, MIN_SPACING);
			dc.SetBkMode(TRANSPARENT);
			COLORREF	color = GetSysColor(COLOR_3DDKSHADOW);
			dc.SetTextColor(color);
			dc.SelectObject(GetStockObject(DEFAULT_GUI_FONT));
			for (int i = step; i < samps; i += step) {
				int	x = round(i / double(samps) * w);
				dc.FillSolidRect(x, 0, 1, h, color);
				CString	s;
				s.Format(_T("%d"), i);
				dc.TextOut(x + 2, 0, s);
			}
		}
		double	BarWidth = double(w) / samps;
		double	MaxSamp = m_MaxSamp;
		if (m_ScalingType == ST_LOG10)
			MaxSamp = log10(MaxSamp + 1);
		for (int i = 0; i < samps; i++) {
			if (m_Sample[i]) {
				double	samp = m_Sample[i];
				if (m_ScalingType == ST_LOG10)
					samp = log10(samp + 1);	// offset so single count shows
				int	y = round(samp / MaxSamp * h);
				double	x = i * BarWidth;
				CRect	r(round(x), h - y, round(x + BarWidth), h);
				if (r.right == r.left)
					r.right = r.left + 1;	// at least one pixel wide
				r.left = max(r.left, 0);	// clip bar sides to window
				r.right = min(r.right, w);
				dc.FillSolidRect(r, RGB(0, 0, 0));
			}
		}
	}
}
