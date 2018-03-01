// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      02feb09	initial version
		01		22mar09	compensate docked minimums for non-client area
		02		05jan10	standardize OnInitDialog prototype

		sizable dialog bar base class
 
*/

// SizingDialogBar.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "SizingDialogBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSizingDialogBar dialog

IMPLEMENT_DYNAMIC(CSizingDialogBar, CMySizingControlBar);

const SIZE CSizingDialogBar::m_DlgMargin = {2, 3};

CSizingDialogBar::CSizingDialogBar()
{
	//{{AFX_DATA_INIT(CSizingDialogBar)
	//}}AFX_DATA_INIT
	m_pDlg = NULL;
	m_ResID = 0;
	m_Dirty = FALSE;
}

BEGIN_MESSAGE_MAP(CSizingDialogBar, CMySizingControlBar)
	//{{AFX_MSG_MAP(CSizingDialogBar)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_WINDOWPOSCHANGED()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_INITDIALOG, OnInitDialog)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSizingDialogBar message handlers

int CSizingDialogBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMySizingControlBar::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	ASSERT(m_pDlg != NULL);
	ASSERT(m_ResID);
	m_pDlg->Create(m_ResID, this);
	CRect	r;
	m_pDlg->GetClientRect(r);
	CSize	sz = r.Size() + m_DlgMargin;	// adjust size to match resource
	m_szMinFloat = sz;
	m_szFloat = sz;
	enum {
		NONCLIENT_SIZE = 10	// non-client area of docked sizing bar
	};
	m_szMinHorz = sz + CSize(NONCLIENT_SIZE, 0);
	m_szMinVert = sz + CSize(0, NONCLIENT_SIZE);

	return 0;
}

LRESULT CSizingDialogBar::OnInitDialog(WPARAM wParam, LPARAM lParam)
{
	m_pDlg->ShowWindow(SW_SHOW);
	return TRUE;
}

void CSizingDialogBar::OnSize(UINT nType, int cx, int cy) 
{
	CMySizingControlBar::OnSize(nType, cx, cy);
	m_pDlg->MoveWindow(-m_DlgMargin.cx, -m_DlgMargin.cy, 
		cx + m_DlgMargin.cx, cy + m_DlgMargin.cy);
}

void CSizingDialogBar::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	CMySizingControlBar::OnWindowPosChanged(lpwndpos);
	if ((lpwndpos->flags & SWP_SHOWWINDOW) && m_Dirty) {
		m_pDlg->SendMessage(UWM_DLGBARUPDATE);
		m_Dirty = FALSE;
	}
}
