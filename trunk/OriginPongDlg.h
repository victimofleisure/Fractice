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

#if !defined(AFX_ORIGINPONGDLG_H__BA7627CB_6DA4_406C_B5BD_5251458C0809__INCLUDED_)
#define AFX_ORIGINPONGDLG_H__BA7627CB_6DA4_406C_B5BD_5251458C0809__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OriginPongDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COriginPongDlg dialog

#include "ChildDlg.h"
#include "EditSliderCtrl.h"

class CMixerInfo;

class COriginPongDlg : public CChildDlg
{
// Construction
public:
	COriginPongDlg(CMixerInfo& Info, CWnd *pParent = NULL);

// Operations
	static	double	NormSpeed(double Speed);
	static	double	DenormSpeed(double NormVal);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COriginPongDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(COriginPongDlg)
	enum { IDD = IDD_ORIGIN_PONG };
	CNumEdit	m_SpeedEdit;
	CEditSliderCtrl	m_SpeedSlider;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(COriginPongDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangedSpeedEdit(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Constants
	enum {	// define sliders; must match m_SliderInfo init in .cpp
		SL_SPEED,
		SLIDERS
	};
	static const CEditSliderCtrl::INFO m_SliderInfo[SLIDERS];

// Data members
	CMixerInfo&	m_mi;	// mixer info reference

// Helpers
};

inline double COriginPongDlg::NormSpeed(double Speed)
{
	return(CEditSliderCtrl::Norm(m_SliderInfo[SL_SPEED], Speed));
}

inline double COriginPongDlg::DenormSpeed(double NormVal)
{
	return(CEditSliderCtrl::Denorm(m_SliderInfo[SL_SPEED], NormVal));
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ORIGINPONGDLG_H__BA7627CB_6DA4_406C_B5BD_5251458C0809__INCLUDED_)
