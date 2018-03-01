// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		25jan09	add MoveSelection

        gradient control
 
*/

#if !defined(AFX_GRADIENTCTRL_H__25E0B638_6817_47F1_9218_66D9AE1D28E3__INCLUDED_)
#define AFX_GRADIENTCTRL_H__25E0B638_6817_47F1_9218_66D9AE1D28E3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GradientCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGradientCtrl window

#include "Gradient.h"

#define GCN_GRADIENTCHANGE	1100	// gradient change notification

class CGradientCtrl : public CStatic
{
	DECLARE_DYNAMIC(CGradientCtrl);
// Construction
public:
	CGradientCtrl();

// Attributes
public:
	void	GetGradient(CGradient& Grad) const;
	void	SetGradient(const CGradient& Grad);
	int		GetCount() const;
	void	GetPoint(int PtIdx, CGradientPt& Pt) const;
	void	SetPoint(int PtIdx, const CGradientPt& Pt);
	void	GetColor(int PtIdx, GRADIENT_COLOR& Color) const;
	void	SetColor(int PtIdx, const GRADIENT_COLOR& Color);
	void	SetColor(const GRADIENT_COLOR& Color);
	double	GetPos(int PtIdx) const;
	void	SetPos(int PtIdx, double Pos);
	int		GetColorMode() const;
	void	SetColorMode(int Mode);
	bool	GetSpaceEvenly() const;
	void	SetSpaceEvenly(bool Enable);
	int		GetCurSel() const;
	void	SetCurSel(int PtIdx);
	void	UpdateCurSel(int PtIdx);
	bool	IsSelected(int PtIdx) const;
	int		GetSelCount() const;
	bool	HasSel() const;
	void	GetSelection(CDWordArray& SelIdx) const;
	void	SetSelection(const CDWordArray& SelIdx);
	void	GetMarkerRect(int PtIdx, CRect& r) const;
	void	GetMarkerRect(CRect& r) const;
	bool	MarkersVisible() const;

// Operations
public:
	void	Insert(CGradientPt& pt);
	void	Delete();
	int		HitTest(CPoint pt) const;
	void	InvalidateMarkers(LPCRECT lpRect = NULL, bool Erase = TRUE);
	void	Interpolate(double Pos, DRGB& Color) const;
	void	UpdateGradient();
	void	Select(int PtIdx, bool Enable);
	void	ToggleSelect(int PtIdx);
	void	SelectSpan(int PtIdx);
	void	SelectAll();
	void	Deselect();
	void	SortByPos();
	void	MoveSelection(double Pos);
	void	Invert(int PtIdx);
	void	RotateHue(int PtIdx, double Rotation);
	void	ShowMarkers(bool Enable);
	void	RotateGradient(double Offset);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGradientCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CGradientCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CGradientCtrl)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnEnable(BOOL bEnable);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Types
	class CSortGradPt : public CGradientPt {
	public:
		bool operator>(const CSortGradPt& pt) const;
		bool operator<(const CSortGradPt& pt) const;
		int		m_Idx;		// point's index, for keeping sort stable
		BYTE	m_IsSel;	// true if point is selected
		BYTE	m_IsCurSel;	// true if point is current selection mark
	};
// Constants
	enum {
		MARKER_SIZE = 6,	// size of marker, in client coords
		MARKER_OFFSET = 1	// marker's offset from bottom edge of gradient
	};

// Member data
	CGradient	m_Grad;		// gradient data
	int		m_CurSel;		// index of point from which selection starts
	CByteArray	m_IsSel;	// for each point, true if point is selected
	int		m_SelCount;		// total number of selected points
	bool	m_ShowMarkers;	// true if markers are visible

// Helpers
	void	DumpPts() const;
	void	Select(int PtIdx, bool Enable, CDC *pDC);
	void	Deselect(CDC *pDC);
	void	DrawMarker(int PtIdx, CDC *pDC);
	void	NotifyParent();
};

inline bool CGradientCtrl::CSortGradPt::operator>(const CSortGradPt& pt) const
{
	return(m_Pos != pt.m_Pos ? m_Pos > pt.m_Pos : m_Idx > pt.m_Idx);
}

inline bool CGradientCtrl::CSortGradPt::operator<(const CSortGradPt& pt) const
{
	return(m_Pos != pt.m_Pos ? m_Pos < pt.m_Pos : m_Idx < pt.m_Idx);
}

inline void CGradientCtrl::GetGradient(CGradient& Grad) const
{
	Grad.Copy(m_Grad);
}

inline int CGradientCtrl::GetCount() const
{
	return(m_Grad.GetSize());
}

inline void CGradientCtrl::GetPoint(int PtIdx, CGradientPt& Pt) const
{
	Pt = m_Grad[PtIdx];
}

inline void CGradientCtrl::GetColor(int PtIdx, GRADIENT_COLOR& Color) const
{
	Color = m_Grad[PtIdx].m_Color;
}

inline double CGradientCtrl::GetPos(int PtIdx) const
{
	return(m_Grad[PtIdx].m_Pos);
}

inline int CGradientCtrl::GetColorMode() const
{
	return(m_Grad.m_ColorMode);
}

inline bool CGradientCtrl::GetSpaceEvenly() const
{
	return(m_Grad.m_SpaceEvenly);
}

inline int CGradientCtrl::GetCurSel() const
{
	return(m_CurSel);
}

inline bool CGradientCtrl::IsSelected(int PtIdx) const
{
	return(m_IsSel[PtIdx] != 0);
}

inline int CGradientCtrl::GetSelCount() const
{
	return(m_SelCount);
}

inline bool CGradientCtrl::HasSel() const
{
	return(m_SelCount > 0);
}

inline void CGradientCtrl::GetMarkerRect(CRect& r) const
{
	GetMarkerRect(-1, r);
}

inline bool CGradientCtrl::MarkersVisible() const
{
	return(m_ShowMarkers);
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GRADIENTCTRL_H__25E0B638_6817_47F1_9218_66D9AE1D28E3__INCLUDED_)
