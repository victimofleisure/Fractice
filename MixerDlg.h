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
		03		25mar09	add FastSetFaderPos
		04		30mar09	extend get/set info to include doc paths
		05		01jun09	remove unused saved state

        mixer property sheet
 
*/

#if !defined(AFX_MIXERDLG_H__2EF17119_9713_40D8_8BA0_8D5A182DEA75__INCLUDED_)
#define AFX_MIXERDLG_H__2EF17119_9713_40D8_8BA0_8D5A182DEA75__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MixerDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMixerDlg

#include "MixerInfo.h"
#include "MixMainDlg.h"
#include "MixMotionDlg.h"
#include "MixMiscDlg.h"

class CMixerDlg : public CPropertySheet, protected CMixerInfo
{
	DECLARE_DYNAMIC(CMixerDlg)
// Construction
public:
	CMixerDlg(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

// Constants
	enum {	// query siblings codes
		QSC_UPDATECTRLS	= 1000,	// update page control(s) from member data;
								// to update a specific control, pass its
								// control ID in lParam, else all controls
								// in all pages are updated unconditionally
	};

// Attributes
	void	GetInfo(CMixerInfo& Info) const;
	void	SetInfo(const CMixerInfo& Info);
	void	GetInfoEx(CMixerInfo& Info) const;
	void	SetInfoEx(const CMixerInfo& Info);
	int		GetCurSel() const;
	void	SetCurSel(int ChanIdx);
	double	GetFaderPos() const;
	void	SetFaderPos(double Pos);
	double	GetAutofadeTimeNorm() const;
	void	SetAutofadeTimeNorm(double NormTime);
	void	SetAutofadePlay(bool Enable);
	void	SetAutofadeLoop(bool Enable);
	int		GetOriginMotion(int ChanIdx) const;
	void	SetOriginMotion(int ChanIdx, int OMType);
	bool	GetMirror() const;
	void	SetMirror(bool Enable);
	double	GetTempoNorm(int ChanIdx) const;
	void	SetTempoNorm(int ChanIdx, double NormTempo);
	double	GetDampingNorm(int ChanIdx) const;
	void	SetDampingNorm(int ChanIdx, double NormDamping);
	double	GetPongSpeedNorm(int ChanIdx) const;
	void	SetPongSpeedNorm(int ChanIdx, double NormSpeed);
	double	GetLissSpeedNorm(int ChanIdx) const;
	void	SetLissSpeedNorm(int ChanIdx, double NormSpeed);
	double	GetLissPhaseNorm(int ChanIdx) const;
	void	SetLissPhaseNorm(int ChanIdx, double NormPhase);
	double	GetPalTweenTimeNorm() const;
	void	SetPalTweenTimeNorm(double NormTime);
	void	FastSetFaderPos(int Pos);
	void	GetDefaults(CMixerInfo& Info) const;
	void	SetDefaults();
	
// Operations
	void	AddSource(CFracticeView *View);
	bool	RemoveSource(CFracticeView *View);
	void	UpdateAutofadeState();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMixerDlg)
	public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMixerDlg();

// Generated message map functions
protected:
	//{{AFX_MSG(CMixerDlg)
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Constants
	static const MIXER_CHAN_INFO	m_DefaultChanInfo;
	static const MIXER_MISC_INFO	m_DefaultMiscInfo;

// Member data
	CMixer	*m_Mixer;			// pointer to mixer
	CMixMainDlg	m_MainDlg;		// main page
	CMixMotionDlg	m_MotionDlg;	// motion page
	CMixMiscDlg	m_MiscDlg;		// miscellaneous page
	int		m_CurPage;			// index of current page

// Helpers
	void	ReadState();
	void	WriteState();
	void	UpdateCtrls(int CtrlID = 0);
	void	SetNormSlider(CWnd *Parent, int ResID, double Pos);
};

inline void CMixerDlg::GetInfo(CMixerInfo& Info) const
{
	Info = *this;
}

inline void CMixerDlg::AddSource(CFracticeView *View)
{
	m_MainDlg.AddSource(View);
}

inline bool CMixerDlg::RemoveSource(CFracticeView *View)
{
	return(m_MainDlg.RemoveSource(View));
}

inline int CMixerDlg::GetCurSel() const
{
	return(m_CurChan);
}

inline int CMixerDlg::GetOriginMotion(int ChanIdx) const
{
	return(GetChan(ChanIdx).m_OrgMotion);
}

inline bool CMixerDlg::GetMirror() const
{
	return(m_Mirror != 0);
}

inline void CMixerDlg::FastSetFaderPos(int Pos)
{
	m_MainDlg.FastSetFaderPos(Pos);
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MIXERDLG_H__2EF17119_9713_40D8_8BA0_8D5A182DEA75__INCLUDED_)
