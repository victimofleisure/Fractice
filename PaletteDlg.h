// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version

        palette dialog
 
*/

#if !defined(AFX_PALETTEDLG_H__1C517929_3A1F_4497_98AB_0772D83B313A__INCLUDED_)
#define AFX_PALETTEDLG_H__1C517929_3A1F_4497_98AB_0772D83B313A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PaletteDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPaletteDlg dialog

#include "GradientDlg.h"
#include "Palette.h"
#include "CtrlResize.h"

class CMainFrame;
class CFracticeView;

class CPaletteDlg : public CGradientDlg
{
	DECLARE_DYNAMIC(CPaletteDlg);
// Construction
public:
	CPaletteDlg(CWnd* pParent = NULL);   // standard constructor
	~CPaletteDlg();

// Attributes
	bool	IsPreviewing() const;
	bool	IsModified() const;
	void	GetCtrls(DPalette& Palette);
	void	SetCtrls(const DPalette& Palette);
	void	SetPaletteName(const CString& Name);
	
// Operations
	void	InitCtrls();
	void	ResetCtrls();
	void	EnableCtrls(bool Enable);
	void	SetView(CFracticeView *View);
	void	UpdateView();
	void	SetBkColor(COLORREF Color);
	void	SetModify(bool Enable);
	static	bool	ChooseFile(CString& Path, bool& Preview, CWnd *NotifyWnd, bool Import = FALSE);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPaletteDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CPaletteDlg)
	enum { IDD = IDD_PALETTE };
	CButton	m_ApplyBtn;
	CButton	m_PreviewChk;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CPaletteDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnBkColor();
	afx_msg void OnPreview();
	virtual void OnCancel();
	afx_msg void OnDestroy();
    afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnApplyNow();
	afx_msg void OnEditName();
	//}}AFX_MSG
	afx_msg LRESULT	OnEnableCtrls(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

// Types
	class CBkColorDlg : public CColorPickerDlg {
	public:
		void	UpdateColor();
		BOOL	OnInitDialog();
		CPaletteDlg	*m_PalDlg;
	};
	
// Overrides
	void	UpdateColor();
	void	UpdateGradient();

// Member data
	CMainFrame	*m_Main;	// pointer to main frame
	CFracticeView	*m_View;	// pointer to current view
	DPalette	*m_Palette;	// pointer to current view's palette
	CRect	m_InitRect;		// initial dimensions
	CCtrlResize	m_Resize;	// control resizer
	COLORREF	m_BkColor;	// background color
	bool	m_WasShown;		// true if dialog was shown at least once
	bool	m_Preview;		// true if previewing palette changes
	bool	m_Modified;		// true if there are changes to apply
	bool	m_CtrlsEnabled;	// true if child controls are enabled
	CString	m_PaletteName;	// stores palette name during editing
	CString	m_OrigCaption;	// copy of dialog's original caption

// Helpers
	static	int	FindMenuItem(const CMenu& menu, UINT ItemID);
};

inline bool CPaletteDlg::IsPreviewing() const
{
	return(m_Preview);
}

inline bool CPaletteDlg::IsModified() const
{
	return(m_Modified);
}

inline void CPaletteDlg::SetBkColor(COLORREF Color)
{
	m_BkColor = Color;
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PALETTEDLG_H__1C517929_3A1F_4497_98AB_0772D83B313A__INCLUDED_)
