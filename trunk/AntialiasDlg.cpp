// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version

        antialias dialog
 
*/

// AntialiasDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "AntialiasDlg.h"
#include "Snapshot.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAntialiasDlg dialog

IMPLEMENT_DYNAMIC(CAntialiasDlg, CDialog);

CAntialiasDlg::CAntialiasDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
	//{{AFX_DATA_INIT(CAntialiasDlg)
	//}}AFX_DATA_INIT
	m_Value = 0;
}

void CAntialiasDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAntialiasDlg)
	//}}AFX_DATA_MAP
	DDX_Text(pDX, IDC_ANTIALIAS_VALUE_EDIT, m_Value);
	DDV_MinMaxUInt(pDX, m_Value, MIN_ANTIALIAS, MAX_ANTIALIAS);
}

BEGIN_MESSAGE_MAP(CAntialiasDlg, CDialog)
	//{{AFX_MSG_MAP(CAntialiasDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAntialiasDlg message handlers

