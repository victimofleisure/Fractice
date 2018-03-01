// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      08feb09	initial version
		01		22feb09	add palette tween time

        palette options page
 
*/

// OptsPaletteDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Fractice.h"
#include "OptionsDlg.h"
#include "OptsPaletteDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptsPaletteDlg property page

#define RK_PAL_FIRST_BKGND	_T("PalFirstBkgnd")
#define RK_PAL_SMOOTH_WRAP	_T("PalSmoothWrap")
#define RK_PAL_EXPORT_SIZE	_T("PalExportSize")
#define RK_HUE_ROTATION		_T("HueRotation")

COptsPaletteDlg::COptsPaletteDlg(COptionsInfo& Info)
	: CPropertyPage(IDD),
	m_oi(Info)
{
	//{{AFX_DATA_INIT(COptsPaletteDlg)
	//}}AFX_DATA_INIT
	theApp.RdReg2Int(RK_PAL_FIRST_BKGND, m_oi.m_PalFirstBkgnd);
	theApp.RdReg2Int(RK_PAL_SMOOTH_WRAP, m_oi.m_PalSmoothWrap);
	theApp.RdReg2UInt(RK_PAL_EXPORT_SIZE, m_oi.m_PalExportSize);
	theApp.RdReg2Int(RK_HUE_ROTATION, m_oi.m_HueRotation);
}

COptsPaletteDlg::~COptsPaletteDlg()
{
	theApp.WrRegInt(RK_PAL_FIRST_BKGND, m_oi.m_PalFirstBkgnd);
	theApp.WrRegInt(RK_PAL_SMOOTH_WRAP, m_oi.m_PalSmoothWrap);
	theApp.WrRegInt(RK_PAL_EXPORT_SIZE, m_oi.m_PalExportSize);
	theApp.WrRegInt(RK_HUE_ROTATION, m_oi.m_HueRotation);
}

void COptsPaletteDlg::UpdateUI()
{
	UpdateData();	// get controls
}

void COptsPaletteDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptsPaletteDlg)
	//}}AFX_DATA_MAP
	DDX_Check(pDX, IDC_OPTS_PAL_FIRST_BKGND, m_oi.m_PalFirstBkgnd);
	DDX_Check(pDX, IDC_OPTS_PAL_SMOOTH_WRAP, m_oi.m_PalSmoothWrap);
	DDX_Text(pDX, IDC_OPTS_PAL_EXPORT_SIZE, m_oi.m_PalExportSize);
	DDV_MinMaxInt(pDX, m_oi.m_PalExportSize, 1, INT_MAX);
	DDX_Text(pDX, IDC_OPTS_HUE_ROTATION, m_oi.m_HueRotation);
	DDV_MinMaxInt(pDX, m_oi.m_HueRotation, -360, 360);
}

BEGIN_MESSAGE_MAP(COptsPaletteDlg, CPropertyPage)
	//{{AFX_MSG_MAP(COptsPaletteDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptsPaletteDlg message handlers

BOOL COptsPaletteDlg::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	UpdateUI();

	return TRUE;
}
