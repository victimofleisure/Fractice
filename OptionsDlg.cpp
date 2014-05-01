// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
        01      08feb09	convert from dialog to property sheet
		02		12feb09	add network adapter selection
		03		16feb09	add display options page
		04		01mar09	add scroll delta
		05		03mar09	add MIDI page
		06		06jan10	W64: DoModal return value is 64-bit

        options property sheet
 
*/

// OptionsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Fractice.h"
#include "OptionsDlg.h"
#include "FracticeDoc.h"
#include "Packets.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsDlg

IMPLEMENT_DYNAMIC(COptionsDlg, CPropertySheet)

#define RK_DEF_SNAPSHOT		_T("DefSnapshot")

const BASE_OPTIONS_INFO	COptionsDlg::m_DefaultState = {
	{DEF_THUMB_WIDTH, DEF_THUMB_HEIGHT},	// m_ThumbSize
	2,		// m_ZoomStep
	1.05,	// m_ContZoomStep
	20,		// m_ScrollDelta
	TRUE,	// m_ThreadsAuto
	0,		// m_ThreadCount
	CFracticeDoc::DOCF_PROJECT,	// m_DefDocFmt
	TRUE,	// m_UndoUnlimited
	-1,		// m_UndoLevels
	TRUE,	// m_SaveChgsWarn
	TRUE,	// m_UseSSE2
	FRACTICE_NET_PORT,	// m_NetworkPort
	TRUE,	// m_NetDefPort
	RGB(255, 255, 255),	// m_GridColor
	FALSE,	// m_ListenServers
	TRUE,	// m_CacheImages
	FALSE,	// m_SaveEscTimes
	FALSE,	// m_PalFirstBkgnd
	TRUE,	// m_PalSmoothWrap
	256,	// m_PalExportSize
	60,		// m_HueRotation
	4,		// m_RecentParams
	4,		// m_RecentPalettes
	25,		// m_FrameRate
};

COptionsDlg::COptionsDlg(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage),
	COptionsInfo(m_DefaultState),
#pragma warning(push)
#pragma warning(disable : 4355)	// 'this' used in base member initializer list
	m_GeneralDlg(*this),	// passing reference to our COptionsInfo base class
	m_DocumentDlg(*this),
	m_NavigateDlg(*this),
	m_EngineDlg(*this),
	m_PaletteDlg(*this),
	m_NetworkDlg(*this),
	m_DisplayDlg(*this),
	m_MidiDlg(*this)
#pragma warning(pop)
{
	ReadState();
	m_psh.dwFlags |= PSH_NOAPPLYNOW;
	AddPage(&m_GeneralDlg);
	AddPage(&m_DocumentDlg);
	AddPage(&m_NavigateDlg);
	AddPage(&m_EngineDlg);
	AddPage(&m_PaletteDlg);
	AddPage(&m_NetworkDlg);
	AddPage(&m_DisplayDlg);
	AddPage(&m_MidiDlg);
	m_CurPage = 0;
}

COptionsDlg::~COptionsDlg()
{
	WriteState();
}

void COptionsDlg::InitState()
{
	m_ThreadCount = theApp.GetCPUCount();
	m_UseSSE2 = theApp.HaveSSE2();
	m_DefSnapshot.Empty();
}

void COptionsDlg::ReadState()
{
	m_DefSnapshot = theApp.RdRegString(RK_DEF_SNAPSHOT);
}

void COptionsDlg::WriteState()
{
	theApp.WrRegString(RK_DEF_SNAPSHOT, m_DefSnapshot);
}

void COptionsDlg::SetDefaults()
{
	SetBaseInfo(m_DefaultState);
	InitState();
	theApp.GetMain()->GetMixerDlg().SetDefaults();	// reset mixer state too
}

void COptionsDlg::CreateResetAllButton()
{
	CRect	r, rt;
	GetDlgItem(IDOK)->GetWindowRect(r);
	GetTabControl()->GetWindowRect(rt);
	ScreenToClient(r);
	ScreenToClient(rt);
	int	w = r.Width();
	r.left = rt.left;
	r.right = rt.left + w;
	CString	Title(LPCTSTR(IDS_OPTS_RESET_ALL));
	m_ResetAll.Create(Title, BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE | WS_TABSTOP,
		r, this, IDS_OPTS_RESET_ALL);
	m_ResetAll.SetFont(GetFont());
	// adjust tab order so new button comes last
	CWnd	*pCancelBtn = GetDlgItem(IDCANCEL);	// assume cancel is now last
	if (pCancelBtn != NULL)
		m_ResetAll.SetWindowPos(pCancelBtn, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

UINT COptionsDlg::GetPalImpExpFlags() const
{
	UINT	mask = 0;
	if (m_PalFirstBkgnd)
		mask |= DPalette::IEF_FIRST_BKGND;
	if (m_PalSmoothWrap)
		mask |= DPalette::IEF_SMOOTH_WRAP;
	return(mask);
}

BEGIN_MESSAGE_MAP(COptionsDlg, CPropertySheet)
	//{{AFX_MSG_MAP(COptionsDlg)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDS_OPTS_RESET_ALL, OnResetAll)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsDlg message handlers

BOOL COptionsDlg::OnInitDialog() 
{
	BOOL bResult = CPropertySheet::OnInitDialog();
	CreateResetAllButton();	// create reset all button
	SetActivePage(m_CurPage);	// set current page
	GetInfo(m_Save);	// save current state; restore on cancel
	return bResult;
}

W64INT COptionsDlg::DoModal() 
{
	W64INT	retc = CPropertySheet::DoModal();
	if (retc == IDCANCEL) {
		SetInfo(m_Save);	// restore previous state
	}
	return(retc);
}

void COptionsDlg::OnDestroy() 
{
	m_CurPage = GetActiveIndex();
	CPropertySheet::OnDestroy();
}

void COptionsDlg::OnResetAll() 
{
	if (AfxMessageBox(IDS_OPTS_RESTORE_DEFAULTS, MB_YESNO | MB_DEFBUTTON2) == IDYES) {
		EndDialog(IDOK);
		SetDefaults();
		m_NetworkDlg.OnResetAll();
		m_DisplayDlg.OnResetAll();
	}
}
