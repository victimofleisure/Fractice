// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		29oct08	add title and folder args to ctor
		02		06jan10	W64: DoModal return value is 64-bit

        Save As dialog with enhanced support for multiple file types
 
*/

#if !defined(AFX_SAVEASDLG_H__EEE10EF0_3E57_4C15_BD7E_FA68591A7C88__INCLUDED_)
#define AFX_SAVEASDLG_H__EEE10EF0_3E57_4C15_BD7E_FA68591A7C88__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SaveAsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSaveAsDlg dialog

class CSaveAsDlg : public CFileDialog
{
	DECLARE_DYNAMIC(CSaveAsDlg);
// Construction
public:
	CSaveAsDlg(BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD	dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter = NULL,
		CWnd*	pParentWnd = NULL,
		LPCTSTR	lpszTitle = NULL,	// non-standard but useful
		CString	*psFolder = NULL);	// non-standard but useful

// Constants

// Attributes
	void	GetFolder(CString& Folder);

// Operations
	W64INT	DoModal();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSaveAsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog data
	//{{AFX_DATA(CSaveAsDlg)
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CSaveAsDlg)
	//}}AFX_MSG
	afx_msg	void OnTypeChange(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

// Member data
	CStringArray	m_Ext;	// array of file extensions, one per filter
	CString	m_Title;		// title for dialog caption bar
	CString	*m_psFolder;	// optional pointer to initial and final folder

// Helpers
	void	BuildExtList();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SAVEASDLG_H__EEE10EF0_3E57_4C15_BD7E_FA68591A7C88__INCLUDED_)
