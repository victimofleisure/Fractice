// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14mar09	initial version

        transform history dialog
 
*/

#if !defined(AFX_TRANSFORMDLG_H__F1CF9D56_CB75_4799_A51B_322A2ADBEC49__INCLUDED_)
#define AFX_TRANSFORMDLG_H__F1CF9D56_CB75_4799_A51B_322A2ADBEC49__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TransformDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTransformDlg dialog

#include "ParamsCtrl.h"

class CFracticeView;

class CTransformDlg : public CDialog
{
	DECLARE_DYNAMIC(CTransformDlg);
// Construction
public:
	CTransformDlg(CFracticeView& View, CWnd* pParent = NULL);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTransformDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CTransformDlg)
	enum { IDD = IDD_TRANSFORM };
	CParamsCtrl	m_ParamsCtrl;
	CString	m_PalettePath;
	BOOL	m_Render;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CTransformDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnPaletteBrowse();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Member data
	CFracticeView&	m_View;			// associated view
	bool	m_ParmMod[NUM_PARMS];	// for each parameter, true if changing it
	int		m_ParmModCount;			// number of parameters to be changed
	int		m_FilterIndex;			// file dialog filter index
	DPalette	m_NewPalette;		// if m_PalettePath not empty, palette to apply

// Helpers
	void	ReadParams();
	bool	ReadPalette();
	void	ApplyChanges();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TRANSFORMDLG_H__F1CF9D56_CB75_4799_A51B_322A2ADBEC49__INCLUDED_)
