// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      08feb09	initial version

        palette options page
 
*/

#if !defined(AFX_OPTSPALETTEDLG_H__AB164691_D56D_4B3C_B0DD_3B5FF831F055__INCLUDED_)
#define AFX_OPTSPALETTEDLG_H__AB164691_D56D_4B3C_B0DD_3B5FF831F055__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptsPaletteDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptsPaletteDlg dialog

class COptionsInfo;

class COptsPaletteDlg : public CPropertyPage
{
// Construction
public:
	COptsPaletteDlg(COptionsInfo& Info);
	~COptsPaletteDlg();

// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(COptsPaletteDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(COptsPaletteDlg)
	enum { IDD = IDD_OPTS_PALETTE };
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(COptsPaletteDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Member data
	COptionsInfo& m_oi;			// reference to parent's options info 

// Helpers
	void	UpdateUI();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTSPALETTEDLG_H__AB164691_D56D_4B3C_B0DD_3B5FF831F055__INCLUDED_)
