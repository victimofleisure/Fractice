// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      16feb09	initial version
		01		31mar09	in UpdateDisplayList, add IsWindow test

        display options page
 
*/

// OptsDisplayDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Fractice.h"
#include "OptionsDlg.h"
#include "OptsDisplayDlg.h"
#include "Mixer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptsDisplayDlg property page

#define RK_DISPLAY_NAME		_T("DisplayName")
#define RK_FRAME_RATE		_T("FrameRate")

COptsDisplayDlg::COptsDisplayDlg(COptionsInfo& Info) 
	: CPropertyPage(IDD),
	m_oi(Info)
{
	//{{AFX_DATA_INIT(COptsDisplayDlg)
	//}}AFX_DATA_INIT
	m_DisplayName = theApp.RdRegString(RK_DISPLAY_NAME);
	m_CurDisplay = -1;
	CMixer::EnumDDObjects(m_DDObj);
	UpdateCurDisplay();
	theApp.RdReg2UInt(RK_FRAME_RATE, m_oi.m_FrameRate);
}

COptsDisplayDlg::~COptsDisplayDlg()
{
	theApp.WrRegString(RK_DISPLAY_NAME, m_DisplayName);
	theApp.WrRegInt(RK_FRAME_RATE, m_oi.m_FrameRate);
}

void COptsDisplayDlg::UpdateUI()
{
	UpdateData();	// get controls
}

bool COptsDisplayDlg::GetDisplayInfo(CDDEnumObj& DDObj) const
{
	if (m_CurDisplay < 0)
		return(FALSE);
	DDObj = m_DDObj[m_CurDisplay];
	return(TRUE);
}

void COptsDisplayDlg::UpdateCurDisplay()
{
	int	objs = m_DDObj.GetSize();
	m_CurDisplay = -1;
	int	primary = -1;
	for (int i = 0; i < objs; i++) {
		const CDDEnumObj&	obj = m_DDObj[i];
		if (m_DisplayName == obj.m_DriverName)
			m_CurDisplay = i;
		if (!obj.m_hMonitor)
			primary = i;
	}
	if (m_CurDisplay < 0) {
		m_CurDisplay = primary;
		m_DisplayName = m_DDObj[primary].m_DriverName;
	}
}

void COptsDisplayDlg::SetCurDisplay(int DispIdx)
{
	if (DispIdx >= 0)
		m_DisplayName = m_DDObj[DispIdx].m_DriverName;
	else
		m_DisplayName.Empty();
	UpdateCurDisplay();
}

void COptsDisplayDlg::UpdateScreenSize()
{
	CString	s;
	int	sel = m_DisplayCombo.GetCurSel();
	if (sel >= 0) {
		CRect	rc;
		m_DDObj[sel].GetScreenCoords(rc);
		s.Format(_T("%d x %d"), rc.Width(), rc.Height());
	}
	GetDlgItem(IDC_OPTS_DISPLAY_SIZE)->SetWindowText(s);
}

void COptsDisplayDlg::UpdateDisplayList()
{
	CMixer::EnumDDObjects(m_DDObj);
	UpdateCurDisplay();
	if (IsWindow(m_hWnd)) {
		int	objs = m_DDObj.GetSize();
		for (int i = 0; i < objs; i++) {
			const CDDEnumObj&	obj = m_DDObj[i];
			CString	s(obj.m_DriverName);
			s.TrimLeft(_T("\\."));	// remove leading delimiters
			m_DisplayCombo.AddString(s + _T(" / ") + obj.m_DriverDescription);
		}
		m_DisplayCombo.SetCurSel(m_CurDisplay);
		UpdateScreenSize();
	}
}

void COptsDisplayDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptsDisplayDlg)
	DDX_Control(pDX, IDC_OPTS_DISPLAY_COMBO, m_DisplayCombo);
	//}}AFX_DATA_MAP
	DDX_Text(pDX, IDC_OPTS_FRAME_RATE, m_oi.m_FrameRate);
	DDV_MinMaxInt(pDX, m_oi.m_FrameRate, 1, 100);
}

BEGIN_MESSAGE_MAP(COptsDisplayDlg, CPropertyPage)
	//{{AFX_MSG_MAP(COptsDisplayDlg)
	ON_CBN_SELCHANGE(IDC_OPTS_DISPLAY_COMBO, OnSelchangeDisplayCombo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptsDisplayDlg message handlers

BOOL COptsDisplayDlg::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	UpdateDisplayList();
	UpdateUI();

	return TRUE;
}

void COptsDisplayDlg::OnOK() 
{
	SetCurDisplay(m_DisplayCombo.GetCurSel());
	CPropertyPage::OnOK();
}

void COptsDisplayDlg::OnResetAll()
{
	SetCurDisplay(-1);
}

void COptsDisplayDlg::OnSelchangeDisplayCombo() 
{
	UpdateScreenSize();
}
