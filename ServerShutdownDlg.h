// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version

        server shutdown dialog
 
*/

#if !defined(AFX_SERVERSHUTDOWNDLG_H__8D0F82C5_0246_49E5_B4D8_70AC29BE4C29__INCLUDED_)
#define AFX_SERVERSHUTDOWNDLG_H__8D0F82C5_0246_49E5_B4D8_70AC29BE4C29__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ServerShutdownDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CServerShutdownDlg dialog

class CServerShutdownDlg : public CDialog
{
	DECLARE_DYNAMIC(CServerShutdownDlg);
// Construction
public:
	CServerShutdownDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CServerShutdownDlg)
	enum { IDD = IDD_SERVER_SHUTDOWN };
	int		m_Action;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CServerShutdownDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CServerShutdownDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SERVERSHUTDOWNDLG_H__8D0F82C5_0246_49E5_B4D8_70AC29BE4C29__INCLUDED_)
