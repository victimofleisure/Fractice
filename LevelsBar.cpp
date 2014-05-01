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
		
		levels control bar
 
*/

// LevelsBar.cpp : implementation file
//

#include "stdafx.h"
#include "Fractice.h"
#include "MainFrm.h"
#include "LevelsBar.h"
#include "PersistDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLevelsBar dialog

IMPLEMENT_DYNAMIC(CLevelsBar, CMySizingControlBar);

#define RK_SCALING_TYPE	_T("LevsScalingType")
#define RK_SHOW_AXIS	_T("LevsShowAxis")

CLevelsBar::CLevelsBar()
{
	//{{AFX_DATA_INIT(CLevelsBar)
	//}}AFX_DATA_INIT
    m_szHorz = CSize(400, 87);	// size of bar when horizontally docked
	m_IsDirty = FALSE;
}

BEGIN_MESSAGE_MAP(CLevelsBar, CMySizingControlBar)
	//{{AFX_MSG_MAP(CLevelsBar)
	ON_WM_SIZE()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_LEVS_SHOW_AXIS, OnShowAxis)
	ON_UPDATE_COMMAND_UI(ID_LEVS_SHOW_AXIS, OnUpdateShowAxis)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_INITDIALOG, OnInitDialog)
	ON_COMMAND_RANGE(ID_LEVS_SCALING_TYPE, ID_LEVS_SCALING_TYPE2, OnScalingType)
	ON_UPDATE_COMMAND_UI_RANGE(ID_LEVS_SCALING_TYPE, ID_LEVS_SCALING_TYPE2, OnUpdateScalingType)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLevelsBar message handlers

LRESULT CLevelsBar::OnInitDialog(WPARAM wParam, LPARAM lParam)
{
	CRect	r(0, 0, 0, 0);
	m_Levels.Create(_T(""), WS_CHILD | WS_VISIBLE, r, this, ID_LEVELS_VIEW);
	m_Levels.ModifyStyleEx(0, WS_EX_CLIENTEDGE);
	if (IsVisible()) {
		GetClientRect(r);
		OnSize(0, r.Width(), r.Height());
	}
	m_Levels.SetScalingType(theApp.RdRegInt(RK_SCALING_TYPE, CLevelsCtrl::ST_LINEAR));
	m_Levels.ShowAxis(theApp.RdRegBool(RK_SHOW_AXIS, FALSE));
	return TRUE;
}

void CLevelsBar::OnDestroy() 
{
	theApp.WrRegInt(RK_SCALING_TYPE, m_Levels.GetScalingType());
	theApp.WrRegBool(RK_SHOW_AXIS, m_Levels.IsAxisShowing());
	CMySizingControlBar::OnDestroy();
}

void CLevelsBar::OnSize(UINT nType, int cx, int cy)
{
	CMySizingControlBar::OnSize(nType, cx, cy);
	if (IsWindow(m_Levels)) {
		m_Levels.SetWindowPos(NULL, 0, 0, cx, cy, 
			SWP_NOMOVE | SWP_NOZORDER | SWP_NOCOPYBITS);
	}
}

void CLevelsBar::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	CMySizingControlBar::OnWindowPosChanged(lpwndpos);
	if ((lpwndpos->flags & SWP_SHOWWINDOW) && m_IsDirty
	&& theApp.GetMain()->GetView() != NULL) {
		m_Levels.CalcLevels();
		m_IsDirty = FALSE;
	}
}

void CLevelsBar::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	CMenu	menu, *mp;
	menu.LoadMenu(IDR_LEVELS);
	mp = menu.GetSubMenu(0);
	CPersistDlg::UpdateMenu(this, mp);
	mp->TrackPopupMenu(0, point.x, point.y, this);
}

void CLevelsBar::OnScalingType(UINT nID)
{
	int	Type = nID - ID_LEVS_SCALING_TYPE;
	m_Levels.SetScalingType(Type);
}

void CLevelsBar::OnUpdateScalingType(CCmdUI* pCmdUI)
{
	int	Type = pCmdUI->m_nID - ID_LEVS_SCALING_TYPE;
	pCmdUI->SetRadio(Type == m_Levels.GetScalingType());
}

void CLevelsBar::OnShowAxis()
{
	m_Levels.ShowAxis(!m_Levels.IsAxisShowing());
}

void CLevelsBar::OnUpdateShowAxis(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_Levels.IsAxisShowing());
}
