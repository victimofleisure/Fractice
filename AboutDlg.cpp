// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		09jan10	show bignum version

        about dialog
 
*/

#include "stdafx.h"
#include "Resource.h"
#include "AboutDlg.h"
#include "VersionInfo.h"
#include "BigNum.h"	// for version

CAboutDlg::CAboutDlg() 
	: CDialog(IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	DDX_Control(pDX, IDC_ABOUT_LICENSE, m_License);
	DDX_Control(pDX, IDC_ABOUT_URL, m_AboutUrl);
	DDX_Control(pDX, IDC_ABOUT_TEXT, m_AboutText);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg message handlers

BOOL CAboutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	VS_FIXEDFILEINFO	AppInfo;
	CVersionInfo::GetFileInfo(AppInfo, NULL);
	CString	s;
	s.Format(IDS_APP_ABOUT_TEXT,
		HIWORD(AppInfo.dwFileVersionMS), LOWORD(AppInfo.dwFileVersionMS),
		HIWORD(AppInfo.dwFileVersionLS), LOWORD(AppInfo.dwFileVersionLS),
		BigNum::GetLibName(), BigNum::GetVersion(), BigNum::GetVersionMinor(),
		BigNum::GetPatchLevel(), BigNum::GetOSName(), BigNum::GetCPUName());
	m_AboutText.SetWindowText(s);
	m_AboutUrl.SetUrl(LDS(IDS_APP_HOME_PAGE_URL));
	m_License.SetWindowText(LDS(IDS_APP_LICENSE));
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
