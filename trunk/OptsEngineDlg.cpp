// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      08feb09	initial version

        engine options page
 
*/

// OptsEngineDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Fractice.h"
#include "OptionsDlg.h"
#include "OptsEngineDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptsEngineDlg property page

#define RK_THREAD_COUNT		_T("ThreadCount")
#define RK_USE_SSE2			_T("UseSSE2")

COptsEngineDlg::COptsEngineDlg(COptionsInfo& Info)
	: CPropertyPage(IDD),
	m_oi(Info)
{
	//{{AFX_DATA_INIT(COptsEngineDlg)
	//}}AFX_DATA_INIT
	m_oi.m_ThreadCount = theApp.RdRegInt(RK_THREAD_COUNT);
	m_oi.m_ThreadsAuto = !m_oi.m_ThreadCount;
	if (m_oi.m_ThreadsAuto)
		m_oi.m_ThreadCount = theApp.GetCPUCount();
	theApp.RdReg2Int(RK_USE_SSE2, m_oi.m_UseSSE2);
}

COptsEngineDlg::~COptsEngineDlg()
{
	UINT	ThreadCount = m_oi.m_ThreadsAuto ? 0 : m_oi.m_ThreadCount;
	theApp.WrRegInt(RK_THREAD_COUNT, ThreadCount);
	theApp.WrRegInt(RK_USE_SSE2, m_oi.m_UseSSE2);
}

void COptsEngineDlg::UpdateUI()
{
	UpdateData();	// get controls
	GetDlgItem(IDC_OPTS_THREAD_COUNT_EDIT)->EnableWindow(!m_oi.m_ThreadsAuto);
}

void COptsEngineDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptsEngineDlg)
	//}}AFX_DATA_MAP
	DDX_Check(pDX, IDC_OPTS_THREADS_AUTO_CHK, m_oi.m_ThreadsAuto);
	DDX_Text(pDX, IDC_OPTS_THREAD_COUNT_EDIT, m_oi.m_ThreadCount);
	DDV_MinMaxUInt(pDX, m_oi.m_ThreadCount, 
		COptionsInfo::MIN_THREAD_COUNT, COptionsInfo::MAX_THREAD_COUNT);
	DDX_Check(pDX, IDC_OPTS_USE_SSE2_CHK, m_oi.m_UseSSE2);
}

BEGIN_MESSAGE_MAP(COptsEngineDlg, CPropertyPage)
	//{{AFX_MSG_MAP(COptsEngineDlg)
	ON_BN_CLICKED(IDC_OPTS_THREADS_AUTO_CHK, OnThreadsAutoChk)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptsEngineDlg message handlers

BOOL COptsEngineDlg::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	UpdateUI();
	GetDlgItem(IDC_OPTS_USE_SSE2_CHK)->EnableWindow(theApp.HaveSSE2());

	return TRUE;
}

void COptsEngineDlg::OnThreadsAutoChk() 
{
	UpdateData();	// get controls
	if (m_oi.m_ThreadsAuto) {
		m_oi.m_ThreadCount = theApp.GetCPUCount();
		UpdateData(FALSE);	// init controls
	}
	UpdateUI();
}
