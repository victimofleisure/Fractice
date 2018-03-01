// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      08feb09	initial version

        document options page
 
*/

// OptsDocumentDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Fractice.h"
#include "OptionsDlg.h"
#include "OptsDocumentDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptsDocumentDlg property page

#define RK_DEF_DOC_FMT		_T("DefDocFmt")
#define RK_THUMB_SIZE		_T("ThumbSize")
#define RK_SAVE_CHGS_WARN	_T("SaveChgsWarn")

COptsDocumentDlg::COptsDocumentDlg(COptionsInfo& Info)
	: CPropertyPage(IDD),
	m_oi(Info)
{
	//{{AFX_DATA_INIT(COptsDocumentDlg)
	//}}AFX_DATA_INIT
	theApp.RdRegStruct(RK_THUMB_SIZE, m_oi.m_ThumbSize, m_oi.m_ThumbSize);
	theApp.RdReg2Int(RK_DEF_DOC_FMT, m_oi.m_DefDocFmt);
	theApp.RdReg2Int(RK_SAVE_CHGS_WARN, m_oi.m_SaveChgsWarn);
}

COptsDocumentDlg::~COptsDocumentDlg()
{
	theApp.WrRegStruct(RK_THUMB_SIZE, m_oi.m_ThumbSize);
	theApp.WrRegInt(RK_DEF_DOC_FMT, m_oi.m_DefDocFmt);
	theApp.WrRegInt(RK_SAVE_CHGS_WARN, m_oi.m_SaveChgsWarn);
}

void COptsDocumentDlg::UpdateUI()
{
	UpdateData();	// get controls
}

void COptsDocumentDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptsDocumentDlg)
	//}}AFX_DATA_MAP
	DDX_Radio(pDX, IDC_OPTS_FILE_FORMAT, m_oi.m_DefDocFmt);
	DDX_Text(pDX, IDC_OPTS_THUMB_WIDTH_EDIT, m_oi.m_ThumbSize.cx);
	DDV_MinMaxInt(pDX, m_oi.m_ThumbSize.cx, 
		COptionsInfo::MIN_THUMB_WIDTH, COptionsInfo::MAX_THUMB_WIDTH);
	DDX_Text(pDX, IDC_OPTS_THUMB_HEIGHT_EDIT, m_oi.m_ThumbSize.cy);
	DDV_MinMaxInt(pDX, m_oi.m_ThumbSize.cy, 
		COptionsInfo::MIN_THUMB_HEIGHT, COptionsInfo::MAX_THUMB_HEIGHT);
	DDX_Check(pDX, IDC_OPTS_SAVE_CHGS_WARN_CHK, m_oi.m_SaveChgsWarn);
}

BEGIN_MESSAGE_MAP(COptsDocumentDlg, CPropertyPage)
	//{{AFX_MSG_MAP(COptsDocumentDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptsDocumentDlg message handlers

BOOL COptsDocumentDlg::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	UpdateUI();

	return TRUE;
}
