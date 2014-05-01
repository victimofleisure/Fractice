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
		03		25mar09	add FastSetFaderPos
		04		06jan10	W64: in GetSourceCount, cast source array size to 32-bit

        main mixer page
 
*/

#if !defined(AFX_MIXMAINDLG_H__51F7FA3A_F298_4A31_9EF4_DF86C4FDDE88__INCLUDED_)
#define AFX_MIXMAINDLG_H__51F7FA3A_F298_4A31_9EF4_DF86C4FDDE88__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MixMainDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMixMainDlg dialog

#include "MixerInfo.h"
#include "EditSliderCtrl.h"
#include "IconButton.h"

class CFracticeView;
class CMainFrame;
class CMixer;

class CMixMainDlg : public CPropertyPage
{
// Construction
public:
	CMixMainDlg(CMixerInfo& Info);
	~CMixMainDlg();

// Constants
	enum {
		FADER_RANGE = 200,
	};

// Attributes
	int		GetSourceCount() const;
	CFracticeView *GetSource(int SrcIdx);
	void	SetPlay(bool Enable);
	void	SetLoop(bool Enable);
	void	SetFaderPos(double Pos);
	void	FastSetFaderPos(int Pos);

// Operations
	void	AddSource(CFracticeView *View);
	bool	RemoveSource(CFracticeView *View);
	bool	SelectSource(int ChanIdx, CFracticeView *View);
	static	double	NormAutofadeTime(double Seconds);
	static	double	DenormAutofadeTime(double NormVal);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMixMainDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Constants
	enum {	// define sliders; must match m_SliderInfo init in .cpp
		SL_FADE_TIME,
		SLIDERS
	};
	static const CEditSliderCtrl::INFO m_SliderInfo[SLIDERS];

// Dialog Data
	//{{AFX_DATA(CMixMainDlg)
	enum { IDD = IDD_MIX_MAIN };
	CEditSliderCtrl	m_FadeTimeSlider;
	CNumEdit	m_FadeTimeEdit;
	CIconButton	m_StopBtn;
	CIconButton	m_PlayBtn;
	CIconButton	m_LoopBtn;
	CClickSliderCtrl	m_Fader;
	//}}AFX_DATA
	CComboBox	m_SrcCombo[MIXER_CHANS];

// Generated message map functions
	//{{AFX_MSG(CMixMainDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeSrcACombo();
	afx_msg void OnSelchangeSrcBCombo();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnLoop();
	afx_msg void OnPlay();
	afx_msg void OnStop();
	afx_msg void OnChangedFadeTimeEdit(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	afx_msg LRESULT OnQuerySiblings(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

// Data members
	CMixerInfo&	m_mi;			// mixer info reference
	CPtrArray	m_Source;		// array of source view pointers
	CMainFrame	*m_Main;		// pointer to main frame
	CMixer	*m_Mixer;			// pointer to mixer

// Helpers
	void	UpdateCtrls(int CtrlID);
	int		FindSource(CFracticeView *View) const;
	void	UpdateSourceCombos();
};

inline int CMixMainDlg::GetSourceCount() const
{
	return(INT64TO32(m_Source.GetSize()));
}

inline CFracticeView *CMixMainDlg::GetSource(int SrcIdx)
{
	return((CFracticeView *)m_Source[SrcIdx]);
}

inline double CMixMainDlg::NormAutofadeTime(double Seconds)
{
	return(CEditSliderCtrl::Norm(m_SliderInfo[SL_FADE_TIME], Seconds));
}

inline double CMixMainDlg::DenormAutofadeTime(double NormVal)
{
	return(CEditSliderCtrl::Denorm(m_SliderInfo[SL_FADE_TIME], NormVal));
}

inline void CMixMainDlg::FastSetFaderPos(int Pos)
{
	m_Fader.SetPos(Pos);
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MIXMAINDLG_H__51F7FA3A_F298_4A31_9EF4_DF86C4FDDE88__INCLUDED_)
