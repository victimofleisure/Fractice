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
		
		parameter control bar
 
*/

// ParamsBar.cpp : implementation file
//

#include "stdafx.h"
#include "Fractice.h"
#include "ParamsBar.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CParamsBar dialog

IMPLEMENT_DYNAMIC(CParamsBar, CMySizingControlBar);

#define	RK_PARAM_COL_WIDTHS	_T("ParamColWidths")

CParamsBar::CParamsBar()
{
	//{{AFX_DATA_INIT(CParamsBar)
	//}}AFX_DATA_INIT
    m_szVert = CSize(150, 400);
	m_Dirty = FALSE;
}

bool CParamsBar::LoadColumnWidths()
{
	UINT	width[COLUMNS];
	DWORD	size = sizeof(width);
	if (!CPersist::GetBinary(REG_SETTINGS, RK_PARAM_COL_WIDTHS, width, &size))
		return(FALSE);
	static const int MIN_WIDTH = 6;
	for (int i = 0; i < COLUMNS; i++) {
		if (width[i] > MIN_WIDTH)
			m_List.SetColumnWidth(i, width[i]);
	}
	return(TRUE);
}

bool CParamsBar::StoreColumnWidths()
{
	UINT	width[COLUMNS];
	for (int i = 0; i < COLUMNS; i++)
		width[i] = m_List.GetColumnWidth(i);
	return(CPersist::WriteBinary(REG_SETTINGS, RK_PARAM_COL_WIDTHS, 
		width, sizeof(width)) != 0);
}

BEGIN_MESSAGE_MAP(CParamsBar, CMySizingControlBar)
	//{{AFX_MSG_MAP(CParamsBar)
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_INITDIALOG, OnInitDialog)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CParamsBar message handlers

int CParamsBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMySizingControlBar::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect	r(0, 0, 100, 100);
	int	style = WS_CHILD | LVS_REPORT | LVS_NOSORTHEADER;
	if (!m_List.Create(style, r, this, IDC_PARAMS_CTRL))
		return -1;

	return 0;
}

LRESULT CParamsBar::OnInitDialog(WPARAM wParam, LPARAM lParam)
{
	m_List.Init();
	LoadColumnWidths();
	m_List.ShowWindow(SW_SHOW);
	return TRUE;
}

void CParamsBar::OnDestroy() 
{
	StoreColumnWidths();
	CMySizingControlBar::OnDestroy();
}

void CParamsBar::OnSize(UINT nType, int cx, int cy)
{
	CMySizingControlBar::OnSize(nType, cx, cy);
	m_List.MoveWindow(0, 0, cx, cy);
}

void CParamsBar::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	CMySizingControlBar::OnWindowPosChanged(lpwndpos);
	if ((lpwndpos->flags & SWP_SHOWWINDOW) && m_Dirty
	&& theApp.GetMain()->GetView() != NULL) {
		m_List.Update();
		m_Dirty = FALSE;
	}
}
