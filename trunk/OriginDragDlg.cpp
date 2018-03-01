// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      16feb09	initial version
		01		29mar09	use classic damping norm/denorm

        cursor drag origin motion page
 
*/

// OriginDragDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Fractice.h"
#include "OriginDragDlg.h"
#include "MixerInfo.h"
#include "MainFrm.h"
#include <math.h>	// for log and pow

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COriginDragDlg dialog

const CEditSliderCtrl::INFO COriginDragDlg::m_SliderInfo[SLIDERS] = {
//	Range	Range	Log		Slider	Default	Tic		Edit	Edit
//	Min		Max		Base	Scale	Pos		Count	Scale	Precision
	{0,		100,	20,		101,	50,		0,		100,	3},	// Damping
};

const double COriginDragDlg::CDampingEdit::SCALE	= m_SliderInfo[SL_DAMPING].EditScale;

COriginDragDlg::COriginDragDlg(CMixerInfo& Info, CWnd *pParent) 
	: CChildDlg(IDD, pParent), m_mi(Info)
{
	//{{AFX_DATA_INIT(COriginDragDlg)
	//}}AFX_DATA_INIT
}

inline double COriginDragDlg::Log(double Base, double x)
{
	return(log(x) / log(Base));	// log with arbitrary base
}

inline double COriginDragDlg::LogNorm(double x, double Base, double Scale)
{
	// x is exponential from 0..1, result is linear from 0..Scale
	return(Log(Base, x * (Base - 1) + 1) * Scale);
}

inline double COriginDragDlg::ExpNorm(double x, double Base, double Scale)
{
	// x is linear from 0..Scale, result is exponential from 0..1
	return((pow(Base, x / Scale) - 1) / (Base - 1));
}

double COriginDragDlg::NormDampingSliderPos(double Damping)
{
	const CEditSliderCtrl::INFO&	info = m_SliderInfo[SL_DAMPING];
	double b = info.LogBase;
	double s = info.SliderScale;
	return(s - LogNorm(Damping, b, s));
}

double COriginDragDlg::DenormDampingSliderPos(double Pos)
{
	const CEditSliderCtrl::INFO&	info = m_SliderInfo[SL_DAMPING];
	double b = info.LogBase;
	double s = info.SliderScale;
	return(ExpNorm(s - Pos, b, s));
}

double COriginDragDlg::CDampingSlider::Norm(double x) const
{
	return(NormDampingSliderPos(x));
}

double COriginDragDlg::CDampingSlider::Denorm(double x) const
{
	return(DenormDampingSliderPos(x));
}

void COriginDragDlg::CDampingEdit::ValToStr(CString& Str)
{
	Str.Format(_T("%.1f"), (1 - m_Val) * SCALE);
}

void COriginDragDlg::CDampingEdit::StrToVal(LPCTSTR Str)
{
	m_Val = 1 - _tstof(Str) / SCALE;
}

void COriginDragDlg::DoDataExchange(CDataExchange* pDX)
{
	CChildDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COriginDragDlg)
	DDX_Control(pDX, IDC_ORG_DAMPING_EDIT, m_DampingEdit);
	DDX_Control(pDX, IDC_ORG_DAMPING_SLIDER, m_DampingSlider);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COriginDragDlg, CChildDlg)
	//{{AFX_MSG_MAP(COriginDragDlg)
	ON_NOTIFY(NEN_CHANGED, IDC_ORG_DAMPING_EDIT, OnChangedDampingEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COriginDragDlg message handlers

BOOL COriginDragDlg::OnInitDialog() 
{
	CChildDlg::OnInitDialog();
	
	m_DampingSlider.SetInfo(m_SliderInfo[SL_DAMPING], &m_DampingEdit);
	m_DampingSlider.SetVal(m_mi.GetCurChan().m_Damping);
	m_DampingEdit.SetRange(0, 1);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COriginDragDlg::OnChangedDampingEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	double	Damping = m_DampingSlider.GetVal();
	m_mi.GetCurChan().m_Damping = Damping;
	theApp.GetMain()->GetMixer().SetDamping(m_mi.m_CurChan, Damping);
	*pResult = 0;
}
