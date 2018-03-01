// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      28feb09	initial version
		01		31mar09	add open/save

        main mixer page
 
*/

#if !defined(AFX_MIXMISCDLG_H__51F7FA3A_F298_4A31_9EF4_DF86C4FDDE88__INCLUDED_)
#define AFX_MIXMISCDLG_H__51F7FA3A_F298_4A31_9EF4_DF86C4FDDE88__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MixMiscDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMixMiscDlg dialog

#include "MixerInfo.h"
#include "EditSliderCtrl.h"

class CFracticeView;
class CMainFrame;
class CMixer;

class CMixMiscDlg : public CPropertyPage
{
// Construction
public:
	CMixMiscDlg(CMixerInfo& Info);
	~CMixMiscDlg();

// Attributes

// Operations
	static	double	NormPalTweenTime(double Seconds);
	static	double	DenormPalTweenTime(double NormVal);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMixMiscDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Constants
	enum {	// define sliders; must match m_SliderInfo init in .cpp
		SL_TWEEN_TIME,
		SLIDERS
	};
	static const CEditSliderCtrl::INFO m_SliderInfo[SLIDERS];

// Dialog Data
	//{{AFX_DATA(CMixMiscDlg)
	enum { IDD = IDD_MIX_MISC };
	CEditSliderCtrl	m_TweenTimeSlider;
	CNumEdit	m_TweenTimeEdit;
	//}}AFX_DATA
	CComboBox	m_SrcCombo[MIXER_CHANS];

// Generated message map functions
	//{{AFX_MSG(CMixMiscDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangedTweenTimeEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnOpen();
	afx_msg void OnSave();
	//}}AFX_MSG
	afx_msg LRESULT OnQuerySiblings(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

// Data members
	CMixerInfo&	m_mi;			// mixer info reference

// Helpers
	void	UpdateCtrls(int CtrlID);
};

inline double CMixMiscDlg::NormPalTweenTime(double Seconds)
{
	return(CEditSliderCtrl::Norm(m_SliderInfo[SL_TWEEN_TIME], Seconds));
}

inline double CMixMiscDlg::DenormPalTweenTime(double NormVal)
{
	return(CEditSliderCtrl::Denorm(m_SliderInfo[SL_TWEEN_TIME], NormVal));
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MIXMISCDLG_H__51F7FA3A_F298_4A31_9EF4_DF86C4FDDE88__INCLUDED_)
