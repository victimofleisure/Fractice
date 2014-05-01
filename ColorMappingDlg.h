// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      02feb09	initial version
		01		23mar09	add view argument to normalized accessors

        color mapping dialog
 
*/

#if !defined(AFX_COLORMAPPINGDLG_H__C38472C8_C8F4_4C06_B5F0_E71B96B5679E__INCLUDED_)
#define AFX_COLORMAPPINGDLG_H__C38472C8_C8F4_4C06_B5F0_E71B96B5679E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ColorMappingDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CColorMappingDlg dialog

#include "ChildDlg.h"
#include "NumEdit.h"
#include "ClickSliderCtrl.h"
#include "CtrlResize.h"

class CFracticeView;

class CColorMappingDlg : public CChildDlg
{
	DECLARE_DYNAMIC(CColorMappingDlg);
// Construction
public:
	CColorMappingDlg(CWnd* pParent = NULL);   // standard constructor

// Attributes
	void	SetView(CFracticeView *View);
	void	EnableCtrls(bool Enable);
	void	SetCycleLengthNorm(CFracticeView *View, double Val);
	void	SetColorOffsetNorm(CFracticeView *View, double Val);

// Operations
	void	Update();
	static	double	Norm(double Val, double Base, double MinVal, double MaxVal);
	static	double	Denorm(double Val, double Base, double MinVal, double MaxVal);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CColorMappingDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CColorMappingDlg)
	enum { IDD = IDD_COLOR_MAPPING };
	CNumEdit	m_CycleLenEdit;
	CNumEdit	m_ColorOfsEdit;
	CClickSliderCtrl	m_ColorOfsSlider;
	CClickSliderCtrl	m_CycleLenSlider;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CColorMappingDlg)
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	virtual BOOL OnInitDialog();
	afx_msg void OnChangedCycleLenEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangedColorOfsEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	afx_msg LRESULT	OnDlgBarUpdate(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

// Constants
	enum {
		CYCLE_LEN_RANGE = 250,		// cycle length slider range
		CYCLE_LEN_TICS = 32,		// cycle length slider tic count
		COLOR_OFS_RANGE = 250,		// color offset slider range
		COLOR_OFS_TICS = 10,		// color offset slider tic count
	};
	static const double	CYCLE_LEN_BASE;	// cycle length slider log base
	static const double	CYCLE_LEN_QUAL_RATIO;	// ratio of cycle length to quality
	static const CCtrlResize::CTRL_LIST	m_CtrlList[];	// resizable controls

// Member data
	CFracticeView	*m_View;	// pointer to current view
	int		m_Quality;		// iteration limit
	int		m_CycleLen;		// palette cycle length
	double	m_ColorOfs;		// palette color offset
	bool	m_AllowUpdate;	// if false, Update does nothing
	CCtrlResize	m_Resize;	// control resizer

// Helpers
	double	GetCycleLenBase() const;
	double	PosToCycleLen(double Pos) const;
	double	CycleLenToPos(double CycleLen) const;
	double	PosToColorOfs(double Pos) const;
	double	ColorOfsToPos(double ColorOfs) const;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLORMAPPINGDLG_H__C38472C8_C8F4_4C06_B5F0_E71B96B5679E__INCLUDED_)
