// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      13aug05	initial version
		01		12sep05	add row position
		02		11jul07	remove row dialog tab message; use DS_CONTROL instead
		03		24mar09	add hook for dialog key handler

        generic row form
 
*/

// RowDialogRow.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "RowDialogRow.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRowDialogRow dialog

IMPLEMENT_DYNAMIC(CRowDialogRow, CDialog);

CRowDialogRow::CRowDialogRow(UINT Template, CWnd* pParent /*=NULL*/)
	: CDialog(Template, pParent)
{
	//{{AFX_DATA_INIT(CRowDialogRow)
	//}}AFX_DATA_INIT
	m_NotifyWnd = NULL;
	m_RowIdx = 0;
	m_RowPos = 0;
	m_Accel = NULL;
}

void CRowDialogRow::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRowDialogRow)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CRowDialogRow, CDialog)
	//{{AFX_MSG_MAP(CRowDialogRow)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRowDialogRow message handlers

void CRowDialogRow::OnOK()
{
}

void CRowDialogRow::OnCancel()
{
	m_NotifyWnd->PostMessage(WM_CLOSE);
}

BOOL CRowDialogRow::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST) {
		if (m_Accel != NULL) {
			if (TranslateAccelerator(m_NotifyWnd->m_hWnd, m_Accel, pMsg))
				return(TRUE);
		} else {	// no local accelerator table; try dialog key handler
			if (pMsg->message == WM_KEYDOWN	// for non-system keys only
			&& m_NotifyWnd->SendMessage(UWM_HANDLEDLGKEY, (WPARAM)pMsg))
				return(TRUE);
		}
	}
	// NOTE that the derived row dialog's resource is now assumed to have the
	// DS_CONTROL style, which makes a dialog work well as a child of another
	// dialog, e.g. by integrating the child dialog's tab layout into the tab
	// layout of its parent; in previous versions, the parent handled tabbing
	// explicitly, so we detected the tab key here and send a notification.
	return CDialog::PreTranslateMessage(pMsg);
}
