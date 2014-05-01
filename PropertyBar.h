// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      28feb09	initial version
		01		05jan10	standardize OnInitDialog prototype

		property bar base class
 
*/

#if !defined(AFX_PROPERTYBAR_H__DF8CAC05_A18B_4898_93FC_8FD9D24B6984__INCLUDED_)
#define AFX_PROPERTYBAR_H__DF8CAC05_A18B_4898_93FC_8FD9D24B6984__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PropertyBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPropertyBar dialog

#include "MySizingControlBar.h"

class CFracticeView;

class CPropertyBar : public CMySizingControlBar
{
	DECLARE_DYNAMIC(CPropertyBar);
// Construction
public:
	CPropertyBar();

// Attributes
	CPropertySheet	*GetDlg();
	void	SetDlg(CPropertySheet *pDlg);

// Operations
	void	Update();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPropertyBar)
	//}}AFX_VIRTUAL

// Implementation
protected:
// Generated message map functions
	//{{AFX_MSG(CPropertyBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnWindowPosChanged(WINDOWPOS* lpwndpos);
	//}}AFX_MSG
	afx_msg LRESULT OnInitDialog(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

// Constants
	static const SIZE	m_DlgMargin;	// dialog margin offset within bar

// Member data
	CPropertySheet	*m_pDlg;	// pointer to child property sheet
	bool	m_Dirty;		// true if property sheet needs updating
};

inline CPropertySheet *CPropertyBar::GetDlg()
{
	return(m_pDlg);
}

inline void CPropertyBar::Update()
{
	if (FastIsVisible())
		m_pDlg->SendMessage(UWM_DLGBARUPDATE);
	else
		m_Dirty = TRUE;
}

inline void CPropertyBar::SetDlg(CPropertySheet *pDlg)
{
	m_pDlg = pDlg;
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROPERTYBAR_H__DF8CAC05_A18B_4898_93FC_8FD9D24B6984__INCLUDED_)
