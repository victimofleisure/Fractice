// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14mar09	initial version

        slide show dialog
 
*/

// SlideShowDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Fractice.h"
#include "SlideShowDlg.h"
#include "FracticeDoc.h"
#include "FracticeView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSlideShowDlg dialog

IMPLEMENT_DYNAMIC(CSlideShowDlg, CChildDlg);

const CEditSliderCtrl::INFO CSlideShowDlg::m_SliderInfo[SLIDERS] = {
//	Range	Range	Log		Slider	Default	Tic		Edit	Edit
//	Min		Max		Base	Scale	Pos		Count	Scale	Precision
	{1,		60,		0,		1,		30,		0,		1,		1},	// Interval
};

CSlideShowDlg::CSlideShowDlg(CWnd* pParent)
	: CChildDlg(IDD, pParent)
{
	//{{AFX_DATA_INIT(CSlideShowDlg)
	//}}AFX_DATA_INIT
	m_View = NULL;
	m_SlideMgr = NULL;
}

void CSlideShowDlg::SetTransport(int State, bool UpdateMgr)
{
	m_PlayBtn.SetCheck(State == CSlideShowMgr::TS_PLAY);
	m_PauseBtn.SetCheck(State == CSlideShowMgr::TS_PAUSE);
	m_StopBtn.SetCheck(State == CSlideShowMgr::TS_STOP);
	if (UpdateMgr && m_SlideMgr != NULL)
		m_SlideMgr->SetTransport(State);
}

void CSlideShowDlg::UpdateStatus()
{
	CString	s;
	if (m_SlideMgr != NULL && m_SlideMgr->IsRunning()) {
		s.Format(IDS_SLIDE_STATUS, m_SlideMgr->GetCurPos() + 1, 
			m_SlideMgr->GetItemCount());
	}
	m_Status.SetWindowText(s);
}

void CSlideShowDlg::Update()
{
	if (m_View != NULL) {
		m_IntervalSlider.SetVal(m_SlideMgr->GetInterval());
		SetTransport(m_SlideMgr->GetTransport(), FALSE);
		m_LoopBtn.SetCheck(m_SlideMgr->GetLoop());
		m_Order = m_SlideMgr->GetOrder();
		UpdateStatus();
	} else {
		m_IntervalSlider.SetVal(0);
		SetTransport(CSlideShowMgr::TS_STOP);
		m_LoopBtn.SetCheck(FALSE);
		m_Order = 0;
		m_Status.SetWindowText(_T(""));
	}
	UpdateData(FALSE);
}

void CSlideShowDlg::SetView(CFracticeView *View)
{
	if (View == m_View)
		return;	// nothing to do
	bool	PrevEnab = m_View != NULL;
	m_View = View;
	m_SlideMgr = View != NULL ? &View->GetHistoryView()->GetSlideShowMgr() : NULL;
	Update();
	bool	Enable = View != NULL;
	if (Enable != PrevEnab)	// if enable changed
		EnableCtrls(Enable);
}

void CSlideShowDlg::EnableCtrls(bool Enable)
{
	theApp.EnableChildWindows(*this, Enable);
}

void CSlideShowDlg::DoDataExchange(CDataExchange* pDX)
{
	CChildDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSlideShowDlg)
	DDX_Control(pDX, IDC_SLIDE_PLAY, m_PlayBtn);
	DDX_Control(pDX, IDC_SLIDE_PAUSE, m_PauseBtn);
	DDX_Control(pDX, IDC_SLIDE_STOP, m_StopBtn);
	DDX_Control(pDX, IDC_SLIDE_LOOP, m_LoopBtn);
	DDX_Control(pDX, IDC_SLIDE_STATUS, m_Status);
	DDX_Control(pDX, IDC_SLIDE_INTERVAL_SLIDER, m_IntervalSlider);
	DDX_Control(pDX, IDC_SLIDE_INTERVAL_EDIT, m_IntervalEdit);
	DDX_Radio(pDX, IDC_SLIDE_ORDER, m_Order);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSlideShowDlg, CChildDlg)
	//{{AFX_MSG_MAP(CSlideShowDlg)
	ON_NOTIFY(NEN_CHANGED, IDC_SLIDE_INTERVAL_EDIT, OnChangedEdit)
	ON_BN_CLICKED(IDC_SLIDE_PLAY, OnPlay)
	ON_BN_CLICKED(IDC_SLIDE_PAUSE, OnPause)
	ON_BN_CLICKED(IDC_SLIDE_STOP, OnStop)
	ON_BN_CLICKED(IDC_SLIDE_LOOP, OnLoop)
	ON_BN_CLICKED(IDC_SLIDE_ORDER, OnOrder)
	ON_BN_CLICKED(IDC_SLIDE_ORDER2, OnOrder)
	//}}AFX_MSG_MAP
	ON_MESSAGE(UWM_DLGBARUPDATE, OnDlgBarUpdate)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSlideShowDlg message handlers

BOOL CSlideShowDlg::OnInitDialog() 
{
	CChildDlg::OnInitDialog();

	m_PlayBtn.SetIcons(IDI_TC_PLAY_UP, IDI_TC_PLAY_DOWN);
	m_PauseBtn.SetIcons(IDI_TC_PAUSE_UP, IDI_TC_PAUSE_DOWN);
	m_StopBtn.SetIcons(IDI_TC_STOP_UP, IDI_TC_STOP_DOWN);
	m_LoopBtn.SetIcons(IDI_TC_LOOP_UP, IDI_TC_LOOP_DOWN);
	m_IntervalSlider.SetInfo(m_SliderInfo[SL_INTERVAL], &m_IntervalEdit);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

LRESULT	CSlideShowDlg::OnDlgBarUpdate(WPARAM wParam, LPARAM lParam)
{
	Update();
	return(0);
}

void CSlideShowDlg::OnChangedEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	if (m_SlideMgr != NULL)
		m_SlideMgr->SetInterval(m_IntervalSlider.GetVal());
	*pResult = 0;
}

void CSlideShowDlg::OnPlay() 
{
	SetTransport(CSlideShowMgr::TS_PLAY);
}

void CSlideShowDlg::OnPause() 
{
	SetTransport(CSlideShowMgr::TS_PAUSE);
}

void CSlideShowDlg::OnStop() 
{
	SetTransport(CSlideShowMgr::TS_STOP);
}

void CSlideShowDlg::OnLoop() 
{
	bool	loop = !m_LoopBtn.GetCheck();
	m_LoopBtn.SetCheck(loop);
	if (m_SlideMgr != NULL)
		m_SlideMgr->SetLoop(loop);
}

void CSlideShowDlg::OnOrder() 
{
	UpdateData();
	if (m_SlideMgr != NULL)
		m_SlideMgr->SetOrder(m_Order);
}
