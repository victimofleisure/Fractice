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

#if !defined(AFX_PARAMSBAR_H__DF8CAC05_A18B_4898_93FC_8FD9D24B6984__INCLUDED_)
#define AFX_PARAMSBAR_H__DF8CAC05_A18B_4898_93FC_8FD9D24B6984__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ParamsBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CParamsBar dialog

#include "MySizingControlBar.h"
#include "ParamsView.h"

class CParamsBar : public CMySizingControlBar
{
	DECLARE_DYNAMIC(CParamsBar);
// Construction
public:
	CParamsBar();

// Attributes
	void	SetView(CFracticeView *View);
	CParamsView&	GetList();

// Operations
	void	Update();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CParamsBar)
	//}}AFX_VIRTUAL

// Implementation
protected:
// Generated message map functions
	//{{AFX_MSG(CParamsBar)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnWindowPosChanged(WINDOWPOS* lpwndpos);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	afx_msg LRESULT OnInitDialog(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

// Constants
	enum {
		COLUMNS = CParamsCtrl::COLUMNS
	};

// Member data
	CParamsView	m_List;		// list of parameters
	bool	m_Dirty;		// true if parameters need updating

// Helpers
	bool	LoadColumnWidths();
	bool	StoreColumnWidths();
};

inline void CParamsBar::SetView(CFracticeView *View)
{
	m_List.SetView(View);
}

inline CParamsView&	CParamsBar::GetList()
{
	return(m_List);
}

inline void CParamsBar::Update()
{
	if (FastIsVisible())
		m_List.Update();
	else
		m_Dirty = TRUE;
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PARAMSBAR_H__DF8CAC05_A18B_4898_93FC_8FD9D24B6984__INCLUDED_)
