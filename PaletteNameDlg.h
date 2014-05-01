// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version

        palette dialog
 
*/

#if !defined(AFX_PALETTENAMEDLG_H__7529D409_0654_4894_B3FA_749AE04598E7__INCLUDED_)
#define AFX_PALETTENAMEDLG_H__7529D409_0654_4894_B3FA_749AE04598E7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PaletteNameDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPaletteNameDlg dialog

class CPaletteNameDlg : public CDialog
{
	DECLARE_DYNAMIC(CPaletteNameDlg);
// Construction
public:
	CPaletteNameDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPaletteNameDlg)
	enum { IDD = IDD_PALETTE_NAME };
	CString	m_Name;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPaletteNameDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Generated message map functions
	//{{AFX_MSG(CPaletteNameDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PALETTENAMEDLG_H__7529D409_0654_4894_B3FA_749AE04598E7__INCLUDED_)
