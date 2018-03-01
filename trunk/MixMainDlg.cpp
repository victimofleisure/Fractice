// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      28feb09	initial version
		01		10mar09	add autofade
		02		14mar09	use icon buttons
		03		01jun09	in SetFaderPos and OnHScroll, update mixer info
		04		06jan10	W64: in OnQuerySiblings, cast control ID to 32-bit

        main mixer page
 
*/

// MixMainDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Fractice.h"
#include "MixMainDlg.h"
#include "MainFrm.h"
#include "FracticeDoc.h"
#include "FracticeView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMixMainDlg dialog

const CEditSliderCtrl::INFO CMixMainDlg::m_SliderInfo[SLIDERS] = {
//	Range	Range	Log		Slider	Default	Tic		Edit	Edit
//	Min		Max		Base	Scale	Pos		Count	Scale	Precision
	{0,		300,	0,		10,		150,	0,		1,		1},	// FadeTime
};

CMixMainDlg::CMixMainDlg(CMixerInfo& Info)
	: CPropertyPage(IDD),
	m_mi(Info)
{
	//{{AFX_DATA_INIT(CMixMainDlg)
	//}}AFX_DATA_INIT
	m_Main = NULL;
	m_Mixer = NULL;
}

CMixMainDlg::~CMixMainDlg()
{
}

void CMixMainDlg::UpdateCtrls(int CtrlID)
{
	// if no control was specified, or control is ours
	if (!CtrlID || GetDescendantWindow(CtrlID) != NULL)	{
		UpdateData(FALSE);	// update controls
		if (!CtrlID || CtrlID == IDC_MIX_FADER)
			m_Fader.SetPos(round(m_mi.m_FaderPos * FADER_RANGE));
		if (!CtrlID || CtrlID == IDC_MIX_AUTOFADE_TIME_SLIDER)
			m_FadeTimeSlider.SetVal(m_mi.m_AutofadeTime);
		if (!CtrlID || CtrlID == IDC_MIX_AUTOFADE_PLAY)
			SetPlay(m_mi.m_AutofadePlay != 0);
		if (!CtrlID || CtrlID == IDC_MIX_AUTOFADE_LOOP)
			SetLoop(m_mi.m_AutofadeLoop != 0);
		for (int i = 0; i < MIXER_CHANS; i++) {
			if (!CtrlID || CtrlID == IDC_MIX_SRC_A_COMBO + i) {
				int	sel = FindSource(m_Mixer->GetView(i));
				m_SrcCombo[i].SetCurSel(sel);
			}
		}
	}
}

int CMixMainDlg::FindSource(CFracticeView *View) const
{
	int	sources = GetSourceCount();
	for (int i = 0; i < sources; i++) {
		if (m_Source[i] == View)
			return(i);
	}
	return(-1);
}

void CMixMainDlg::UpdateSourceCombos()
{
	for (int i = 0; i < MIXER_CHANS; i++) {
		CComboBox&	combo = m_SrcCombo[i];
		combo.ResetContent();
		int	sources = GetSourceCount();
		combo.EnableWindow(sources);
		for (int j = 0; j < sources; j++) {
			CString	s = GetSource(j)->GetDocument()->GetTitle();
			combo.AddString(s);
		}
		int	sel = FindSource(m_Mixer->GetView(i));
		combo.SetCurSel(sel);
	}
}

void CMixMainDlg::AddSource(CFracticeView *View)
{
	ASSERT(View != NULL);
	m_Source.Add(View);
	for (int i = 0; i < MIXER_CHANS; i++) {
		if (m_Mixer->GetView(i) == NULL)
			m_Mixer->SetView(i, View);
	}
	UpdateSourceCombos();
}

bool CMixMainDlg::RemoveSource(CFracticeView *View)
{
	if (View == NULL)
		return(FALSE);
	int	pos = FindSource(View);
	if (pos < 0)
		return(FALSE);
	m_Source.RemoveAt(pos);
	if (pos >= GetSourceCount())
		pos--;
	for (int i = 0; i < MIXER_CHANS; i++) {
		if (m_Mixer->GetView(i) == View) {
			m_Mixer->SetView(i, pos >= 0 ? GetSource(pos) : NULL);
		}
	}
	UpdateSourceCombos();
	return(TRUE);
}

bool CMixMainDlg::SelectSource(int ChanIdx, CFracticeView *View)
{
	int	pos = FindSource(View);
	if (pos < 0)
		return(FALSE);
	m_SrcCombo[ChanIdx].SetCurSel(pos);
	m_Mixer->SetView(ChanIdx, GetSource(pos));
	return(TRUE);
}

void CMixMainDlg::SetFaderPos(double Pos)
{
	m_Fader.SetPos(round(Pos * FADER_RANGE));
	m_mi.m_FaderPos = Pos;	// 01jun09 update mixer info
	m_Mixer->SetFaderPos(Pos);
}

void CMixMainDlg::SetPlay(bool Enable)
{
	m_PlayBtn.SetCheck(Enable != 0);
	m_StopBtn.SetCheck(Enable == 0);
	m_mi.m_AutofadePlay = Enable;
	m_Mixer->SetPlay(Enable);
}

void CMixMainDlg::SetLoop(bool Enable)
{
	m_LoopBtn.SetCheck(Enable != 0);
	m_mi.m_AutofadeLoop = Enable;
	m_Mixer->SetLoop(Enable);
}

void CMixMainDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMixMainDlg)
	DDX_Control(pDX, IDC_MIX_AUTOFADE_TIME_SLIDER, m_FadeTimeSlider);
	DDX_Control(pDX, IDC_MIX_AUTOFADE_TIME_EDIT, m_FadeTimeEdit);
	DDX_Control(pDX, IDC_MIX_AUTOFADE_STOP, m_StopBtn);
	DDX_Control(pDX, IDC_MIX_AUTOFADE_PLAY, m_PlayBtn);
	DDX_Control(pDX, IDC_MIX_AUTOFADE_LOOP, m_LoopBtn);
	DDX_Control(pDX, IDC_MIX_FADER, m_Fader);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_MIX_SRC_A_COMBO, m_SrcCombo[MIXER_CHAN_A]);
	DDX_Control(pDX, IDC_MIX_SRC_B_COMBO, m_SrcCombo[MIXER_CHAN_B]);
}

BEGIN_MESSAGE_MAP(CMixMainDlg, CPropertyPage)
	//{{AFX_MSG_MAP(CMixMainDlg)
	ON_CBN_SELCHANGE(IDC_MIX_SRC_A_COMBO, OnSelchangeSrcACombo)
	ON_CBN_SELCHANGE(IDC_MIX_SRC_B_COMBO, OnSelchangeSrcBCombo)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_MIX_AUTOFADE_LOOP, OnLoop)
	ON_BN_CLICKED(IDC_MIX_AUTOFADE_PLAY, OnPlay)
	ON_BN_CLICKED(IDC_MIX_AUTOFADE_STOP, OnStop)
	ON_NOTIFY(NEN_CHANGED, IDC_MIX_AUTOFADE_TIME_EDIT, OnChangedFadeTimeEdit)
	//}}AFX_MSG_MAP
	ON_MESSAGE(PSM_QUERYSIBLINGS, OnQuerySiblings)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMixMainDlg message handlers

BOOL CMixMainDlg::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	m_PlayBtn.SetIcons(IDI_TC_PLAY_UP, IDI_TC_PLAY_DOWN);
	m_StopBtn.SetIcons(IDI_TC_STOP_UP, IDI_TC_STOP_DOWN);
	m_LoopBtn.SetIcons(IDI_TC_LOOP_UP, IDI_TC_LOOP_DOWN);
	m_Main = theApp.GetMain();
	m_Mixer = &m_Main->GetMixer();
	m_Fader.SetRange(0, FADER_RANGE);
	m_Fader.SetTicFreq(FADER_RANGE / 10);
	m_Fader.SetDefaultPos(FADER_RANGE / 2);
	SetPlay(m_mi.m_AutofadePlay != 0);
	SetLoop(m_mi.m_AutofadeLoop != 0);
	m_FadeTimeSlider.SetInfo(m_SliderInfo[SL_FADE_TIME], &m_FadeTimeEdit);
	m_FadeTimeSlider.SetVal(m_mi.m_AutofadeTime);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CMixMainDlg::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message >= WM_KEYFIRST || pMsg->message <= WM_KEYLAST) {
		if (theApp.HandleDlgKeyMsg(pMsg))	// give main accelerators a try
			return(TRUE);	// message was translated, stop dispatching
	}
	return CPropertyPage::PreTranslateMessage(pMsg);
}

LRESULT CMixMainDlg::OnQuerySiblings(WPARAM wParam, LPARAM lParam)
{
	switch (wParam) {
	case CMixerDlg::QSC_UPDATECTRLS:
		UpdateCtrls(INT64TO32(lParam));	// lParam is control ID
		break;
	}
	return(0);	// continue dispatching
}

void CMixMainDlg::OnSelchangeSrcACombo() 
{
	int	sel = m_SrcCombo[MIXER_CHAN_A].GetCurSel();
	if (sel >= 0)
		m_Mixer->SetView(MIXER_CHAN_A, GetSource(sel));
}

void CMixMainDlg::OnSelchangeSrcBCombo() 
{
	int	sel = m_SrcCombo[MIXER_CHAN_B].GetCurSel();
	if (sel >= 0)
		m_Mixer->SetView(MIXER_CHAN_B, GetSource(sel));
}

void CMixMainDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if ((CSliderCtrl *)pScrollBar == &m_Fader) {
		double	pos = double(m_Fader.GetPos()) / FADER_RANGE;
		m_mi.m_FaderPos = pos;	// 01jun09 update mixer info
		m_Mixer->SetFaderPos(pos);
	}
	CPropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CMixMainDlg::OnLoop() 
{
	SetLoop(!m_mi.m_AutofadeLoop);
}

void CMixMainDlg::OnPlay() 
{
	SetPlay(TRUE);
}

void CMixMainDlg::OnStop() 
{
	SetPlay(FALSE);
}

void CMixMainDlg::OnChangedFadeTimeEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	m_mi.m_AutofadeTime = m_FadeTimeEdit.GetVal();
	theApp.GetMain()->GetMixer().SetAutofadeTime(m_mi.m_AutofadeTime);
}
