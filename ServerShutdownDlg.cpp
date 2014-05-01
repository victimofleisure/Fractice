// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version

        server shutdown dialog
 
*/

// ServerShutdownDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "ServerShutdownDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CServerShutdownDlg dialog

IMPLEMENT_DYNAMIC(CServerShutdownDlg, CDialog);

CServerShutdownDlg::CServerShutdownDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
	//{{AFX_DATA_INIT(CServerShutdownDlg)
	m_Action = 0;
	//}}AFX_DATA_INIT
}


void CServerShutdownDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CServerShutdownDlg)
	DDX_Radio(pDX, IDC_SRVSD_ACTION, m_Action);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CServerShutdownDlg, CDialog)
	//{{AFX_MSG_MAP(CServerShutdownDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CServerShutdownDlg message handlers
