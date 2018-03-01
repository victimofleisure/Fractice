// Copyleft 2004 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      31jan04	initial version
		01		19apr08	add SetTextPtr
		02		22dec08	add style parameter

        popup edit control
 
*/

#if !defined(AFX_POPUPEDIT_H__D540148F_907B_4CC5_B189_2F75F1269E75__INCLUDED_)
#define AFX_POPUPEDIT_H__D540148F_907B_4CC5_B189_2F75F1269E75__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// PopupEdit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPopupEdit window

class CPopupEdit : public CEdit
{
	DECLARE_DYNAMIC(CPopupEdit);
public:
// Constants
	enum {
		DEF_STYLE = WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL
	};

// Construction
	CPopupEdit(CWnd *Parent, UINT Style = DEF_STYLE, const CRect *Rect = NULL);
	virtual ~CPopupEdit();

// Attributes
	static	bool	Exists();
	CWnd	*GetParent() const;
	bool	GetModify() const;
	void	SetLimitText(UINT nMax);
	int		GetExitKey() const;

// Operations
	int		DoModal();
	static	void	Abort();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPopupEdit)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:
// Generated message map functions
	//{{AFX_MSG(CPopupEdit)
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Data members
	static	HHOOK	m_MouseHook;	// handle to mouse hook function
	static	CPopupEdit	*m_This;	// pointer to our one and only instance
	CWnd	*m_Parent;		// pointer to parent window
	const	CRect	*m_Rect;	// our dimensions, or NULL for same as parent
	UINT	m_Style;		// edit control style bitmask
	bool	m_Modify;		// true if text was modified
	UINT	m_MaxText;		// how much text can be entered, in chars
	int		m_ExitKey;		// if non-zero, virtual key that ended modal loop

// Overridables
	virtual	void	OnBeginEdit(CString& Text);
	virtual	void	OnEndEdit(LPCTSTR Text);

// Helpers
	void	EndEdit(int retc);

// Friends
	static	LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam);
};

inline bool CPopupEdit::Exists()
{
	return(m_This != NULL);
}

inline CWnd *CPopupEdit::GetParent() const
{
	return(m_Parent);
}

inline bool CPopupEdit::GetModify() const
{
	return(m_Modify);
}

inline int CPopupEdit::GetExitKey() const
{
	return(m_ExitKey);
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_POPUPEDIT_H__D540148F_907B_4CC5_B189_2F75F1269E75__INCLUDED_)
