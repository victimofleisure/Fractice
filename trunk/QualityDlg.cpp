// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version

        quality dialog
 
*/

// QualityDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "QualityDlg.h"
#include "Snapshot.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CQualityDlg dialog

IMPLEMENT_DYNAMIC(CQualityDlg, CDialog);

CQualityDlg::CQualityDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
	//{{AFX_DATA_INIT(CQualityDlg)
	//}}AFX_DATA_INIT
	m_Value = 0;
}

void CQualityDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CQualityDlg)
	//}}AFX_DATA_MAP
	DDX_Text(pDX, IDC_QUALITY_VALUE_EDIT, m_Value);
	DDV_MinMaxUInt(pDX, m_Value, MIN_QUALITY, MAX_QUALITY);
}

BEGIN_MESSAGE_MAP(CQualityDlg, CDialog)
	//{{AFX_MSG_MAP(CQualityDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQualityDlg message handlers

