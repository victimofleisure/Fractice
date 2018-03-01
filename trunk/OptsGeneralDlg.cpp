// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      08feb09	initial version

        general options page
 
*/

// OptsGeneralDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Fractice.h"
#include "OptionsDlg.h"
#include "OptsGeneralDlg.h"
#include "MultiFileDlg.h"
#include "shlwapi.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptsGeneralDlg property page

#define RK_UNDO_LEVELS		_T("UndoLevels")
#define RK_CACHE_IMAGES		_T("CacheImages")
#define RK_SAVE_ESC_TIMES	_T("SaveEscTimes")

COptsGeneralDlg::COptsGeneralDlg(COptionsInfo& Info)
	: CPropertyPage(IDD),
	m_oi(Info)
{
	//{{AFX_DATA_INIT(COptsGeneralDlg)
	//}}AFX_DATA_INIT
	theApp.RdReg2Int(RK_UNDO_LEVELS, m_oi.m_UndoLevels);
	m_oi.m_UndoUnlimited = m_oi.m_UndoLevels < 0;
	theApp.RdReg2Int(RK_CACHE_IMAGES, m_oi.m_CacheImages);
	theApp.RdReg2Int(RK_SAVE_ESC_TIMES, m_oi.m_SaveEscTimes);
}

COptsGeneralDlg::~COptsGeneralDlg()
{
	int	UndoLevels = m_oi.m_UndoUnlimited ? -1 : m_oi.m_UndoLevels;
	theApp.WrRegInt(RK_UNDO_LEVELS, UndoLevels);
	theApp.WrRegInt(RK_CACHE_IMAGES, m_oi.m_CacheImages);
	theApp.WrRegInt(RK_SAVE_ESC_TIMES, m_oi.m_SaveEscTimes);
}

void COptsGeneralDlg::UpdateUI()
{
	UpdateData();	// get controls
	GetDlgItem(IDC_OPTS_UNDO_LEVELS_EDIT)->EnableWindow(!m_oi.m_UndoUnlimited);
}

void COptsGeneralDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptsGeneralDlg)
	//}}AFX_DATA_MAP
	DDX_Text(pDX, IDC_OPTS_DEF_SNAP_EDIT, m_oi.m_DefSnapshot);
	DDX_Check(pDX, IDC_OPTS_UNDO_UNLIMITED_CHK, m_oi.m_UndoUnlimited);
	if (!m_oi.m_UndoUnlimited)	// if undo levels edit control isn't blanked
		DDX_Text(pDX, IDC_OPTS_UNDO_LEVELS_EDIT, m_oi.m_UndoLevels);
	DDX_Check(pDX, IDC_OPTS_CACHE_IMAGES_CHK, m_oi.m_CacheImages);
	DDX_Check(pDX, IDC_OPTS_SAVE_ESC_TIMES_CHK, m_oi.m_SaveEscTimes);
}

BEGIN_MESSAGE_MAP(COptsGeneralDlg, CPropertyPage)
	//{{AFX_MSG_MAP(COptsGeneralDlg)
	ON_BN_CLICKED(IDC_OPTS_DEF_SNAP_BROWSE, OnDefSnapBrowse)
	ON_BN_CLICKED(IDC_OPTS_UNDO_UNLIMITED_CHK, OnUndoUnlimitedChk)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptsGeneralDlg message handlers

BOOL COptsGeneralDlg::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	UpdateUI();

	return TRUE;
}

void COptsGeneralDlg::OnDefSnapBrowse() 
{
	CMultiFileDlg	fd(TRUE, SNAPSHOT_EXT, NULL, OFN_HIDEREADONLY,
		LDS(IDS_SNAPSHOT_FILTER), NULL, NULL, &theApp.m_DocFolder);
	if (fd.DoModal() == IDOK) {
		m_oi.m_DefSnapshot = fd.GetPathName();
		UpdateData(FALSE);	// init controls
	}
}

void COptsGeneralDlg::OnUndoUnlimitedChk() 
{
	if (m_oi.m_UndoUnlimited)	// avoid spurious validation message
		GetDlgItem(IDC_OPTS_UNDO_LEVELS_EDIT)->SetWindowText(_T("0"));
	UpdateData();	// get controls
	m_oi.m_UndoLevels = m_oi.m_UndoUnlimited ? -1 : 100;
	UpdateData(FALSE);	// init controls
	UpdateUI();
	if (m_oi.m_UndoUnlimited)	// if unlimited, blank levels edit control
		GetDlgItem(IDC_OPTS_UNDO_LEVELS_EDIT)->SetWindowText(_T(""));
}

BOOL COptsGeneralDlg::OnKillActive() 
{
	if (IsWindowVisible()) {	// ignore spurious initial call
		UpdateData();	// get controls
		if (!m_oi.m_DefSnapshot.IsEmpty()
		&& !PathFileExists(m_oi.m_DefSnapshot)) {
			AfxMessageBox(IDS_OPTS_SNAP_NOT_FOUND);
			GotoDlgCtrl(GetDlgItem(IDC_OPTS_DEF_SNAP_EDIT));
			return FALSE;
		}
	}
	return CPropertyPage::OnKillActive();
}
