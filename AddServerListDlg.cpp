// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version

		add server list dialog
 
*/

// AddServerListDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Fractice.h"
#include "AddServerListDlg.h"
#include "shlwapi.h"
#include "MultiFileDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAddServerListDlg dialog

IMPLEMENT_DYNAMIC(CAddServerListDlg, CDialog);

#define RK_SERVER_LIST_PATH	_T("ServerListPath")

CAddServerListDlg::CAddServerListDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
	//{{AFX_DATA_INIT(CAddServerListDlg)
	//}}AFX_DATA_INIT
	m_ListPath = theApp.RdRegString(RK_SERVER_LIST_PATH);
}

CAddServerListDlg::~CAddServerListDlg()
{
	theApp.WrRegString(RK_SERVER_LIST_PATH, m_ListPath);
}

void CAddServerListDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddServerListDlg)
	DDX_Control(pDX, IDC_ASRVL_PATH_EDIT, m_PathEdit);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAddServerListDlg, CDialog)
	//{{AFX_MSG_MAP(CAddServerListDlg)
	ON_BN_CLICKED(IDC_ASRVL_BROWSE, OnBrowse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddServerListDlg message handlers

BOOL CAddServerListDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_PathEdit.SetWindowText(m_ListPath);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAddServerListDlg::OnBrowse() 
{
	CString	Path;
	m_PathEdit.GetWindowText(Path);
	LPCTSTR	FileName = PathFindFileName(Path);
	CFileDialog	fd(TRUE, _T(".txt"), FileName, 
		OFN_HIDEREADONLY | OFN_NOCHANGEDIR, LDS(IDS_TEXT_FILTER));
	fd.m_ofn.lpstrInitialDir = Path;
	if (fd.DoModal() == IDOK)
		m_PathEdit.SetWindowText(fd.GetPathName());
}

void CAddServerListDlg::OnOK() 
{
	m_PathEdit.GetWindowText(m_ListPath);
	CDialog::OnOK();
}
