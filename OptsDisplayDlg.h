// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      16feb09	initial version
		01		31mar09	make UpdateDisplayList public

        display options page
 
*/

#if !defined(AFX_OPTSDISPLAYDLG_H__AB164691_D56D_4B3C_B0DD_3B5FF831F055__INCLUDED_)
#define AFX_OPTSDISPLAYDLG_H__AB164691_D56D_4B3C_B0DD_3B5FF831F055__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptsDisplayDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptsDisplayDlg dialog

#include "DDEnumObj.h"

class COptionsInfo;

class COptsDisplayDlg : public CPropertyPage
{
// Construction
public:
	COptsDisplayDlg(COptionsInfo& Info);
	~COptsDisplayDlg();

// Attributes
	bool	GetDisplayInfo(CDDEnumObj& DDObj) const;

// Operations
	void	OnResetAll();
	void	UpdateDisplayList();

// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(COptsDisplayDlg)
	public:
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(COptsDisplayDlg)
	enum { IDD = IDD_OPTS_DISPLAY };
	CComboBox	m_DisplayCombo;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(COptsDisplayDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeDisplayCombo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Constants

// Member data
	COptionsInfo& m_oi;			// reference to parent's options info 
	CDDEnumObjArray	m_DDObj;	// array of enumerated display devices
	int		m_CurDisplay;		// index of current display, or -1 for default
	CString	m_DisplayName;		// name of current display

// Helpers
	void	UpdateUI();
	void	UpdateCurDisplay();
	void	UpdateScreenSize();
	void	SetCurDisplay(int DispIdx);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTSDISPLAYDLG_H__AB164691_D56D_4B3C_B0DD_3B5FF831F055__INCLUDED_)
