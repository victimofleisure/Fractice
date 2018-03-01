// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version

        quality dialog
 
*/

#if !defined(AFX_QUALITYDLG_H__0C7CD4A9_4CD8_42D9_8EFC_5AC863CCEDDC__INCLUDED_)
#define AFX_QUALITYDLG_H__0C7CD4A9_4CD8_42D9_8EFC_5AC863CCEDDC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// QualityDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CQualityDlg dialog

class CQualityDlg : public CDialog
{
	DECLARE_DYNAMIC(CQualityDlg);
// Construction
public:
	CQualityDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CQualityDlg)
	enum { IDD = IDD_QUALITY };
	//}}AFX_DATA
	UINT	m_Value;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CQualityDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Generated message map functions
	//{{AFX_MSG(CQualityDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_QUALITYDLG_H__0C7CD4A9_4CD8_42D9_8EFC_5AC863CCEDDC__INCLUDED_)
