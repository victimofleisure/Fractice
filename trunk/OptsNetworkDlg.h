// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      08feb09	initial version
		01		12feb09	add network adapter combo box

        network options page
 
*/

#if !defined(AFX_OPTSNETWORKDLG_H__AB164691_D56D_4B3C_B0DD_3B5FF831F055__INCLUDED_)
#define AFX_OPTSNETWORKDLG_H__AB164691_D56D_4B3C_B0DD_3B5FF831F055__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptsNetworkDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptsNetworkDlg dialog

#include "NetAdapterInfo.h"

class COptionsInfo;

class COptsNetworkDlg : public CPropertyPage
{
// Construction
public:
	COptsNetworkDlg(COptionsInfo& Info);
	~COptsNetworkDlg();

// Attributes
	CString	GetNicDescription() const;
	CString	GetNicIPAddress() const;

// Operations
	void	OnResetAll();

// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(COptsNetworkDlg)
	public:
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(COptsNetworkDlg)
	enum { IDD = IDD_OPTS_NETWORK };
	CComboBox	m_NicCombo;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(COptsNetworkDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnNetDefPortChk();
	afx_msg void OnNetAdapterProps();
	afx_msg void OnSelchangeNicCombo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Constants
	enum {
		DEFAULT_NIC = -1
	};

// Member data
	COptionsInfo& m_oi;		// reference to parent's options info 
	CNetAdapterInfoArray	m_NicInfo;	// array of network adapter info
	int		m_CurNic;		// index of current adapter, or -1 for default
	CString	m_NicDescrip;	// current adapter's description; may be empty
	CString	m_NicIPAddr;	// current adapter's IP address; may be empty

// Helpers
	void	UpdateUI();
	bool	EnumNics();
	void	UpdateNicInfo();
	void	InitNicCombo();
	void	ReadNicCombo();
	static	CString	FormatIPAddrList(const CNetAdapterInfo::CNetIPAddrList& List);
	static	CString	FormatNetAdapterProps(const CNetAdapterInfo& Info);
};

inline CString COptsNetworkDlg::GetNicDescription() const
{
	return(m_NicDescrip);
}

inline CString COptsNetworkDlg::GetNicIPAddress() const
{
	return(m_NicIPAddr);
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTSNETWORKDLG_H__AB164691_D56D_4B3C_B0DD_3B5FF831F055__INCLUDED_)
