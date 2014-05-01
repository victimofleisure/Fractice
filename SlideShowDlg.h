// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14mar09	initial version

        slide show dialog
 
*/

#if !defined(AFX_SLIDESHOWDLG_H__30663ECD_DDE6_4A74_ABBD_160F48F9B607__INCLUDED_)
#define AFX_SLIDESHOWDLG_H__30663ECD_DDE6_4A74_ABBD_160F48F9B607__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SlideShowDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSlideShowDlg dialog

#include "ChildDlg.h"
#include "EditSliderCtrl.h"
#include "IconButton.h"

class CFracticeView;
class CSlideShowMgr;

class CSlideShowDlg : public CChildDlg
{
	DECLARE_DYNAMIC(CSlideShowDlg);
// Construction
public:
	CSlideShowDlg(CWnd* pParent = NULL);

// Attributes
	void	SetView(CFracticeView *View);

// Operations
	void	EnableCtrls(bool Enable);
	void	Update();
	void	UpdateStatus();

// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSlideShowDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CSlideShowDlg)
	enum { IDD = IDD_SLIDE_SHOW };
	CIconButton	m_StopBtn;
	CIconButton	m_PlayBtn;
	CIconButton	m_PauseBtn;
	CIconButton	m_LoopBtn;
	CStatic	m_Status;
	CEditSliderCtrl	m_IntervalSlider;
	CNumEdit	m_IntervalEdit;
	int		m_Order;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CSlideShowDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangedEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnPlay();
	afx_msg void OnPause();
	afx_msg void OnStop();
	afx_msg void OnLoop();
	afx_msg void OnOrder();
	//}}AFX_MSG
	afx_msg LRESULT	OnDlgBarUpdate(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

// Constants
	enum {	// define sliders; must match m_SliderInfo init in .cpp
		SL_INTERVAL,
		SLIDERS
	};
	static const CEditSliderCtrl::INFO m_SliderInfo[SLIDERS];

// Member data
	CFracticeView	*m_View;		// pointer to parent view
	CSlideShowMgr	*m_SlideMgr;	// pointer to slide show manager

// Helpers
	void	SetTransport(int State, bool UpdateMgr = TRUE);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SLIDESHOWDLG_H__30663ECD_DDE6_4A74_ABBD_160F48F9B607__INCLUDED_)
