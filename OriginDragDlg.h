// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      16feb09	initial version
		01		29mar09	use classic damping norm/denorm

        cursor drag origin motion page
 
*/

#if !defined(AFX_ORIGINDRAGDLG_H__BA7627CB_6DA4_406C_B5BD_5251458C0809__INCLUDED_)
#define AFX_ORIGINDRAGDLG_H__BA7627CB_6DA4_406C_B5BD_5251458C0809__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OriginDragDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COriginDragDlg dialog

#include "ChildDlg.h"
#include "EditSliderCtrl.h"

class CMixerInfo;

class COriginDragDlg : public CChildDlg
{
// Construction
public:
	COriginDragDlg(CMixerInfo& Info, CWnd *pParent = NULL);

// Operations
	static	double	Log(double Base, double x);
	static	double	LogNorm(double x, double Base, double Scale);
	static	double	ExpNorm(double x, double Base, double Scale);
	static	double	NormDamping(double Damping);
	static	double	DenormDamping(double NormVal);
	static	double	NormDampingSliderPos(double Damping);
	static	double	DenormDampingSliderPos(double Pos);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COriginDragDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Types
	class CDampingSlider : public CEditSliderCtrl {
	protected:
		double	Norm(double x) const;
		double	Denorm(double x) const;
	};
	class CDampingEdit : public CNumEdit {
	protected:
		static const double	SCALE;
		void	ValToStr(CString& Str);
		void	StrToVal(LPCTSTR Str);
	};

// Dialog Data
	//{{AFX_DATA(COriginDragDlg)
	enum { IDD = IDD_ORIGIN_DRAG };
	CDampingEdit	m_DampingEdit;
	CDampingSlider	m_DampingSlider;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(COriginDragDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangedDampingEdit(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Constants
	enum {	// define sliders; must match m_SliderInfo init in .cpp
		SL_DAMPING,
		SLIDERS
	};
	static const CEditSliderCtrl::INFO m_SliderInfo[SLIDERS];

// Data members
	CMixerInfo&	m_mi;	// mixer info reference

// Helpers
	friend	CDampingSlider;
	friend	CDampingEdit;
};

inline double COriginDragDlg::NormDamping(double x)
{
	return(NormDampingSliderPos(x) / m_SliderInfo[SL_DAMPING].RangeMax);
}

inline double COriginDragDlg::DenormDamping(double x)
{
	return(DenormDampingSliderPos(x * m_SliderInfo[SL_DAMPING].RangeMax));
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ORIGINDRAGDLG_H__BA7627CB_6DA4_406C_B5BD_5251458C0809__INCLUDED_)
