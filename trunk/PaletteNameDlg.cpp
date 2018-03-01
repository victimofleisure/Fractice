// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version

        palette dialog
 
*/

// PaletteNameDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "PaletteNameDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPaletteNameDlg dialog

IMPLEMENT_DYNAMIC(CPaletteNameDlg, CDialog);

CPaletteNameDlg::CPaletteNameDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
	//{{AFX_DATA_INIT(CPaletteNameDlg)
	m_Name = _T("");
	//}}AFX_DATA_INIT
}

void CPaletteNameDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPaletteNameDlg)
	DDX_Text(pDX, IDC_PALN_NAME_EDIT, m_Name);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPaletteNameDlg, CDialog)
	//{{AFX_MSG_MAP(CPaletteNameDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPaletteNameDlg message handlers

BOOL CPaletteNameDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
