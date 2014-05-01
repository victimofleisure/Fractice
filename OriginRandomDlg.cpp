// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      16feb09	initial version

        random jump origin motion page
 
*/

// OriginRandomDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Fractice.h"
#include "OriginRandomDlg.h"
#include "MixerInfo.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COriginRandomDlg dialog

const CEditSliderCtrl::INFO COriginRandomDlg::m_SliderInfo[SLIDERS] = {
//	Range	Range	Log		Slider	Default	Tic		Edit	Edit
//	Min		Max		Base	Scale	Pos		Count	Scale	Precision
	{0,		250,	0,		1,		125,	0,		1,		3},	// Tempo
};

COriginRandomDlg::COriginRandomDlg(CMixerInfo& Info, CWnd *pParent) 
	: COriginDragDlg(Info, pParent)
{
	//{{AFX_DATA_INIT(COriginRandomDlg)
	//}}AFX_DATA_INIT
}

void COriginRandomDlg::DoDataExchange(CDataExchange* pDX)
{
	COriginDragDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COriginRandomDlg)
	DDX_Control(pDX, IDC_ORG_TEMPO_EDIT, m_TempoEdit);
	DDX_Control(pDX, IDC_ORG_TEMPO_SLIDER, m_TempoSlider);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COriginRandomDlg, COriginDragDlg)
	//{{AFX_MSG_MAP(COriginRandomDlg)
	ON_NOTIFY(NEN_CHANGED, IDC_ORG_TEMPO_EDIT, OnChangedTempoEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COriginRandomDlg message handlers

BOOL COriginRandomDlg::OnInitDialog() 
{
	COriginDragDlg::OnInitDialog();
	
	m_TempoSlider.SetInfo(m_SliderInfo[SL_TEMPO], &m_TempoEdit);
	m_TempoSlider.SetVal(m_mi.GetCurChan().m_Tempo);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COriginRandomDlg::OnChangedTempoEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	double	Tempo = m_TempoSlider.GetVal();
	m_mi.GetCurChan().m_Tempo = Tempo;
	theApp.GetMain()->GetMixer().SetTempo(m_mi.m_CurChan, Tempo);
	*pResult = 0;
}
