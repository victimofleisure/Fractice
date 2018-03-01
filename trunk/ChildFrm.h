// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		02mar09	override non-client handling in exclusive mode 

        MDI child frame
 
*/

// ChildFrm.h : interface of the CChildFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHILDFRM_H__497E0524_A479_4E58_955A_F90A7F0CD5B4__INCLUDED_)
#define AFX_CHILDFRM_H__497E0524_A479_4E58_955A_F90A7F0CD5B4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CFracticeView;
class CMainFrame;

class CChildFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(CChildFrame)
public:
	CChildFrame();

// Attributes
public:
	void	SetFullScreenRect(CRect *pRect);
	bool	IsFullScreen() const;
	void	FullScreen(bool Enable);

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChildFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void ActivateFrame(int nCmdShow = -1);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CChildFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	//{{AFX_MSG(CChildFrame)
	afx_msg void OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnDestroy();
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNcPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	//}}AFX_MSG
	afx_msg LRESULT OnSetMessageString(WPARAM wParam, LPARAM lParam);
	LRESULT OnEnterSizeMove(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

// Data members
	CFracticeView	*m_View;	// pointer to our view
	CMainFrame	*m_Main;		// pointer to main frame
	bool	m_IsFullScreen;		// true if in full-screen mode
	CRect	m_FullScreenRect;	// full-screen rectangle in client coords
	int		m_ShowCmd;			// in full-screen mode, previous show state
	bool	m_IsNCMoving;		// if true, we're being moved by non-client drag
	CPoint	m_NCLBDownPos;		// cursor position at non-client left button down
};

inline bool CChildFrame::IsFullScreen() const
{
	return(m_IsFullScreen);
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHILDFRM_H__497E0524_A479_4E58_955A_F90A7F0CD5B4__INCLUDED_)
