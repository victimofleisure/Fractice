// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      03mar09	initial version
		01		31mar09	add refresh button

        display options page
 
*/

#if !defined(AFX_OPTSMIDIDLG_H__AB164691_D56D_4B3C_B0DD_3B5FF831F055__INCLUDED_)
#define AFX_OPTSMIDIDLG_H__AB164691_D56D_4B3C_B0DD_3B5FF831F055__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptsMidiDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptsMidiDlg dialog

#include "DDEnumObj.h"

class COptionsInfo;

class COptsMidiDlg : public CPropertyPage
{
// Construction
public:
	COptsMidiDlg(COptionsInfo& Info);
	~COptsMidiDlg();

// Constants

// Attributes
	int		GetMidiDevice() const;
	CString	GetMidiDeviceName() const;
	bool	SetMidiDeviceName(const CString& DevName);

// Operations
	void	OnResetAll();

// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(COptsMidiDlg)
	public:
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(COptsMidiDlg)
	enum { IDD = IDD_OPTS_MIDI };
	CComboBox	m_MidiDevCombo;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(COptsMidiDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnMidiRefresh();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Constants

// Member data
	COptionsInfo& m_oi;			// reference to parent's options info 
	CStringArray	m_DevList;	// MIDI device list
	int		m_MidiDev;			// index of MIDI device in m_DevList

// Helpers
	void	UpdateUI();
	void	UpdateDevCombo();
};

inline int COptsMidiDlg::GetMidiDevice() const
{
	return(m_MidiDev);
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTSMIDIDLG_H__AB164691_D56D_4B3C_B0DD_3B5FF831F055__INCLUDED_)
