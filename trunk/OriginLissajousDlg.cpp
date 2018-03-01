// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      16feb09	initial version
		01		01jun09	give each property its own edit handler

        Lissajous origin motion page
 
*/

// OriginLissajousDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Fractice.h"
#include "OriginLissajousDlg.h"
#include "MixerInfo.h"
#include "MainFrm.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COriginLissajousDlg dialog

const CEditSliderCtrl::INFO COriginLissajousDlg::m_SliderInfo[SLIDERS] = {
//	Range	Range	Log		Slider	Default	Tic		Edit	Edit
//	Min		Max		Base	Scale	Pos		Count	Scale	Precision
	{-400,	-100,	10,		100,	-250,	0,		.1f,	4},	// Speed
	{0,		20,		0,		1,		10,		0,		1,		3},	// XFreq
	{0,		20,		0,		1,		10,		0,		1,		3},	// YFreq
	{-100,	100,	0,		100,	0,		0,		1,		3},	// Phase
};

COriginLissajousDlg::COriginLissajousDlg(CMixerInfo& Info, CWnd *pParent) 
	: CChildDlg(IDD, pParent), m_mi(Info)
{
	//{{AFX_DATA_INIT(COriginLissajousDlg)
	//}}AFX_DATA_INIT
}

void COriginLissajousDlg::DoDataExchange(CDataExchange* pDX)
{
	CChildDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COriginLissajousDlg)
	DDX_Control(pDX, IDC_ORG_LISS_SPEED_EDIT, m_SpeedEdit);
	DDX_Control(pDX, IDC_ORG_LISS_SPEED_SLIDER, m_SpeedSlider);
	DDX_Control(pDX, IDC_ORG_LISS_XFREQ_EDIT, m_XFreqEdit);
	DDX_Control(pDX, IDC_ORG_LISS_XFREQ_SLIDER, m_XFreqSlider);
	DDX_Control(pDX, IDC_ORG_LISS_YFREQ_EDIT, m_YFreqEdit);
	DDX_Control(pDX, IDC_ORG_LISS_YFREQ_SLIDER, m_YFreqSlider);
	DDX_Control(pDX, IDC_ORG_LISS_PHASE_EDIT, m_PhaseEdit);
	DDX_Control(pDX, IDC_ORG_LISS_PHASE_SLIDER, m_PhaseSlider);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COriginLissajousDlg, CChildDlg)
	//{{AFX_MSG_MAP(COriginLissajousDlg)
	ON_NOTIFY(NEN_CHANGED, IDC_ORG_LISS_XFREQ_EDIT, OnChangedXFreqEdit)
	ON_NOTIFY(NEN_CHANGED, IDC_ORG_LISS_YFREQ_EDIT, OnChangedYFreqEdit)
	ON_NOTIFY(NEN_CHANGED, IDC_ORG_LISS_SPEED_EDIT, OnChangedSpeedEdit)
	ON_NOTIFY(NEN_CHANGED, IDC_ORG_LISS_PHASE_EDIT, OnChangedPhaseEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COriginLissajousDlg message handlers

BOOL COriginLissajousDlg::OnInitDialog() 
{
	CChildDlg::OnInitDialog();
	
	m_SpeedSlider.SetInfo(m_SliderInfo[SL_SPEED], &m_SpeedEdit);
	m_SpeedSlider.SetVal(m_mi.GetCurChan().m_LissSpeed);
	m_XFreqSlider.SetInfo(m_SliderInfo[SL_XFREQ], &m_XFreqEdit);
	m_XFreqSlider.SetVal(m_mi.GetCurChan().m_LissXFreq);
	m_YFreqSlider.SetInfo(m_SliderInfo[SL_YFREQ], &m_YFreqEdit);
	m_YFreqSlider.SetVal(m_mi.GetCurChan().m_LissYFreq);
	m_PhaseSlider.SetInfo(m_SliderInfo[SL_PHASE], &m_PhaseEdit);
	m_PhaseSlider.SetVal(m_mi.GetCurChan().m_LissPhase);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COriginLissajousDlg::OnChangedSpeedEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	double	Speed = m_SpeedEdit.GetVal();
	m_mi.GetCurChan().m_LissSpeed = Speed;
	theApp.GetMain()->GetMixer().SetLissajousSpeed(m_mi.m_CurChan, Speed);
	*pResult = 0;
}

void COriginLissajousDlg::OnChangedXFreqEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	double	XFreq = m_XFreqEdit.GetVal();
	m_mi.GetCurChan().m_LissXFreq = XFreq;
	theApp.GetMain()->GetMixer().SetLissajousXFreq(m_mi.m_CurChan, XFreq);
	*pResult = 0;
}

void COriginLissajousDlg::OnChangedYFreqEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	double	YFreq = m_YFreqEdit.GetVal();
	m_mi.GetCurChan().m_LissYFreq = YFreq;
	theApp.GetMain()->GetMixer().SetLissajousYFreq(m_mi.m_CurChan, YFreq);
	*pResult = 0;
}

void COriginLissajousDlg::OnChangedPhaseEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	double	Phase = m_PhaseEdit.GetVal();
	m_mi.GetCurChan().m_LissPhase = Phase;
	theApp.GetMain()->GetMixer().SetLissajousPhase(m_mi.m_CurChan, Phase);
	*pResult = 0;
}
