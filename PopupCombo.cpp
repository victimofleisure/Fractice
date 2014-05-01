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

// PopupCombo.cpp : implementation file
//

#include "stdafx.h"
#include "PopupCombo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPopupCombo

IMPLEMENT_DYNAMIC(CPopupCombo, CComboBox);

HHOOK	CPopupCombo::m_MouseHook;
CPopupCombo	*CPopupCombo::m_This;

CPopupCombo::CPopupCombo(CWnd *Parent, UINT Style, const CRect *Rect)
	: m_Parent(Parent), m_Style(Style), m_Rect(Rect)
{
	ASSERT(m_This == NULL);	// only one instance at a time
	m_This = this;
	m_Modify = FALSE;
	m_ExitKey = 0;
	m_OrigSel = -1;
}

CPopupCombo::~CPopupCombo()
{
	m_This = NULL;
}

void CPopupCombo::SetComboStrings(const CStringArray& ComboStr)
{
	m_ComboStr.Copy(ComboStr);
}

void CPopupCombo::Abort()
{
	if (m_This != NULL) {
		m_This->EndModalLoop(IDABORT);
		m_This = NULL;	// mark instance deleted; we no longer exist
	}
}

void CPopupCombo::OnBeginEdit(CString& Text)
{
	m_Parent->GetWindowText(Text);
}

void CPopupCombo::OnEndEdit(LPCTSTR Text)
{
	m_Parent->SetWindowText(Text);
}

void CPopupCombo::EndEdit(int retc)
{
	if (retc == IDOK) {
		// set parent text even if we didn't modify it, to force repaint
		CString	Text;
		int	sel = GetCurSel();
		if (sel >= 0 && sel != m_OrigSel) {
			m_Modify = TRUE;
			GetLBText(sel, Text);
			OnEndEdit(Text);
		}
	}
	ShowWindow(SW_HIDE);
	EndModalLoop(retc);
}

LRESULT CALLBACK CPopupCombo::MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HC_ACTION) {	// ignore peek
		// if any kind of mouse button down event
		if (wParam == WM_RBUTTONDOWN || wParam == WM_LBUTTONDOWN
		|| wParam == WM_NCLBUTTONDOWN || wParam == WM_NCRBUTTONDOWN) {
			MOUSEHOOKSTRUCT	*mh = (MOUSEHOOKSTRUCT *)lParam;
			if (m_This != NULL) {
				HWND	hWnd = ::WindowFromPoint(mh->pt);
				if (hWnd != m_This->m_hWnd) {	// if click outside our window
					if (m_This->GetDroppedState()) {	// if list is down
						CRect	r;
						m_This->GetDroppedControlRect(r);
						if (!r.PtInRect(mh->pt))	// if click within list
							m_This->EndEdit(IDOK);
					} else	// list is up
						m_This->EndEdit(IDOK);
				}
			}
		}
	}
	return(CallNextHookEx(m_MouseHook, nCode, wParam, lParam));
}

int CPopupCombo::DoModal()
{
	static const int	DROPDOWN_HEIGHT = 100;
	int	retc = IDABORT;
	m_MouseHook = SetWindowsHookEx(WH_MOUSE, MouseProc, 
		AfxGetApp()->m_hInstance, AfxGetApp()->m_nThreadID);
	if (m_MouseHook != NULL) {
		CString	Text;
		OnBeginEdit(Text);
		CRect	r;
		if (m_Rect != NULL)	// if ctor was passed a rect, use it
			r = *m_Rect;
		else	// otherwise use parent's client rect
			m_Parent->GetClientRect(r);
		// create the edit control
		r.OffsetRect(1, -2);	// fudge to line up with grid control
		r.bottom += DROPDOWN_HEIGHT;	// expand to include drop list
		if (Create(m_Style, r, m_Parent, 0)) {
			PostMessage(CB_SHOWDROPDOWN, TRUE);	// show drop list
			// set font to parent's font
			SetFont(m_Parent->GetFont());
			for (int i = 0; i < m_ComboStr.GetSize(); i++)
				AddString(m_ComboStr[i]);
			m_OrigSel = SelectString(-1, Text);
			SetFocus();
			retc = RunModalLoop();
			DestroyWindow();
		}
		UnhookWindowsHookEx(m_MouseHook);
		m_MouseHook = NULL;
	}
    return(retc);
}

BEGIN_MESSAGE_MAP(CPopupCombo, CComboBox)
	//{{AFX_MSG_MAP(CPopupCombo)
	ON_CONTROL_REFLECT(CBN_SELCHANGE, OnSelchange)
	ON_CONTROL_REFLECT(CBN_KILLFOCUS, OnKillfocus)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPopupCombo message handlers

void CPopupCombo::OnSelchange() 
{
	EndEdit(IDOK);
}

void CPopupCombo::OnKillfocus() 
{
	EndEdit(IDOK);
}
