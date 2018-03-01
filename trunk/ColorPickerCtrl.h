// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
		00		17oct07	initial version
		01		05mar08	add IsDragging, send notification instead of app msg
		02		24dec08	support disabled state

		color picker control
 
*/

#if !defined(AFX_COLORPICKERCTRL_H__A6CF5251_B069_488C_92B9_B54D9D417F19__INCLUDED_)
#define AFX_COLORPICKERCTRL_H__A6CF5251_B069_488C_92B9_B54D9D417F19__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ColorPickerCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CColorPickerCtrl window

#include "DColor.h"

class CShadePickerCtrl;

class CColorPickerCtrl : public CStatic
{
	DECLARE_DYNAMIC(CColorPickerCtrl);
// Construction
public:
	CColorPickerCtrl();

// Attributes
public:
	void	SetBuddy(CShadePickerCtrl *Buddy);
	CShadePickerCtrl *GetBuddy();
	void	SetColor(const DRGB& Color);
	void	GetColor(DRGB& Color) const;
	void	SetColor(const DHLS& Color);
	void	GetColor(DHLS& Color) const;
	void	SetColor(COLORREF Color);
	COLORREF	GetColor() const;
	bool	IsDragging() const;
	bool	IsMarkVisible() const;

// Operations
public:
	void	UpdateColor();
	void	ShowMark(bool Enable);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CColorPickerCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CColorPickerCtrl();

// Generated message map functions
protected:
	//{{AFX_MSG(CColorPickerCtrl)
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnEnable(BOOL bEnable);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Constants
	enum {
		MARK_SIZE = 8
	};

// Member data
	CShadePickerCtrl	*m_Buddy;	// sibling shade control
	DRGB	m_rgb;			// current color in RGB
	DHLS	m_hls;			// current color in HLS
	CPoint	m_Mark;			// mark position, in client coords
	bool	m_Dragging;		// true if color is being dragged
	bool	m_ShowMark;		// true if mark is visible

// Helpers
	static	void	PtToHLS(const CRect& r, const CPoint& pt, DHLS& hls);
	static	void	HLSToPt(const CRect& r, const DHLS& hls, CPoint& pt);
	void	UpdateBuddy();
	void	UpdateMark();
	void	GetMarkRect(CRect& r);
	void	SetMark(CPoint pt);
	void	NotifyParent();
};

inline void CColorPickerCtrl::SetBuddy(CShadePickerCtrl *Buddy)
{
	m_Buddy = Buddy;
}

inline CShadePickerCtrl *CColorPickerCtrl::GetBuddy()
{
	return(m_Buddy);
}

inline void CColorPickerCtrl::GetColor(DRGB& Color) const
{
	Color = m_rgb;
}

inline void	CColorPickerCtrl::GetColor(DHLS& Color) const
{
	Color = m_hls;
}

inline COLORREF CColorPickerCtrl::GetColor() const
{
	return(m_rgb);
}

inline bool CColorPickerCtrl::IsDragging() const
{
	return(m_Dragging);
}

inline bool CColorPickerCtrl::IsMarkVisible() const
{
	return(m_ShowMark);
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLORPICKERCTRL_H__A6CF5251_B069_488C_92B9_B54D9D417F19__INCLUDED_)
