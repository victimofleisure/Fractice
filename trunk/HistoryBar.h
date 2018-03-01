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

#if !defined(AFX_HISTORYBAR_H__DF8CAC05_A18B_4898_93FC_8FD9D24B6984__INCLUDED_)
#define AFX_HISTORYBAR_H__DF8CAC05_A18B_4898_93FC_8FD9D24B6984__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// HistoryBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CHistoryBar dialog

#include "MySizingControlBar.h"
#include "HistoryView.h"

class CHistoryBar : public CMySizingControlBar
{
	DECLARE_DYNAMIC(CHistoryBar);
// Construction
public:
	CHistoryBar();

// Attributes
	void	SetView(CHistoryView *View);
	CHistoryView	*GetView();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHistoryBar)
	//}}AFX_VIRTUAL

// Implementation
protected:
// Generated message map functions
	//{{AFX_MSG(CHistoryBar)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	afx_msg LRESULT OnInitDialog(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

// Member data
	CHistoryView	*m_View;

// Helpers
};

inline CHistoryView *CHistoryBar::GetView()
{
	return(m_View);
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HISTORYBAR_H__DF8CAC05_A18B_4898_93FC_8FD9D24B6984__INCLUDED_)
