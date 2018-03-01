// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		
		grid control
 
*/

#if !defined(AFX_GRIDCTRL_H__C29609CB_CEC1_49C4_82A2_88BCCACB7439__INCLUDED_)
#define AFX_GRIDCTRL_H__C29609CB_CEC1_49C4_82A2_88BCCACB7439__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GridCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGridCtrl window

#include "PopupEdit.h"
#include "PopupCombo.h"

class CGridCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(CGridCtrl);
// Construction
public:
	CGridCtrl();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGridCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CGridCtrl();

// Generated message map functions
protected:
	//{{AFX_MSG(CGridCtrl)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Types
	class CMyPopupEdit : public CPopupEdit {
	public:
		CMyPopupEdit(CWnd *Parent, UINT Style, const CRect *Rect);
		void	OnBeginEdit(CString& Text);
		void	OnEndEdit(LPCTSTR Text);
	};
	class CMyPopupCombo : public CPopupCombo {
	public:
		CMyPopupCombo(CWnd *Parent, UINT Style, const CRect *Rect);
		void	OnBeginEdit(CString& Text);
		void	OnEndEdit(LPCTSTR Text);
	};

// Overridables
	virtual	void	OnBeginEdit();
	virtual	bool	OnItemChange(int Row, int Col, LPCTSTR Text);
	virtual	void	OnEndEdit(bool Modified);
	virtual	bool	GetComboStrings(int Row, CStringArray& ComboStr, UINT& Style);

// Data members
	bool	m_ModFlag;	// true if items were modified
	int		m_Row;		// current row
	int		m_Col;		// current column

// Helpers
	int		EditCell(int Row, int Col);
	friend	class CMyPopupEdit;
	friend	class CMyPopupCombo;
};

inline CGridCtrl::CMyPopupEdit::CMyPopupEdit(CWnd *Parent, UINT Style, const CRect *Rect) :
	CPopupEdit(Parent, Style, Rect)
{
}

inline CGridCtrl::CMyPopupCombo::CMyPopupCombo(CWnd *Parent, UINT Style, const CRect *Rect) :
	CPopupCombo(Parent, Style, Rect)
{
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GRIDCTRL_H__C29609CB_CEC1_49C4_82A2_88BCCACB7439__INCLUDED_)
