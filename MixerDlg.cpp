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
		02		23mar09	add channel index to channel property accessors
		03		28mar09	get target mixer channel from ChanIdx, not m_CurChan
		04		30mar09	extend get/set info to include doc paths
		05		01jun09	in OnInitDialog, remove unused saved state

        mixer property sheet
 
*/

// MixerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Fractice.h"
#include "MixerDlg.h"
#include "FracticeDoc.h"
#include "FracticeView.h"
#include "Packets.h"
#include "MainFrm.h"
#include "OriginPongDlg.h"
#include "OriginDragDlg.h"
#include "OriginRandomDlg.h"
#include "OriginLissajousDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMixerDlg

IMPLEMENT_DYNAMIC(CMixerDlg, CPropertySheet)

#define RK_DEF_SNAPSHOT		_T("DefSnapshot")

const MIXER_CHAN_INFO CMixerDlg::m_DefaultChanInfo = {
	CMixer::OM_LISSAJOUS,	// m_OrgMotion
	1,		// m_PongSpeed
	.19,	// m_Damping
	60,		// m_Tempo
	.001,	// m_LissSpeed
	8,		// m_LissXFreq
	9,		// m_LissYFreq
	0,		// m_LissPhase
};

const MIXER_MISC_INFO CMixerDlg::m_DefaultMiscInfo = {
	0,		// m_FaderPos
	FALSE,	// m_AutofadePlay
	FALSE,	// m_AutofadeLoop
	10,		// m_AutofadeTime
	10,		// m_PalTweenTime
	TRUE,	// m_Mirror
	0,		// m_CurChan
};

#define RK_AUTOFADE_LOOP	_T("AutofadeLoop")
#define RK_AUTOFADE_TIME	_T("AutofadeTime")
#define RK_PAL_TWEEN_TIME	_T("PalTweenTime")
#define RK_MIRROR			_T("Mirror")
#define RK_CHAN_SECTION		_T("%s\\Chan%d")
#define RK_ORG_MOTION		_T("OrgMotion")
#define RK_PONG_SPEED		_T("PongSpeed")
#define RK_DAMPING			_T("Damping")
#define RK_TEMPO			_T("Tempo")
#define RK_LISS_SPEED		_T("LissaSpeed")
#define RK_LISS_XFREQ		_T("LissaXFreq")
#define RK_LISS_YFREQ		_T("LissaYFreq")
#define RK_LISS_PHASE		_T("LissaPhase")

CMixerDlg::CMixerDlg(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage),
	CMixerInfo(m_DefaultMiscInfo, m_DefaultChanInfo),
#pragma warning(push)
#pragma warning(disable : 4355)	// 'this' used in base member initializer list
	m_MainDlg(*this),	// passing reference to our CMixerInfo base class 
	m_MotionDlg(*this),
	m_MiscDlg(*this)
#pragma warning(pop)
{
	m_Mixer = NULL;
	m_CurPage = 0;
	ReadState();
	AddPage(&m_MainDlg);
	AddPage(&m_MotionDlg);
	AddPage(&m_MiscDlg);
}

CMixerDlg::~CMixerDlg()
{
	WriteState();
}

void CMixerDlg::ReadState()
{
	m_AutofadeLoop = CPersist::GetInt(REG_MIXER, RK_AUTOFADE_LOOP, m_AutofadeLoop);
	m_AutofadeTime = CPersist::GetDouble(REG_MIXER, RK_AUTOFADE_TIME, m_AutofadeTime);
	m_PalTweenTime = CPersist::GetDouble(REG_MIXER, RK_PAL_TWEEN_TIME, m_PalTweenTime);
	m_Mirror = CPersist::GetInt(REG_MIXER, RK_MIRROR, m_Mirror);
	for (int i = 0; i < MIXER_CHANS; i++) {
		CString	s;
		s.Format(RK_CHAN_SECTION, REG_MIXER, i);
		MIXER_CHAN_INFO&	ci = m_Chan[i];
		ci.m_OrgMotion	= CPersist::GetInt(s, RK_ORG_MOTION, ci.m_OrgMotion);
		ci.m_PongSpeed	= CPersist::GetInt(s, RK_PONG_SPEED, ci.m_PongSpeed);
		ci.m_Damping	= CPersist::GetDouble(s, RK_DAMPING, ci.m_Damping);
		ci.m_Tempo		= CPersist::GetDouble(s, RK_TEMPO, ci.m_Tempo);
		ci.m_LissSpeed	= CPersist::GetDouble(s, RK_LISS_SPEED, ci.m_LissSpeed);
		ci.m_LissXFreq	= CPersist::GetDouble(s, RK_LISS_XFREQ, ci.m_LissXFreq);
		ci.m_LissYFreq	= CPersist::GetDouble(s, RK_LISS_YFREQ, ci.m_LissYFreq);
		ci.m_LissPhase	= CPersist::GetDouble(s, RK_LISS_PHASE, ci.m_LissPhase);
	}
}

void CMixerDlg::WriteState()
{
	CPersist::WriteInt(REG_MIXER, RK_AUTOFADE_LOOP, m_AutofadeLoop);
	CPersist::WriteDouble(REG_MIXER, RK_AUTOFADE_TIME, m_AutofadeTime);
	CPersist::WriteDouble(REG_MIXER, RK_PAL_TWEEN_TIME, m_PalTweenTime);
	CPersist::WriteInt(REG_MIXER, RK_MIRROR, m_Mirror);
	for (int i = 0; i < MIXER_CHANS; i++) {
		CString	s;
		s.Format(RK_CHAN_SECTION, REG_MIXER, i);
		MIXER_CHAN_INFO&	ci = m_Chan[i];
		CPersist::WriteInt(s, RK_ORG_MOTION, ci.m_OrgMotion);
		CPersist::WriteInt(s, RK_PONG_SPEED, ci.m_PongSpeed);
		CPersist::WriteDouble(s, RK_DAMPING, ci.m_Damping);
		CPersist::WriteDouble(s, RK_TEMPO, ci.m_Tempo);
		CPersist::WriteDouble(s, RK_LISS_SPEED, ci.m_LissSpeed);
		CPersist::WriteDouble(s, RK_LISS_XFREQ, ci.m_LissXFreq);
		CPersist::WriteDouble(s, RK_LISS_YFREQ, ci.m_LissYFreq);
		CPersist::WriteDouble(s, RK_LISS_PHASE, ci.m_LissPhase);
	}
}

void CMixerDlg::UpdateCtrls(int CtrlID)
{
	ASSERT(IsWindow(m_MainDlg));	// first page must exist
	m_MainDlg.QuerySiblings(QSC_UPDATECTRLS, CtrlID);
}

void CMixerDlg::SetInfo(const CMixerInfo& Info)
{
	CMixerInfo&	oi = *this;	// downcast to base class
	oi = Info;
	UpdateCtrls();
	m_Mixer->ApplySettings();
}

void CMixerDlg::GetInfoEx(CMixerInfo& Info) const
{
	GetInfo(Info);
	Info.m_FaderPos = GetFaderPos();
	Info.m_DocPath.SetSize(MIXER_CHANS);
	for (int i = 0; i < MIXER_CHANS; i++) {
		CFracticeView	*View = m_Mixer->GetView(i);
		if (View != NULL)
			Info.m_DocPath[i] = View->GetDocument()->GetPathName();
	}
}

void CMixerDlg::SetInfoEx(const CMixerInfo& Info)
{
	SetInfo(Info);
	if (Info.m_DocPath.GetSize() == MIXER_CHANS) {
		for (int i = 0; i < MIXER_CHANS; i++) {
			if (!m_DocPath[i].IsEmpty()
			&& theApp.OpenDocumentFile(m_DocPath[i]) != NULL)
				m_MainDlg.SelectSource(i, theApp.GetMain()->GetView());
		}
	}
}

void CMixerDlg::GetDefaults(CMixerInfo& Info) const
{
	Info.SetMiscInfo(m_DefaultMiscInfo);
	for (int i = 0; i < MIXER_CHANS; i++)
		Info.SetChan(i, m_DefaultChanInfo);
}

void CMixerDlg::SetDefaults()
{
	CMixerInfo	mi;
	GetDefaults(mi);
	SetInfo(mi);
}

void CMixerDlg::SetNormSlider(CWnd *Parent, int ResID, double Pos)
{
	if (Parent != NULL) {	// if control's parent was specified
		if (!IsWindow(*Parent))	// if parent window doesn't exist
			return;	// nothing to do
	} else	// parent wasn't specified
		Parent = this;	// assume we're the parent
	CWnd	*pDesc = Parent->GetDescendantWindow(ResID);
	CEditSliderCtrl	*pES = DYNAMIC_DOWNCAST(CEditSliderCtrl, pDesc);
	if (pES != NULL)	// control and its parent may not exist
		pES->SetValNorm(Pos);
}

void CMixerDlg::SetCurSel(int ChanIdx)
{
	m_CurChan = ChanIdx;
	UpdateCtrls(IDC_MIX_CHAN_SELECT);
}

double CMixerDlg::GetFaderPos() const
{
	return(m_Mixer->GetFaderPos());
}

void CMixerDlg::SetFaderPos(double Pos)
{
	m_MainDlg.SetFaderPos(Pos);
}

double CMixerDlg::GetAutofadeTimeNorm() const
{
	return(CMixMainDlg::NormAutofadeTime(m_AutofadeTime));
}

void CMixerDlg::SetAutofadeTimeNorm(double NormTime)
{
	SetNormSlider(&m_MainDlg, IDC_MIX_AUTOFADE_TIME_SLIDER, NormTime);
	m_AutofadeTime = CMixMainDlg::DenormAutofadeTime(NormTime);
	m_Mixer->SetAutofadeTime(m_AutofadeTime);
}

void CMixerDlg::SetAutofadePlay(bool Enable)
{
	m_MainDlg.SetPlay(Enable);
}

void CMixerDlg::SetAutofadeLoop(bool Enable)
{
	m_MainDlg.SetLoop(Enable);
}

void CMixerDlg::UpdateAutofadeState()
{
	m_MainDlg.SetPlay(m_Mixer->GetPlay());
	m_MainDlg.SetFaderPos(m_Mixer->GetFaderPos());
}

void CMixerDlg::SetOriginMotion(int ChanIdx, int OMType)
{
	GetChan(ChanIdx).m_OrgMotion = OMType;
	if (ChanIdx == m_CurChan)	// if selected channel, update UI
		UpdateCtrls(IDC_MIX_ORG_MOTION_COMBO);
	m_Mixer->SetOriginMotion(ChanIdx, OMType);
}

void CMixerDlg::SetMirror(bool Enable)
{
	m_Mirror = Enable;
	UpdateCtrls(IDC_MIX_MIRROR_CHK);
	m_Mixer->SetMirror(Enable);
}

double CMixerDlg::GetTempoNorm(int ChanIdx) const
{
	return(COriginRandomDlg::NormTempo(GetChan(ChanIdx).m_Tempo));
}

void CMixerDlg::SetTempoNorm(int ChanIdx, double NormTempo)
{
	if (ChanIdx == m_CurChan)	// if selected channel, update UI
		SetNormSlider(&m_MotionDlg, IDC_ORG_TEMPO_SLIDER, NormTempo);
	double	Tempo = COriginRandomDlg::DenormTempo(NormTempo);
	GetChan(ChanIdx).m_Tempo = Tempo;
	m_Mixer->SetTempo(ChanIdx, Tempo);
}

double CMixerDlg::GetDampingNorm(int ChanIdx) const
{
	return(COriginDragDlg::NormDamping(GetChan(ChanIdx).m_Damping));
}

void CMixerDlg::SetDampingNorm(int ChanIdx, double NormDamping)
{
	if (ChanIdx == m_CurChan)	// if selected channel, update UI
		SetNormSlider(&m_MotionDlg, IDC_ORG_DAMPING_SLIDER, NormDamping);
	double	Damping = COriginDragDlg::DenormDamping(NormDamping);
	GetChan(ChanIdx).m_Damping = Damping;
	m_Mixer->SetDamping(ChanIdx, Damping);
}

double CMixerDlg::GetPongSpeedNorm(int ChanIdx) const
{
	return(COriginPongDlg::NormSpeed(GetChan(ChanIdx).m_PongSpeed));
}

void CMixerDlg::SetPongSpeedNorm(int ChanIdx, double NormSpeed)
{
	if (ChanIdx == m_CurChan)	// if selected channel, update UI
		SetNormSlider(&m_MotionDlg, IDC_ORG_PONG_SPEED_SLIDER, NormSpeed);
	UINT	Speed = round(COriginPongDlg::DenormSpeed(NormSpeed));
	GetChan(ChanIdx).m_PongSpeed = Speed;
	m_Mixer->SetPongSpeed(ChanIdx, Speed);
}

double CMixerDlg::GetLissSpeedNorm(int ChanIdx) const
{
	return(COriginLissajousDlg::NormSpeed(GetChan(ChanIdx).m_LissSpeed));
}

void CMixerDlg::SetLissSpeedNorm(int ChanIdx, double NormSpeed)
{
	if (ChanIdx == m_CurChan)	// if selected channel, update UI
		SetNormSlider(&m_MotionDlg, IDC_ORG_LISS_SPEED_SLIDER, NormSpeed);
	double	Speed = COriginLissajousDlg::DenormSpeed(NormSpeed);
	GetChan(ChanIdx).m_LissSpeed = Speed;
	m_Mixer->SetLissajousSpeed(ChanIdx, Speed);
}

double CMixerDlg::GetLissPhaseNorm(int ChanIdx) const
{
	return(COriginLissajousDlg::NormPhase(GetChan(ChanIdx).m_LissPhase));
}

void CMixerDlg::SetLissPhaseNorm(int ChanIdx, double NormPhase)
{
	if (ChanIdx == m_CurChan)	// if selected channel, update UI
		SetNormSlider(&m_MotionDlg, IDC_ORG_LISS_PHASE_SLIDER, NormPhase);
	double	Phase = COriginLissajousDlg::DenormPhase(NormPhase);
	GetChan(ChanIdx).m_LissPhase = Phase;
	m_Mixer->SetLissajousPhase(ChanIdx, Phase);
}

double CMixerDlg::GetPalTweenTimeNorm() const
{
	return(CMixMiscDlg::NormPalTweenTime(m_PalTweenTime));
}

void CMixerDlg::SetPalTweenTimeNorm(double NormTime)
{
	SetNormSlider(&m_MiscDlg, IDC_MIX_TWEEN_TIME_SLIDER, NormTime);
	m_PalTweenTime = CMixMiscDlg::DenormPalTweenTime(NormTime);
	m_Mixer->SetPalTweenTime(m_PalTweenTime);
}

BEGIN_MESSAGE_MAP(CMixerDlg, CPropertySheet)
	//{{AFX_MSG_MAP(CMixerDlg)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMixerDlg message handlers

BOOL CMixerDlg::OnInitDialog() 
{
	BOOL bResult = CPropertySheet::OnInitDialog();
	SetActivePage(m_CurPage);	// set current page
	m_Mixer = &theApp.GetMain()->GetMixer();
	return bResult;
}

void CMixerDlg::OnDestroy() 
{
	m_CurPage = GetActiveIndex();
	CPropertySheet::OnDestroy();
}

BOOL CMixerDlg::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message >= WM_KEYFIRST || pMsg->message <= WM_KEYLAST) {
		if (theApp.HandleDlgKeyMsg(pMsg))	// give main accelerators a try
			return(TRUE);	// message was translated, stop dispatching
	}
	return CPropertySheet::PreTranslateMessage(pMsg);
}
