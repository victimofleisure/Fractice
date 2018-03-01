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

// GridCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "GridCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGridCtrl

IMPLEMENT_DYNAMIC(CGridCtrl, CListCtrl);

CGridCtrl::CGridCtrl()
{
	m_ModFlag = FALSE;
	m_Row = 0;
	m_Col = 0;
}

CGridCtrl::~CGridCtrl()
{
}

void CGridCtrl::CMyPopupEdit::OnBeginEdit(CString& Text)
{
	CGridCtrl	*gc = (CGridCtrl *)GetParent();
	Text = gc->GetItemText(gc->m_Row, gc->m_Col);
}

void CGridCtrl::CMyPopupEdit::OnEndEdit(LPCTSTR Text)
{
	CGridCtrl	*gc = (CGridCtrl *)GetParent();
	gc->SetItemText(gc->m_Row, gc->m_Col, Text);
}

void CGridCtrl::CMyPopupCombo::OnBeginEdit(CString& Text)
{
	CGridCtrl	*gc = (CGridCtrl *)GetParent();
	Text = gc->GetItemText(gc->m_Row, gc->m_Col);
}

void CGridCtrl::CMyPopupCombo::OnEndEdit(LPCTSTR Text)
{
	CGridCtrl	*gc = (CGridCtrl *)GetParent();
	gc->SetItemText(gc->m_Row, gc->m_Col, Text);
}

int CGridCtrl::EditCell(int Row, int Col)
{
	EnsureVisible(Row, TRUE);
	CRect	r;
	GetSubItemRect(Row, Col, LVIR_BOUNDS, r);
	CString	s(GetItemText(Row, Col));
	CString	OrigText(s);
	int	ExitKey;
	bool	Modified;
	m_Row = Row;
	m_Col = Col;
	//
	// DO NOT access non-static members if CPopupEdit::Exists returns FALSE.
	// If our parent window is closed while we're inside CPopupEdit::DoModal,
	// our instance will be destroyed BEFORE we regain control; incredible, but
	// true.  DoModal runs the windows message loop while it waits, so whatever 
	// message handler destroyed our parent and its children (including us) was
	// called from DoModal and MUST finish executing before DoModal can return.
	//
	while (1) {
		CStringArray	ComboStr;
		UINT	ComboStyle = CPopupCombo::DEF_STYLE;
		if (GetComboStrings(Row, ComboStr, ComboStyle)) {	// if combo box
			CMyPopupCombo	pc(this, ComboStyle, &r);
			pc.SetComboStrings(ComboStr);
			pc.DoModal();
			if (!CMyPopupCombo::Exists())	// if popup combo is destroyed
				return(-1);	// assume app is closed and our instance is invalid
			ExitKey = pc.GetExitKey();
			Modified = pc.GetModify();
		} else {	// edit control
			CMyPopupEdit	pe(this, CPopupEdit::DEF_STYLE, &r);
			pe.DoModal();
			if (!CMyPopupEdit::Exists())	// if popup edit is destroyed
				return(-1);	// assume app is closed and our instance is invalid
			ExitKey = pe.GetExitKey();
			Modified = pe.GetModify();
		}
		if (Modified && ExitKey != VK_ESCAPE) {
			s = GetItemText(Row, Col);
			if (OnItemChange(Row, Col, s)) {
				m_ModFlag = TRUE;
				break;
			}
		} else {
			if (s != OrigText)
				SetItemText(Row, Col, OrigText);
			break;
		}
	}
	return(ExitKey);
}

void CGridCtrl::OnBeginEdit()
{
}

bool CGridCtrl::OnItemChange(int Row, int Col, LPCTSTR Text)
{
	return(TRUE);
}

void CGridCtrl::OnEndEdit(bool Modified)
{
}

bool CGridCtrl::GetComboStrings(int Row, CStringArray& ComboStr, UINT& Style)
{
	return(FALSE);
}

BEGIN_MESSAGE_MAP(CGridCtrl, CListCtrl)
	//{{AFX_MSG_MAP(CGridCtrl)
	ON_WM_LBUTTONDOWN()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGridCtrl message handlers

void CGridCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if (CPopupEdit::Exists() || CPopupCombo::Exists()) {
		// assume we're in the popup edit control's modal loop; we may want to
		// create a popup edit instance, but only one popup edit can exist at
		// a time, so the handler that created the current instance needs to
		// exit first, which we allow by deferring this button down message
		PostMessage(WM_LBUTTONDOWN, nFlags, MAKELONG(point.x, point.y));
		return;
	}
	LVHITTESTINFO hti;
	hti.pt = point;
	ListView_SubItemHitTest(m_hWnd, &hti);
	int	row = hti.iItem;
	int	col = hti.iSubItem;
	if (row >= 0 && col > 0) {
		m_ModFlag = FALSE;
		OnBeginEdit();
		int	key = EditCell(row, col);
		while (key == VK_TAB) {
			if (GetKeyState(VK_SHIFT) & GKS_DOWN) {
				row--;
				if (row < 0)
					row = GetItemCount() - 1;
			} else {
				row++;
				if (row >= GetItemCount())
					row = 0;
			}
			key = EditCell(row, col);
		} 
		if (key < 0)	// if app closed from within popup edit modal loop
			return;	// our instance is gone, so don't access any members
		OnEndEdit(m_ModFlag);
	} else
		CListCtrl::OnLButtonDown(nFlags, point);
}

void CGridCtrl::OnDestroy() 
{
	// closing app while we're stuck in a modal loop causes crash
	CPopupEdit::Abort();	// abort modal loop and destroy edit instance
	CPopupCombo::Abort();	// abort modal loop and destroy combo instance
	CListCtrl::OnDestroy();
}
