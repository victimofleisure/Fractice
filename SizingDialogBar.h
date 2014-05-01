// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      02feb09	initial version
		01		05jan10	standardize OnInitDialog prototype

		sizable dialog bar base class
 
*/

#if !defined(AFX_SIZINGDIALOGBAR_H__DF8CAC05_A18B_4898_93FC_8FD9D24B6984__INCLUDED_)
#define AFX_SIZINGDIALOGBAR_H__DF8CAC05_A18B_4898_93FC_8FD9D24B6984__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SizingDialogBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSizingDialogBar dialog

#include "MySizingControlBar.h"

class CFracticeView;

class CSizingDialogBar : public CMySizingControlBar
{
	DECLARE_DYNAMIC(CSizingDialogBar);
// Construction
public:
	CSizingDialogBar();

// Attributes
	CDialog	*GetDlg();
	void	SetDlg(CDialog *pDlg, UINT ResID);

// Operations
	void	Update();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSizingDialogBar)
	//}}AFX_VIRTUAL

// Implementation
protected:
// Generated message map functions
	//{{AFX_MSG(CSizingDialogBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnWindowPosChanged(WINDOWPOS* lpwndpos);
	//}}AFX_MSG
	afx_msg LRESULT OnInitDialog(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

// Constants
	static const SIZE	m_DlgMargin;	// dialog margin offset within bar

// Member data
	CDialog	*m_pDlg;		// pointer to child dialog
	UINT	m_ResID;		// child dialog's resource ID
	bool	m_Dirty;		// true if dialog needs updating
};

inline CDialog *CSizingDialogBar::GetDlg()
{
	return(m_pDlg);
}

inline void CSizingDialogBar::Update()
{
	if (FastIsVisible())
		m_pDlg->SendMessage(UWM_DLGBARUPDATE);
	else
		m_Dirty = TRUE;
}

inline void CSizingDialogBar::SetDlg(CDialog *pDlg, UINT ResID)
{
	m_pDlg = pDlg;
	m_ResID = ResID;
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SIZINGDIALOGBAR_H__DF8CAC05_A18B_4898_93FC_8FD9D24B6984__INCLUDED_)
