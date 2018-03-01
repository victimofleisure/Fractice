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

#if !defined(AFX_LEVELSBAR_H__DF8CAC05_A18B_4898_93FC_8FD9D24B6984__INCLUDED_)
#define AFX_LEVELSBAR_H__DF8CAC05_A18B_4898_93FC_8FD9D24B6984__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LevelsBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLevelsBar dialog

#include "MySizingControlBar.h"
#include "LevelsCtrl.h"

class CLevelsBar : public CMySizingControlBar
{
	DECLARE_DYNAMIC(CLevelsBar);
// Construction
public:
	CLevelsBar();

// Attributes
	void	SetView(CFracticeView *View);

// Operations
	void	CalcLevels();
	void	SetEmpty();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLevelsBar)
	//}}AFX_VIRTUAL

// Implementation
protected:
// Generated message map functions
	//{{AFX_MSG(CLevelsBar)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnWindowPosChanged(WINDOWPOS* lpwndpos);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnShowAxis();
	afx_msg void OnUpdateShowAxis(CCmdUI* pCmdUI);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	afx_msg LRESULT OnInitDialog(WPARAM wParam, LPARAM lParam);
	afx_msg void OnScalingType(UINT nID);
	afx_msg void OnUpdateScalingType(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

// Member data
	CLevelsCtrl	m_Levels;	// levels control
	bool	m_IsDirty;		// true if levels are stale
};

inline void CLevelsBar::SetView(CFracticeView *View)
{
	m_Levels.SetView(View);
}

inline void CLevelsBar::CalcLevels()
{
	if (FastIsVisible())
		m_Levels.CalcLevels();
	else
		m_IsDirty = TRUE;
}

inline void CLevelsBar::SetEmpty()
{
	m_Levels.SetEmpty();
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LEVELSBAR_H__DF8CAC05_A18B_4898_93FC_8FD9D24B6984__INCLUDED_)
