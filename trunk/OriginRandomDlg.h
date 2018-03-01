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

#if !defined(AFX_ORIGINRANDOMDLG_H__BA7627CB_6DA4_406C_B5BD_5251458C0809__INCLUDED_)
#define AFX_ORIGINRANDOMDLG_H__BA7627CB_6DA4_406C_B5BD_5251458C0809__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OriginRandomDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COriginRandomDlg dialog

#include "OriginDragDlg.h"

class COriginRandomDlg : public COriginDragDlg
{
// Construction
public:
	COriginRandomDlg(CMixerInfo& Info, CWnd *pParent = NULL);

// Operations
	static	double	NormTempo(double Tempo);
	static	double	DenormTempo(double NormVal);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COriginRandomDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(COriginRandomDlg)
	enum { IDD = IDD_ORIGIN_RANDOM };
	CNumEdit	m_TempoEdit;
	CEditSliderCtrl	m_TempoSlider;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(COriginRandomDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangedTempoEdit(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Constants
	enum {	// define sliders; must match m_SliderInfo init in .cpp
		SL_TEMPO,
		SLIDERS
	};
	static const CEditSliderCtrl::INFO m_SliderInfo[SLIDERS];

// Data members

// Helpers
};

inline double COriginRandomDlg::NormTempo(double Tempo)
{
	return(CEditSliderCtrl::Norm(m_SliderInfo[SL_TEMPO], Tempo));
}

inline double COriginRandomDlg::DenormTempo(double NormVal)
{
	return(CEditSliderCtrl::Denorm(m_SliderInfo[SL_TEMPO], NormVal));
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ORIGINRANDOMDLG_H__BA7627CB_6DA4_406C_B5BD_5251458C0809__INCLUDED_)
