// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		05jan10	standardize OnInitDialog prototype
		
		history control bar
 
*/

// HistoryBar.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "HistoryBar.h"
#include "MainFrm.h"
#include "OptionsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHistoryBar dialog

IMPLEMENT_DYNAMIC(CHistoryBar, CMySizingControlBar);

CHistoryBar::CHistoryBar()
{
	//{{AFX_DATA_INIT(CHistoryBar)
	//}}AFX_DATA_INIT
	static const int INITIAL_WIDTH = COptionsInfo::DEF_THUMB_WIDTH
		+ CHistoryView::THUMB_MARGIN * 2 + 30;	// include vertical scroll bar
    m_szVert = CSize(INITIAL_WIDTH, 400);	// size of bar when vertically docked
	m_View = NULL;
}

void CHistoryBar::SetView(CHistoryView *View)
{
	if (View == m_View)
		return;	// nothing to do
	if (m_View != NULL)
		m_View->ShowWindow(SW_HIDE);	// hide previous history view
	if (View != NULL) {
		CRect	r;
		GetClientRect(r);
		View->MoveWindow(0, 0, r.Width(), r.Height());
		View->ShowWindow(SW_SHOW);	// show new history view
	}
	m_View = View;
}

BEGIN_MESSAGE_MAP(CHistoryBar, CMySizingControlBar)
	//{{AFX_MSG_MAP(CHistoryBar)
	ON_WM_SIZE()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_INITDIALOG, OnInitDialog)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHistoryBar message handlers

LRESULT CHistoryBar::OnInitDialog(WPARAM wParam, LPARAM lParam)
{
	return TRUE;
}

int CHistoryBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMySizingControlBar::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

void CHistoryBar::OnSize(UINT nType, int cx, int cy)
{
	CMySizingControlBar::OnSize(nType, cx, cy);
	if (m_View != NULL)	// if we have a view
		m_View->MoveWindow(0, 0, cx, cy);
}
