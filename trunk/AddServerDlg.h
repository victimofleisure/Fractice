// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version

		add server dialog
 
*/

#if !defined(AFX_ADDSERVERDLG_H__553E0E58_81B3_4BE3_87EB_8655E7B4179B__INCLUDED_)
#define AFX_ADDSERVERDLG_H__553E0E58_81B3_4BE3_87EB_8655E7B4179B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AddServerDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAddServerDlg dialog

class CAddServerDlg : public CDialog
{
	DECLARE_DYNAMIC(CAddServerDlg);
// Construction
public:
	CAddServerDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAddServerDlg)
	enum { IDD = IDD_ADD_SERVER };
	CString	m_HostName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAddServerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Generated message map functions
	//{{AFX_MSG(CAddServerDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADDSERVERDLG_H__553E0E58_81B3_4BE3_87EB_8655E7B4179B__INCLUDED_)
