// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
        01		27feb09	in OnPaint, restore DC's previous bitmap

		static control that displays a picture
 
*/

// PictureCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "PictureCtrl.h"
#include "Dib.h"	// for Letterbox

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPictureCtrl

IMPLEMENT_DYNAMIC(CPictureCtrl, CStatic);

CPictureCtrl::CPictureCtrl()
{
	m_Bitmap = NULL;
}

CPictureCtrl::~CPictureCtrl()
{
}

void CPictureCtrl::SetBitmap(HBITMAP Bitmap, const CRect *Detail)
{
	m_Bitmap = Bitmap;
	if (Detail != NULL)
		m_Detail = *Detail;
	else {
		if (Bitmap != NULL) {
			BITMAP	bm;
			GetObject(Bitmap, sizeof(BITMAP), &bm);
			m_Detail = CRect(0, 0, bm.bmWidth, bm.bmHeight);
		}
	}
	Invalidate();
}

void CPictureCtrl::SetAspectRatio(double Aspect)
{
	CRect	r;
	GetClientRect(r);
	r.bottom = int(r.Width() * Aspect) + 1;
	SetWindowPos(NULL, 0, 0, r.Width(), r.Height(), SWP_NOMOVE | SWP_NOZORDER);
}

BEGIN_MESSAGE_MAP(CPictureCtrl, CStatic)
	//{{AFX_MSG_MAP(CPictureCtrl)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPictureCtrl message handlers

void CPictureCtrl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	CRect	rc;
	GetClientRect(rc);
	if (m_Bitmap != NULL) {
		dc.SetStretchBltMode(HALFTONE);
		CRect	rd, rs(m_Detail);
		CDib::Letterbox(rc, rs.Size(), rd);
		CDC	mem;
		mem.CreateCompatibleDC(&dc);
		HGDIOBJ	PrevBmp = mem.SelectObject(m_Bitmap);
		dc.StretchBlt(rd.left, rd.top, rd.Width(), rd.Height(), 
			&mem, rs.left, rs.top, rs.Width(), rs.Height(), SRCCOPY);
		dc.ExcludeClipRect(rd);	// remove picture from clip region
		mem.SelectObject(PrevBmp);	// restore DC's previous bitmap
	}
	dc.FillSolidRect(rc, GetSysColor(COLOR_3DFACE));
}
