// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      08feb09	initial version
		01		01mar09	add scroll delta

        navigate options page
 
*/

// OptsNavigateDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Fractice.h"
#include "OptionsDlg.h"
#include "OptsNavigateDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptsNavigateDlg property page

#define RK_ZOOM_STEP		_T("ZoomStep")
#define RK_CONT_ZOOM_STEP	_T("ContZoomStep")
#define RK_SCROLL_DELTA		_T("ScrollDelta")
#define RK_GRID_COLOR		_T("GridColor")
#define RK_MAX_MRU_PARAMS	_T("MaxMRUParams")
#define RK_MAX_MRU_PALETTES	_T("MaxMRUPalettes")

COptsNavigateDlg::COptsNavigateDlg(COptionsInfo& Info)
	: CPropertyPage(IDD),
	m_oi(Info)
{
	//{{AFX_DATA_INIT(COptsNavigateDlg)
	//}}AFX_DATA_INIT
	theApp.RdReg2Double(RK_ZOOM_STEP, m_oi.m_ZoomStep);
	theApp.RdReg2Double(RK_CONT_ZOOM_STEP, m_oi.m_ContZoomStep);
	theApp.RdReg2Int(RK_SCROLL_DELTA, m_oi.m_ScrollDelta);
	theApp.RdReg2ULong(RK_GRID_COLOR, m_oi.m_GridColor);
	theApp.RdReg2UInt(RK_MAX_MRU_PARAMS, m_oi.m_MaxMRUParams);
	theApp.RdReg2UInt(RK_MAX_MRU_PALETTES, m_oi.m_MaxMRUPalettes);
}

COptsNavigateDlg::~COptsNavigateDlg()
{
	theApp.WrRegDouble(RK_ZOOM_STEP, m_oi.m_ZoomStep);
	theApp.WrRegDouble(RK_CONT_ZOOM_STEP, m_oi.m_ContZoomStep);
	theApp.WrRegInt(RK_SCROLL_DELTA, m_oi.m_ScrollDelta);
	theApp.WrRegInt(RK_GRID_COLOR, m_oi.m_GridColor);
	theApp.WrRegInt(RK_MAX_MRU_PARAMS, m_oi.m_MaxMRUParams);
	theApp.WrRegInt(RK_MAX_MRU_PALETTES, m_oi.m_MaxMRUPalettes);
}

void COptsNavigateDlg::UpdateUI()
{
	UpdateData();	// get controls
}

void COptsNavigateDlg::DoDataExchange(CDataExchange* pDX)
{
	static const double MIN_ZOOM_STEP = 1e-10 + 1;
	static const double MAX_ZOOM_STEP = 1e10;
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptsNavigateDlg)
	//}}AFX_DATA_MAP
	DDX_Text(pDX, IDC_OPTS_ZOOM_STEP_EDIT, m_oi.m_ZoomStep);
	DDV_MinMaxDouble(pDX, m_oi.m_ZoomStep, MIN_ZOOM_STEP, MAX_ZOOM_STEP);
	DDX_Text(pDX, IDC_OPTS_CONT_ZOOM_STEP_EDIT, m_oi.m_ContZoomStep);
	DDV_MinMaxDouble(pDX, m_oi.m_ContZoomStep, MIN_ZOOM_STEP, MAX_ZOOM_STEP);
	DDX_Text(pDX, IDC_OPTS_SCROLL_DELTA, m_oi.m_ScrollDelta);
	DDV_MinMaxInt(pDX, m_oi.m_ScrollDelta, MIN_SCROLL_DELTA, MAX_SCROLL_DELTA);
	DDX_Text(pDX, IDC_OPTS_RECENT_PARAMS, m_oi.m_MaxMRUParams);
	DDV_MinMaxInt(pDX, m_oi.m_MaxMRUParams, MIN_MRU_ITEMS, MAX_MRU_ITEMS);
	DDX_Text(pDX, IDC_OPTS_RECENT_PALETTES, m_oi.m_MaxMRUPalettes);
	DDV_MinMaxInt(pDX, m_oi.m_MaxMRUPalettes, MIN_MRU_ITEMS, MAX_MRU_ITEMS);
}

BEGIN_MESSAGE_MAP(COptsNavigateDlg, CPropertyPage)
	//{{AFX_MSG_MAP(COptsNavigateDlg)
	ON_BN_CLICKED(IDC_OPTS_GRID_COLOR, OnGridColor)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptsNavigateDlg message handlers

BOOL COptsNavigateDlg::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	UpdateUI();

	return TRUE;
}

void COptsNavigateDlg::OnGridColor() 
{
	CColorDialog	dlg;
	dlg.m_cc.Flags |= CC_PREVENTFULLOPEN | CC_RGBINIT;
	dlg.m_cc.rgbResult = m_oi.m_GridColor;
	if (dlg.DoModal() == IDOK)
		m_oi.m_GridColor = dlg.m_cc.rgbResult;
}
