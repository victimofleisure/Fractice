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

#if !defined(AFX_ORIGINLISSAJOUSDLG_H__BA7627CB_6DA4_406C_B5BD_5251458C0809__INCLUDED_)
#define AFX_ORIGINLISSAJOUSDLG_H__BA7627CB_6DA4_406C_B5BD_5251458C0809__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OriginLissajousDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COriginLissajousDlg dialog

#include "ChildDlg.h"
#include "EditSliderCtrl.h"

class CMixerInfo;

class COriginLissajousDlg : public CChildDlg
{
// Construction
public:
	COriginLissajousDlg(CMixerInfo& Info, CWnd *pParent = NULL);

// Operations
	static	double	NormSpeed(double Speed);
	static	double	DenormSpeed(double NormVal);
	static	double	NormPhase(double Speed);
	static	double	DenormPhase(double NormVal);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COriginLissajousDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(COriginLissajousDlg)
	enum { IDD = IDD_ORIGIN_LISSAJOUS };
	CNumEdit	m_SpeedEdit;
	CEditSliderCtrl	m_SpeedSlider;
	CNumEdit	m_XFreqEdit;
	CEditSliderCtrl	m_XFreqSlider;
	CNumEdit	m_YFreqEdit;
	CEditSliderCtrl	m_YFreqSlider;
	CNumEdit	m_PhaseEdit;
	CEditSliderCtrl	m_PhaseSlider;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(COriginLissajousDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangedXFreqEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangedYFreqEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangedSpeedEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangedPhaseEdit(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Constants
	enum {	// define sliders; must match m_SliderInfo init in .cpp
		SL_SPEED,
		SL_XFREQ,
		SL_YFREQ,
		SL_PHASE,
		SLIDERS
	};
	static const CEditSliderCtrl::INFO m_SliderInfo[SLIDERS];

// Data members
	CMixerInfo&	m_mi;	// mixer info reference

// Helpers
};

inline double COriginLissajousDlg::NormSpeed(double Speed)
{
	return(CEditSliderCtrl::Norm(m_SliderInfo[SL_SPEED], Speed));
}

inline double COriginLissajousDlg::DenormSpeed(double NormVal)
{
	return(CEditSliderCtrl::Denorm(m_SliderInfo[SL_SPEED], NormVal));
}

inline double COriginLissajousDlg::NormPhase(double Phase)
{
	return(CEditSliderCtrl::Norm(m_SliderInfo[SL_PHASE], Phase));
}

inline double COriginLissajousDlg::DenormPhase(double NormVal)
{
	return(CEditSliderCtrl::Denorm(m_SliderInfo[SL_PHASE], NormVal));
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ORIGINLISSAJOUSDLG_H__BA7627CB_6DA4_406C_B5BD_5251458C0809__INCLUDED_)
