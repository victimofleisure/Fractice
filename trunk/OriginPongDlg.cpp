// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      16feb09	initial version

        ping pong origin motion page
 
*/

// OriginPongDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Fractice.h"
#include "OriginPongDlg.h"
#include "MixerInfo.h"
#include "ColorMappingDlg.h"	// for slider Norm/Denorm
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COriginPongDlg dialog

const CEditSliderCtrl::INFO COriginPongDlg::m_SliderInfo[SLIDERS] = {
//	Range	Range	Log		Slider	Default	Tic		Edit	Edit
//	Min		Max		Base	Scale	Pos		Count	Scale	Precision
	{0,		10,		0,		1,		5,		0,		1,		0},	// Speed
};

COriginPongDlg::COriginPongDlg(CMixerInfo& Info, CWnd *pParent) 
	: CChildDlg(IDD, pParent), m_mi(Info)
{
	//{{AFX_DATA_INIT(COriginPongDlg)
	//}}AFX_DATA_INIT
}

void COriginPongDlg::DoDataExchange(CDataExchange* pDX)
{
	CChildDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COriginPongDlg)
	DDX_Control(pDX, IDC_ORG_PONG_SPEED_EDIT, m_SpeedEdit);
	DDX_Control(pDX, IDC_ORG_PONG_SPEED_SLIDER, m_SpeedSlider);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COriginPongDlg, CChildDlg)
	//{{AFX_MSG_MAP(COriginPongDlg)
	ON_NOTIFY(NEN_CHANGED, IDC_ORG_PONG_SPEED_EDIT, OnChangedSpeedEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COriginPongDlg message handlers

BOOL COriginPongDlg::OnInitDialog() 
{
	CChildDlg::OnInitDialog();
	
	m_SpeedSlider.SetInfo(m_SliderInfo[SL_SPEED], &m_SpeedEdit);
	m_SpeedSlider.SetVal(m_mi.GetCurChan().m_PongSpeed);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COriginPongDlg::OnChangedSpeedEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	UINT	PongSpeed = m_SpeedEdit.GetIntVal();
	m_mi.GetCurChan().m_PongSpeed = PongSpeed;
	theApp.GetMain()->GetMixer().SetPongSpeed(m_mi.m_CurChan, PongSpeed);
	*pResult = 0;
}
