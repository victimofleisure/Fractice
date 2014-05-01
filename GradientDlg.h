// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		25jan09	implement dragging multiple selection

        gradient dialog
 
*/

#if !defined(AFX_GRADIENTDLG_H__1C517929_3A1F_4497_98AB_0772D83B313A__INCLUDED_)
#define AFX_GRADIENTDLG_H__1C517929_3A1F_4497_98AB_0772D83B313A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GradientDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGradientDlg dialog

#include "ArrayEx.h"
#include "ColorPickerDlg.h"
#include "GradientCtrl.h"
#include "NumEdit.h"

class CGradientDlg : public CColorPickerDlg
{
	DECLARE_DYNAMIC(CGradientDlg);
// Construction
public:
	CGradientDlg(int ResID = IDD_GRADIENT, CWnd* pParent = NULL);

// Attributes
	void	GetGradient(CGradient& Grad) const;
	void	SetGradient(const CGradient& Grad);
	void	GetTriplet(GRADIENT_COLOR& Color) const;
	void	SetTriplet(const GRADIENT_COLOR& Color);
	bool	IsDragging() const;
	int		GetCount() const;
	void	GetSelection(CDWordArray& SelIdx) const;
	void	SetSelection(const CDWordArray& SelIdx);
	double	GetHueRotation() const;
	void	SetHueRotation(double Rotation);

// Operations
	bool	CtrlHitTest(CPoint point);
	int		FindMarker(CPoint point);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGradientDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CGradientDlg)
	enum { IDD = IDD_GRADIENT };
	CButton	m_SpaceEvenlyChk;
	CNumEdit	m_PosEdit;
	CGradientCtrl	m_Grad;
	int		m_ColorMode;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CGradientDlg)
	afx_msg void OnPaint();
	afx_msg void OnInsert();
	afx_msg void OnDelete();
	afx_msg void OnInsertAt();
	virtual BOOL OnInitDialog();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnColorMode();
	afx_msg void OnKillfocusPosEdit();
	afx_msg void OnSpaceEvenly();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	virtual void OnOK();
	afx_msg void OnParentNotify(UINT message, LPARAM lParam);
	afx_msg	void OnUpdateDelete(CCmdUI* pCmdUI);
	afx_msg void OnReverseCycle();
	afx_msg void OnInvertColor();
	afx_msg void OnRotateHue();
	afx_msg void OnSelectAll();
	//}}AFX_MSG
	afx_msg LRESULT OnKickIdle(WPARAM wParam, LPARAM lParam);
	afx_msg void OnGradientChange(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

// Constants
	enum {
		DRAG_THRESHOLD = 2,	// minimum cursor movement to start drag, in client coords
		MARKER_BOTTOM_MARGIN = 8,	// extra space below markers for deselecting them
	};
	enum {	// drag states
		DTS_NONE,		// left button is up
		DTS_TRACK,		// motion hasn't reached drag threshold
		DTS_DRAG,		// drag is in progress
		DTS_EYEDROPPER,	// eyedropper is in progress
		DTS_ROTATE		// rotate gradient is in progress
	};

// Member data
	int		m_DragState;	// true if we're dragging point
	CPoint	m_DragOrigin;	// point where left button was pressed, in client coords
	CPoint	m_ContextPt;	// point where context menu was selected
	bool	m_PickEnable;	// true if color picker can update selected points
	double	m_RotPrevPos;	// previous position for rotate gradient
	double	m_HueRotAmt;	// hue rotation amount, normalized from 0..1

// Overrides
	void	UpdateColor();

// Overridables
	virtual	void	UpdateGradient();

// Helpers
	double	CalcPos(CPoint point) const;
	int		SelectPoint(int nFlags, CPoint point);
	void	ShowInterpolatedColor(CPoint point);
	static	void	EnableMenuItem(CMenu& Menu, UINT CmdID, bool Enable); 
};

inline void CGradientDlg::GetGradient(CGradient& Grad) const
{
	m_Grad.GetGradient(Grad);
}

inline int CGradientDlg::GetCount() const
{
	return(m_Grad.GetCount());
}

inline void CGradientDlg::GetSelection(CDWordArray& SelIdx) const
{
	m_Grad.GetSelection(SelIdx);
}

inline void CGradientDlg::SetSelection(const CDWordArray& SelIdx)
{
	m_Grad.SetSelection(SelIdx);
}

inline double CGradientDlg::GetHueRotation() const
{
	return(m_HueRotAmt);
}

inline void CGradientDlg::SetHueRotation(double Rotation)
{
	m_HueRotAmt = Rotation;
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GRADIENTDLG_H__1C517929_3A1F_4497_98AB_0772D83B313A__INCLUDED_)
