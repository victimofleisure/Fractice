// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version

		add server dialog
 
*/

// AddServerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "AddServerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAddServerDlg dialog

IMPLEMENT_DYNAMIC(CAddServerDlg, CDialog);

CAddServerDlg::CAddServerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
	//{{AFX_DATA_INIT(CAddServerDlg)
	m_HostName = _T("");
	//}}AFX_DATA_INIT
}

void CAddServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddServerDlg)
	DDX_Text(pDX, IDC_ASRV_HOSTNAME_EDIT, m_HostName);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAddServerDlg, CDialog)
	//{{AFX_MSG_MAP(CAddServerDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddServerDlg message handlers
