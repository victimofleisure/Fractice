// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version

		add server list dialog
 
*/

#if !defined(AFX_ADDSERVERLISTDLG_H__4312E91F_6F8E_4257_BCD9_2956E65C8D16__INCLUDED_)
#define AFX_ADDSERVERLISTDLG_H__4312E91F_6F8E_4257_BCD9_2956E65C8D16__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AddServerListDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAddServerListDlg dialog

class CAddServerListDlg : public CDialog
{
	DECLARE_DYNAMIC(CAddServerListDlg);
// Construction
public:
	CAddServerListDlg(CWnd* pParent = NULL);   // standard constructor
	~CAddServerListDlg();

// Attributes
	CString	GetListPath() const;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAddServerListDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CAddServerListDlg)
	enum { IDD = IDD_ADD_SERVER_LIST };
	CEdit	m_PathEdit;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CAddServerListDlg)
	afx_msg void OnBrowse();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Data members
	CString	m_ListPath;
};

inline CString CAddServerListDlg::GetListPath() const
{
	return(m_ListPath);
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADDSERVERLISTDLG_H__4312E91F_6F8E_4257_BCD9_2956E65C8D16__INCLUDED_)
