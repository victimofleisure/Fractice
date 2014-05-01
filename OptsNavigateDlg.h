// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      08feb09	initial version
		01		01mar09	add scroll delta

        navigate options page
 
*/

#if !defined(AFX_OPTSNAVIGATEDLG_H__AB164691_D56D_4B3C_B0DD_3B5FF831F055__INCLUDED_)
#define AFX_OPTSNAVIGATEDLG_H__AB164691_D56D_4B3C_B0DD_3B5FF831F055__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptsNavigateDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptsNavigateDlg dialog

class COptionsInfo;

class COptsNavigateDlg : public CPropertyPage
{
// Construction
public:
	COptsNavigateDlg(COptionsInfo& Info);
	~COptsNavigateDlg();

// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(COptsNavigateDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(COptsNavigateDlg)
	enum { IDD = IDD_OPTS_NAVIGATE };
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(COptsNavigateDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnGridColor();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Constants
	enum {
		MIN_SCROLL_DELTA = 1,
		MAX_SCROLL_DELTA = 200,
		MIN_MRU_ITEMS = 1,
		MAX_MRU_ITEMS = 9,	// don't increase this, you will regret
	};

// Member data
	COptionsInfo& m_oi;			// reference to parent's options info 

// Helpers
	void	UpdateUI();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTSNAVIGATEDLG_H__AB164691_D56D_4B3C_B0DD_3B5FF831F055__INCLUDED_)
