// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      06feb09	initial version
		01		23mar09	add view argument to normalized accessor
		02		22dec09	allow negative color cycling rate
		03		28dec09	in SetCycleRateNorm, allow negative rate

        color cycling dialog
 
*/

// ColorCyclingDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Fractice.h"
#include "ColorCyclingDlg.h"
#include "MainFrm.h"
#include "FracticeDoc.h"
#include "FracticeView.h"
#include "ColorMappingDlg.h"	// for Norm and Denorm
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CColorCyclingDlg dialog

IMPLEMENT_DYNAMIC(CColorCyclingDlg, CChildDlg);

const double CColorCyclingDlg::RATE_BASE = 1000;
const double CColorCyclingDlg::RATE_MIN = .0001;
const double CColorCyclingDlg::RATE_MAX = 1;

const CCtrlResize::CTRL_LIST CColorCyclingDlg::m_CtrlList[] = {
	{IDC_CCYC_RATE_SLIDER,			BIND_LEFT | BIND_RIGHT},
	{0, 0}	// list terminator
};

CColorCyclingDlg::CColorCyclingDlg(CWnd* pParent /*=NULL*/)
	: CChildDlg(IDD, pParent)
{
	//{{AFX_DATA_INIT(CColorCyclingDlg)
	//}}AFX_DATA_INIT
	m_View = NULL;
	m_Cycling = 0;
	m_Rate = 0;
}

static inline double InvPow(double base, double x)
{
	return(log(x) / log(base));	// inverse of pow(base, x)
}

double CColorCyclingDlg::PosToRate(double Pos) const
{
	return(CColorMappingDlg::Denorm(Pos / RATE_RANGE,
		RATE_BASE, RATE_MIN, RATE_MAX));
}

double CColorCyclingDlg::RateToPos(double Rate) const
{
	return(CColorMappingDlg::Norm(Rate, RATE_BASE, 
		RATE_MIN, RATE_MAX) * RATE_RANGE);
}

void CColorCyclingDlg::SetCycleRateNorm(CFracticeView *View, double Val)
{
	if (View != NULL) {
		Val *= RATE_RANGE;
		double	rate = Val >= 0 ? PosToRate(Val) : -PosToRate(-Val);
		View->SetColorCycleRate(rate);
		if (View == m_View)	// if active view
			Update();	// update slider
	}
}

void CColorCyclingDlg::Update()
{
	bool	Cycling;
	double	Rate;
	if (m_View != NULL) {
		if (!m_View->GetImageSize().cx)	// if view isn't fully initialized
			return;	// defer update to avoid flicker
		Cycling = m_View->GetCycleColors();
		Rate = m_View->GetColorCycleRate();
	} else {
		Cycling = FALSE;
		Rate = 0;
	}
	if (Cycling != m_Cycling) {
		m_CycleChk.SetCheck(Cycling);
		m_Cycling = Cycling;
	}
	if (Rate != m_Rate) {
		m_RateSlider.SetPos(round(RateToPos(Rate)));	// set slider
		m_RateEdit.SetVal(Rate);	// set edit control
		m_Rate = Rate;
	}
}

void CColorCyclingDlg::EnableCtrls(bool Enable)
{
	theApp.EnableChildWindows(*this, Enable);
}

void CColorCyclingDlg::SetView(CFracticeView *View)
{
	if (View == m_View)
		return;	// nothing to do
	bool	PrevEnab = m_View != NULL;
	m_View = View;
	Update();
	bool	Enable = View != NULL;
	if (Enable != PrevEnab)	// if enable changed
		EnableCtrls(Enable);
}

void CColorCyclingDlg::DoDataExchange(CDataExchange* pDX)
{
	CChildDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CColorCyclingDlg)
	DDX_Control(pDX, IDC_CCYC_CYCLE_CHK, m_CycleChk);
	DDX_Control(pDX, IDC_CCYC_RATE_EDIT, m_RateEdit);
	DDX_Control(pDX, IDC_CCYC_RATE_SLIDER, m_RateSlider);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CColorCyclingDlg, CChildDlg)
	//{{AFX_MSG_MAP(CColorCyclingDlg)
	ON_WM_HSCROLL()
	ON_EN_KILLFOCUS(IDC_CCYC_RATE_EDIT, OnKillfocusRateEdit)
	ON_BN_CLICKED(IDC_CCYC_CYCLE_CHK, OnCycleChk)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(UWM_DLGBARUPDATE, OnDlgBarUpdate)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CColorCyclingDlg message handlers

BOOL CColorCyclingDlg::OnInitDialog() 
{
	CChildDlg::OnInitDialog();
	
	m_RateSlider.SetRange(0, RATE_RANGE);
	m_RateEdit.SetRange(INT_MIN, INT_MAX);
	m_RateEdit.SetPrecision(RATE_PREC);
	m_CycleChk.SetCheck(m_Cycling);
	m_RateEdit.SetVal(m_Rate);
	m_RateSlider.SetTicFreq(RATE_RANGE / RATE_TICS);
	m_Resize.AddControlList(this, m_CtrlList);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CColorCyclingDlg::OnSize(UINT nType, int cx, int cy) 
{
	CChildDlg::OnSize(nType, cx, cy);
	m_Resize.OnSize();
}

LRESULT	CColorCyclingDlg::OnDlgBarUpdate(WPARAM wParam, LPARAM lParam)
{
	Update();
	return(0);
}

void CColorCyclingDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CSliderCtrl	*pSlider = (CSliderCtrl *)pScrollBar;
	if (pSlider == &m_RateSlider) {
		int	pos = m_RateSlider.GetPos();	// get slider position
		m_Rate = PosToRate(pos);
		m_RateEdit.SetVal(m_Rate);
		UpdateData(FALSE);	// update edit control
		if (m_View != NULL)
			m_View->SetColorCycleRate(m_Rate);
	}		
	CChildDlg::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CColorCyclingDlg::OnKillfocusRateEdit() 
{
	double	m_Rate = m_RateEdit.GetVal();
	int	pos = round(RateToPos(m_Rate));
	m_RateSlider.SetPos(pos);
	if (m_View != NULL)
		m_View->SetColorCycleRate(m_Rate);
}

void CColorCyclingDlg::OnCycleChk() 
{
	m_Cycling = m_CycleChk.GetCheck() != 0;
	if (m_View != NULL)
		m_View->SetCycleColors(m_Cycling);
}
