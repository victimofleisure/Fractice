// Copyleft 2004 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      31jan04	initial version
		01		05mar04	abort marks instance deleted now
		02		04jul04	use parent's font instead of default GUI font
		03		14jan05	add optional rect argument to ctor
		04		29jan05	ctor must initialize m_MaxText
		05		19apr08	add SetTextPtr
		06		22dec08	add style parameter
		07		06jan10	W64: in PreTranslateMessage, cast virtual key to 32-bit

        popup edit control
 
*/

// PopupEdit.cpp : implementation file
//

#include "stdafx.h"
#include "PopupEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPopupEdit

IMPLEMENT_DYNAMIC(CPopupEdit, CEdit);

HHOOK	CPopupEdit::m_MouseHook;
CPopupEdit	*CPopupEdit::m_This;

CPopupEdit::CPopupEdit(CWnd *Parent, UINT Style, const CRect *Rect)
	: m_Parent(Parent), m_Style(Style), m_Rect(Rect)
{
	ASSERT(m_This == NULL);	// only one instance at a time
	m_This = this;
	m_Modify = FALSE;
	m_MaxText = 0;	// no limit
	m_ExitKey = 0;
}

CPopupEdit::~CPopupEdit()
{
	m_This = NULL;
}

void CPopupEdit::SetLimitText(UINT nMax)
{
	m_MaxText = nMax;
}

void CPopupEdit::Abort()
{
	if (m_This != NULL) {
		m_This->EndModalLoop(IDABORT);
		m_This = NULL;	// mark instance deleted; we no longer exist
	}
}

void CPopupEdit::OnBeginEdit(CString& Text)
{
	m_Parent->GetWindowText(Text);
}

void CPopupEdit::OnEndEdit(LPCTSTR Text)
{
	m_Parent->SetWindowText(Text);
}

void CPopupEdit::EndEdit(int retc)
{
	if (retc == IDOK) {
		// set parent text even if we didn't modify it, to force repaint
		CString	Text;
		GetWindowText(Text);
		OnEndEdit(Text);
		m_Modify = CEdit::GetModify() != 0;
	}
	ShowWindow(SW_HIDE);
	EndModalLoop(retc);
}

LRESULT CALLBACK CPopupEdit::MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HC_ACTION) {	// ignore peek
		// if any kind of mouse button down event
		if (wParam == WM_RBUTTONDOWN || wParam == WM_LBUTTONDOWN
		|| wParam == WM_NCLBUTTONDOWN || wParam == WM_NCRBUTTONDOWN) {
			MOUSEHOOKSTRUCT	*mh = (MOUSEHOOKSTRUCT *)lParam;
			if (m_This != NULL) {
				HWND	hWnd = ::WindowFromPoint(mh->pt);
				if (hWnd != m_This->m_hWnd)
					m_This->EndEdit(IDOK);
			}
		}
	}
	return(CallNextHookEx(m_MouseHook, nCode, wParam, lParam));
}

int CPopupEdit::DoModal()
{
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
		if (Create(m_Style, r, m_Parent, 0)) {
			// set font to parent's font
			SetFont(m_Parent->GetFont());
			SetWindowText(Text);
			SetFocus();
			SetSel(0, -1);
			CEdit::SetLimitText(m_MaxText);
			retc = RunModalLoop();
			DestroyWindow();
		}
		UnhookWindowsHookEx(m_MouseHook);
		m_MouseHook = NULL;
	}
    return(retc);
}

BEGIN_MESSAGE_MAP(CPopupEdit, CEdit)
	//{{AFX_MSG_MAP(CPopupEdit)
	ON_WM_KILLFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPopupEdit message handlers

void CPopupEdit::OnKillFocus(CWnd* pNewWnd) 
{
	EndEdit(IDCANCEL);
}

BOOL CPopupEdit::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYDOWN) {
		switch (pMsg->wParam) {
		case VK_ESCAPE:
			EndEdit(IDCANCEL);
			m_ExitKey = INT64TO32(pMsg->wParam);
			return(TRUE);
		case VK_RETURN:
		case VK_TAB:
			EndEdit(IDOK);
			m_ExitKey = INT64TO32(pMsg->wParam);
			return(TRUE);
		}
	}
	return CEdit::PreTranslateMessage(pMsg);
}
