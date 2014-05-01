// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      28feb09	initial version
		01		31mar09	add open/save
		02		06jan10	W64: in OnQuerySiblings, cast control ID to 32-bit

        main mixer page
 
*/

// MixMiscDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Fractice.h"
#include "MixMiscDlg.h"
#include "MainFrm.h"
#include "MultiFileDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMixMiscDlg dialog

const CEditSliderCtrl::INFO CMixMiscDlg::m_SliderInfo[SLIDERS] = {
//	Range	Range	Log		Slider	Default	Tic		Edit	Edit
//	Min		Max		Base	Scale	Pos		Count	Scale	Precision
	{0,		300,	0,		10,		150,	0,		1,		1},	// PalTweenTime
};

CMixMiscDlg::CMixMiscDlg(CMixerInfo& Info)
	: CPropertyPage(IDD),
	m_mi(Info)
{
	//{{AFX_DATA_INIT(CMixMiscDlg)
	//}}AFX_DATA_INIT
}

CMixMiscDlg::~CMixMiscDlg()
{
}

void CMixMiscDlg::UpdateCtrls(int CtrlID)
{
	// if no control was specified, or control is ours
	if (!CtrlID || GetDescendantWindow(CtrlID) != NULL)	{
		UpdateData(FALSE);	// update controls
		if (!CtrlID || CtrlID == IDC_MIX_TWEEN_TIME_SLIDER)
			m_TweenTimeSlider.SetVal(m_mi.m_PalTweenTime);
	}
}

void CMixMiscDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMixMiscDlg)
	DDX_Control(pDX, IDC_MIX_TWEEN_TIME_SLIDER, m_TweenTimeSlider);
	DDX_Control(pDX, IDC_MIX_TWEEN_TIME_EDIT, m_TweenTimeEdit);
	//}}AFX_DATA_MAP
	if (!pDX->m_bSaveAndValidate)
		m_TweenTimeSlider.SetVal(m_mi.m_PalTweenTime);
}

BEGIN_MESSAGE_MAP(CMixMiscDlg, CPropertyPage)
	//{{AFX_MSG_MAP(CMixMiscDlg)
	ON_NOTIFY(NEN_CHANGED, IDC_MIX_TWEEN_TIME_EDIT, OnChangedTweenTimeEdit)
	ON_BN_CLICKED(IDC_MIX_OPEN, OnOpen)
	ON_BN_CLICKED(IDC_MIX_SAVE, OnSave)
	//}}AFX_MSG_MAP
	ON_MESSAGE(PSM_QUERYSIBLINGS, OnQuerySiblings)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMixMiscDlg message handlers

BOOL CMixMiscDlg::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	m_TweenTimeSlider.SetInfo(m_SliderInfo[SL_TWEEN_TIME], &m_TweenTimeEdit);
	m_TweenTimeSlider.SetVal(m_mi.m_PalTweenTime);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CMixMiscDlg::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message >= WM_KEYFIRST || pMsg->message <= WM_KEYLAST) {
		if (theApp.HandleDlgKeyMsg(pMsg))	// give main accelerators a try
			return(TRUE);	// message was translated, stop dispatching
	}
	return CPropertyPage::PreTranslateMessage(pMsg);
}

LRESULT CMixMiscDlg::OnQuerySiblings(WPARAM wParam, LPARAM lParam)
{
	switch (wParam) {
	case CMixerDlg::QSC_UPDATECTRLS:
		UpdateCtrls(INT64TO32(lParam));	// lParam is control ID
		break;
	}
	return(0);	// continue dispatching
}

void CMixMiscDlg::OnChangedTweenTimeEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	m_mi.m_PalTweenTime = m_TweenTimeEdit.GetVal();
	theApp.GetMain()->GetMixer().SetPalTweenTime(m_mi.m_PalTweenTime);
}

void CMixMiscDlg::OnOpen() 
{
	CMultiFileDlg	fd(TRUE, MIXER_EXT, NULL, OFN_HIDEREADONLY, 
		LDS(IDS_MIXER_FILTER));
	if (fd.DoModal() == IDOK) {
		CMixerInfo	mi;
		if (mi.Read(fd.GetPathName()))
			theApp.GetMain()->GetMixerDlg().SetInfoEx(mi);
	}
}

void CMixMiscDlg::OnSave() 
{
	CMultiFileDlg	fd(FALSE, MIXER_EXT, NULL, OFN_OVERWRITEPROMPT,
		LDS(IDS_MIXER_FILTER));
	if (fd.DoModal() == IDOK) {
		CMixerInfo	mi;
		theApp.GetMain()->GetMixerDlg().GetInfoEx(mi);
		mi.Write(fd.GetPathName());
	}
}
