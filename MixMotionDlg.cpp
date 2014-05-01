// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      28feb09	initial version
        01      09mar09	add channel array
		02		06jan10	W64: in OnQuerySiblings, cast control ID to 32-bit

        motion mixer page
 
*/

// MixMotionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Fractice.h"
#include "MixMotionDlg.h"
#include "OriginDragDlg.h"
#include "OriginLissajousDlg.h"
#include "OriginPongDlg.h"
#include "OriginRandomDlg.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMixMotionDlg dialog

// this table must match
// 1) the origin motions enum in MixerChan.h
// 2) the combo box data in the IDD_MIX_MOTION resource
// 3) the switch statement in CreateOrgMotionPage
const int CMixMotionDlg::m_OrgMotionPageID[CMixer::OM_TYPES] = {
	0,						// OM_NONE
	IDD_ORIGIN_PONG,		// OM_PING_PONG
	IDD_ORIGIN_DRAG,		// OM_CURSOR_DRAG
	IDD_ORIGIN_DRAG,		// OM_MIDI_DRAG
	IDD_ORIGIN_RANDOM,		// OM_RANDOM_JUMP
	IDD_ORIGIN_LISSAJOUS	// OM_LISSAJOUS
};

CMixMotionDlg::CMixMotionDlg(CMixerInfo& Info)
	: CPropertyPage(IDD),
	m_mi(Info)
{
	//{{AFX_DATA_INIT(CMixMotionDlg)
	//}}AFX_DATA_INIT
	m_OMPage = NULL;
	m_OMType = -1;
}

CMixMotionDlg::~CMixMotionDlg()
{
}

void CMixMotionDlg::UpdateCtrls(int CtrlID)
{
	// if no control was specified, or control is ours
	if (!CtrlID || GetDescendantWindow(CtrlID) != NULL)	{
		UpdateData(FALSE);	// update controls
		// no control was specifed, or motion type changed
		if (!CtrlID || m_mi.GetCurChan().m_OrgMotion != m_OMType)
			CreateOrgMotionPage(m_mi.GetCurChan().m_OrgMotion);	// rebuild motion page
	}
}

void CMixMotionDlg::CreateOrgMotionPage(int OMType)
{
	DestroyOrgMotionPage();
	CDialog	*page;
	switch (OMType) {
	case CMixer::OM_PING_PONG:
		page = new COriginPongDlg(m_mi);
		break;
	case CMixer::OM_CURSOR_DRAG:
	case CMixer::OM_MIDI_DRAG:
		page = new COriginDragDlg(m_mi);
		break;
	case CMixer::OM_RANDOM_JUMP:
		page = new COriginRandomDlg(m_mi);
		break;
	case CMixer::OM_LISSAJOUS:
		page = new COriginLissajousDlg(m_mi);
		break;
	default:
		page = NULL;
	}
	if (page != NULL) {
		page->Create(m_OrgMotionPageID[OMType], this);
		CRect	r;
		GetDlgItem(IDC_MIX_ORG_MOTION_PARAMS)->GetWindowRect(r);
		ScreenToClient(r);
		page->MoveWindow(r);
		page->ShowWindow(SW_SHOW);
		m_OMPage = page;
		m_OMType = OMType;
	}
}

void CMixMotionDlg::DestroyOrgMotionPage()
{
	if (m_OMPage != NULL) {
		m_OMPage->DestroyWindow();
		delete m_OMPage;
		m_OMPage = NULL;
		m_OMType = -1;
	}
}

void CMixMotionDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMixMotionDlg)
	//}}AFX_DATA_MAP
	DDX_Check(pDX, IDC_MIX_MIRROR_CHK, m_mi.m_Mirror);
	DDX_CBIndex(pDX, IDC_MIX_ORG_MOTION_COMBO, m_mi.GetCurChan().m_OrgMotion);
	DDX_Radio(pDX, IDC_MIX_CHAN_SELECT, m_mi.m_CurChan);
}

BEGIN_MESSAGE_MAP(CMixMotionDlg, CPropertyPage)
	//{{AFX_MSG_MAP(CMixMotionDlg)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_MIX_MIRROR_CHK, OnMixMirrorChk)
	ON_BN_CLICKED(IDC_MIX_CHAN_SELECT, OnChanSelect)
	ON_BN_CLICKED(IDC_MIX_CHAN_SELECT2, OnChanSelect)
	//}}AFX_MSG_MAP
	ON_CBN_SELCHANGE(IDC_MIX_ORG_MOTION_COMBO, OnSelchangeOrgMotionCombo)
	ON_MESSAGE(PSM_QUERYSIBLINGS, OnQuerySiblings)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMixMotionDlg message handlers

BOOL CMixMotionDlg::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	CreateOrgMotionPage(m_mi.GetCurChan().m_OrgMotion);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMixMotionDlg::OnDestroy() 
{
	CPropertyPage::OnDestroy();
	DestroyOrgMotionPage();
}

BOOL CMixMotionDlg::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message >= WM_KEYFIRST || pMsg->message <= WM_KEYLAST) {
		if (theApp.HandleDlgKeyMsg(pMsg))	// give main accelerators a try
			return(TRUE);	// message was translated, stop dispatching
	}
	return CPropertyPage::PreTranslateMessage(pMsg);
}

LRESULT CMixMotionDlg::OnQuerySiblings(WPARAM wParam, LPARAM lParam)
{
	switch (wParam) {
	case CMixerDlg::QSC_UPDATECTRLS:
		UpdateCtrls(INT64TO32(lParam));	// lParam is control ID
		break;
	}
	return(0);	// continue dispatching
}

void CMixMotionDlg::OnMixMirrorChk() 
{
	UpdateData();	// get controls
	theApp.GetMain()->GetMixer().SetMirror(m_mi.m_Mirror != 0);
}

void CMixMotionDlg::OnSelchangeOrgMotionCombo() 
{
	UpdateData();	// get controls
	int	om = m_mi.GetCurChan().m_OrgMotion;
	CreateOrgMotionPage(om);
	theApp.GetMain()->GetMixer().SetOriginMotion(m_mi.m_CurChan, om);
}

void CMixMotionDlg::OnChanSelect() 
{
	UpdateData();	// get controls
	UpdateData(FALSE);	// update controls from member data
	CreateOrgMotionPage(m_mi.GetCurChan().m_OrgMotion);	// rebuild motion page
}
