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

        color cycling dialog
 
*/

#if !defined(AFX_COLORCYCLINGDLG_H__51F7FA3A_F298_4A31_9EF4_DF86C4FDDE88__INCLUDED_)
#define AFX_COLORCYCLINGDLG_H__51F7FA3A_F298_4A31_9EF4_DF86C4FDDE88__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ColorCyclingDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CColorCyclingDlg dialog

#include "ChildDlg.h"
#include "PersistDlg.h"
#include "NumEdit.h"
#include "ClickSliderCtrl.h"
#include "CtrlResize.h"

class CFracticeView;

class CColorCyclingDlg : public CChildDlg
{
	DECLARE_DYNAMIC(CColorCyclingDlg);
// Construction
public:
	CColorCyclingDlg(CWnd* pParent = NULL);   // standard constructor

// Attributes
	void	SetView(CFracticeView *View);
	void	EnableCtrls(bool Enable);
	void	SetCycleRateNorm(CFracticeView *View, double Val);

// Operations
	void	Update();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CColorCyclingDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CColorCyclingDlg)
	enum { IDD = IDD_COLOR_CYCLING };
	CButton	m_CycleChk;
	CNumEdit	m_RateEdit;
	CClickSliderCtrl	m_RateSlider;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CColorCyclingDlg)
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnKillfocusRateEdit();
	virtual BOOL OnInitDialog();
	afx_msg void OnCycleChk();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	afx_msg LRESULT	OnDlgBarUpdate(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

// Constants
	enum {
		RATE_RANGE = 250,		// rate slider range
		RATE_TICS = 10,			// rate slider tic count
		RATE_PREC = 5,			// rate edit precision in digits
	};
	static const double	RATE_BASE;	// rate slider log base 
	static const double	RATE_MIN;	// rate slider minimum value
	static const double	RATE_MAX;	// rate slider maximum value
	static const CCtrlResize::CTRL_LIST	m_CtrlList[];	// resizable controls

// Data members
	CFracticeView	*m_View;	// pointer to current view
	bool	m_Cycling;			// true if we're cycling colors
	double	m_Rate;				// cycling rate, in cycles per second
	CCtrlResize	m_Resize;		// control resizer

// Helpers
	double	PosToRate(double Pos) const;
	double	RateToPos(double Rate) const;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLORCYCLINGDLG_H__51F7FA3A_F298_4A31_9EF4_DF86C4FDDE88__INCLUDED_)
