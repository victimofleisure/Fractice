// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
		00		17oct07	initial version
		01		08nov07	make Get/SetEditVal const
		02		05mar08	add IsDragging, send notification instead of app msg

		color picker dialog
 
*/

#if !defined(AFX_COLORPICKERDLG_H__76C45DE0_2376_4CF6_AAB7_9094498B7C48__INCLUDED_)
#define AFX_COLORPICKERDLG_H__76C45DE0_2376_4CF6_AAB7_9094498B7C48__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ColorPickerDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CColorPickerDlg dialog

#include "ColorPickerCtrl.h"
#include "ShadePickerCtrl.h"

class CColorPickerDlg : public CDialog
{
	DECLARE_DYNAMIC(CColorPickerDlg);
// Construction
public:
	CColorPickerDlg(int ResID = IDD_COLOR_PICKER, CWnd* pParent = NULL);

// Attributes
	void	SetColor(const DRGB& Color);
	void	GetColor(DRGB& Color) const;
	void	SetColor(const DHLS& Color);
	void	GetColor(DHLS& Color) const;
	void	SetColor(COLORREF Color);
	COLORREF	GetColor() const;
	bool	IsDragging() const;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CColorPickerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CColorPickerDlg)
	enum { IDD = IDD_COLOR_PICKER };
	CColorPickerCtrl	m_ColorPicker;
	CShadePickerCtrl	m_ShadePicker;
	CStatic	m_Swatch;
	CEdit	m_EditR;
	CEdit	m_EditG;
	CEdit	m_EditB;
	CEdit	m_EditH;
	CEdit	m_EditL;
	CEdit	m_EditS;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CColorPickerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnColorChange();
	afx_msg void OnKillFocusR();
	afx_msg void OnKillFocusG();
	afx_msg void OnKillFocusB();
	afx_msg void OnKillFocusH();
	afx_msg void OnKillFocusL();
	afx_msg void OnKillFocusS();
	//}}AFX_MSG
	afx_msg void OnColorPick(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

// Data members
	CBrush	m_SwatchBrush;

// Overridables
	virtual	void	UpdateColor();

// Helpers
	void	SetRGB(int Chan, double Val);
	void	SetHLS(int Chan, double Val);
	double	GetEditVal(CEdit& Edit) const;
	void	SetEditVal(CEdit& Edit, double Val) const;
};

inline void CColorPickerDlg::GetColor(DRGB& Color) const
{
	m_ColorPicker.GetColor(Color);
}

inline void	CColorPickerDlg::GetColor(DHLS& Color) const
{
	m_ColorPicker.GetColor(Color);
}

inline COLORREF CColorPickerDlg::GetColor() const
{
	return(m_ColorPicker.GetColor());
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLORPICKERDLG_H__76C45DE0_2376_4CF6_AAB7_9094498B7C48__INCLUDED_)
