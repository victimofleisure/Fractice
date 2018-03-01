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
		02		09mar08	don't pass window to DrawMark
		03		24dec08	support disabled state

		shade picker control, used as a buddy for CColorPickerCtrl
 
*/

#if !defined(AFX_SHADEPICKERCTRL_H__041EEB27_9957_4055_98E5_788481C5120E__INCLUDED_)
#define AFX_SHADEPICKERCTRL_H__041EEB27_9957_4055_98E5_788481C5120E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ShadePickerCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CShadePickerCtrl window

#include "DColor.h"

#define CPN_COLORPICK	1000	// color pick notification

class CColorPickerCtrl;

class CShadePickerCtrl : public CStatic
{
	DECLARE_DYNAMIC(CShadePickerCtrl);
// Construction
public:
	CShadePickerCtrl();

// Attributes
public:
	void	SetBuddy(CColorPickerCtrl *Buddy);
	CColorPickerCtrl *GetBuddy();
	void	SetColor(const DHLS& Color);
	void	GetColor(DHLS& Color) const;
	bool	IsDragging() const;
	bool	IsMarkVisible() const;

// Operations
public:
	void	UpdateColor();
	void	DrawMark(bool Show);
	void	DrawMark(bool Show, CDC *pDC);
	void	DragTest(CPoint point);
	void	ShowMark(bool Enable);
	void	UpdateMark();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CShadePickerCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CShadePickerCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CShadePickerCtrl)
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnEnable(BOOL bEnable);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Constants
	enum {
		MARK_SIZE	= 5,
		MARK_OFFSET = 1
	};

// Member data
	CColorPickerCtrl	*m_Buddy;	// sibling color picker
	DHLS	m_hls;			// current color in HLS
	int		m_Mark;			// mark position, in client coords
	bool	m_Dragging;		// true if shade is being dragged
	bool	m_ShowMark;		// true if mark is visible

// Helpers
	static	void	PtToHLS(const CRect& r, const CPoint& pt, DHLS& hls);
	static	void	HLSToPt(const CRect& r, const DHLS& hls, CPoint& pt);
	void	UpdateBuddy();
	void	GetMarkRect(CRect& r);
	void	SetMark(CPoint pt);
	void	NotifyParent();
};

inline void CShadePickerCtrl::SetBuddy(CColorPickerCtrl *Buddy)
{
	m_Buddy = Buddy;
}

inline CColorPickerCtrl *CShadePickerCtrl::GetBuddy()
{
	return(m_Buddy);
}

inline void CShadePickerCtrl::GetColor(DHLS& Color) const
{
	Color = m_hls;
}

inline bool CShadePickerCtrl::IsDragging() const
{
	return(m_Dragging);
}

inline bool CShadePickerCtrl::IsMarkVisible() const
{
	return(m_ShowMark);
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SHADEPICKERCTRL_H__041EEB27_9957_4055_98E5_788481C5120E__INCLUDED_)
