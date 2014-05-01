// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version

        popup combo box control
 
*/

#if !defined(AFX_POPUPCOMBO_H__D540148F_907B_4CC5_B189_2F75F1269E75__INCLUDED_)
#define AFX_POPUPCOMBO_H__D540148F_907B_4CC5_B189_2F75F1269E75__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// PopupCombo.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPopupCombo window

class CPopupCombo : public CComboBox
{
	DECLARE_DYNAMIC(CPopupCombo);
public:
// Constants
	enum {
		DEF_STYLE = WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST
	};

// Construction
	CPopupCombo(CWnd *Parent, UINT Style = DEF_STYLE, const CRect *Rect = NULL);
	virtual ~CPopupCombo();

// Attributes
	static	bool	Exists();
	void	SetComboStrings(const CStringArray& ComboStr);
	CWnd	*GetParent() const;
	bool	GetModify() const;
	int		GetExitKey() const;

// Operations
	int		DoModal();
	static	void	Abort();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPopupCombo)
	//}}AFX_VIRTUAL

// Implementation
protected:
// Generated message map functions
	//{{AFX_MSG(CPopupCombo)
	afx_msg void OnSelchange();
	afx_msg void OnKillfocus();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Data members
	static	HHOOK	m_MouseHook;	// handle to mouse hook function
	static	CPopupCombo	*m_This;	// pointer to our one and only instance
	CWnd	*m_Parent;		// pointer to parent window
	const	CRect	*m_Rect;	// our dimensions, or NULL for same as parent
	UINT	m_Style;		// combo box style bitmask
	bool	m_Modify;		// true if combo selection was modified
	int		m_ExitKey;		// if non-zero, virtual key that ended modal loop
	int		m_OrigSel;		// original selection for detecting changes
	CStringArray	m_ComboStr;	// array of combo box strings

// Overridables
	virtual	void	OnBeginEdit(CString& Text);
	virtual	void	OnEndEdit(LPCTSTR Text);

// Helpers
	void	EndEdit(int retc);

// Friends
	static	LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam);
};

inline bool CPopupCombo::Exists()
{
	return(m_This != NULL);
}

inline CWnd *CPopupCombo::GetParent() const
{
	return(m_Parent);
}

inline bool CPopupCombo::GetModify() const
{
	return(m_Modify);
}

inline int CPopupCombo::GetExitKey() const
{
	return(m_ExitKey);
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_POPUPCOMBO_H__D540148F_907B_4CC5_B189_2F75F1269E75__INCLUDED_)
