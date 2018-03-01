// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		01feb09	in OnEndEdit, add color offset
		02		06jan10	W64: in OnMouseWheel, cast SendMessage to bool
		
		parameters view
 
*/

// ParamsView.cpp : implementation file
//

#include "stdafx.h"
#include "Fractice.h"
#include "ParamsView.h"
#include "FormatIO.h"
#include "OptionsDlg.h"
#include "FracticeDoc.h"
#include "FracticeView.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CParamsView

IMPLEMENT_DYNAMIC(CParamsView, CParamsCtrl);

CParamsView::CParamsView()
{
	m_View = NULL;
	m_MainAccel = NULL;
}

CParamsView::~CParamsView()
{
}

void CParamsView::SetView(CFracticeView *View)
{
	if (View == m_View)
		return;	// nothing do to
	m_View = View;
	if (View != NULL)
		Update();
	else
		SetEmpty();
	EnableWindow(View != NULL && !theApp.GetMain()->IsRecording());
}

void CParamsView::Update()
{
	m_View->GetSnapshot(m_Snap);
	CParamsCtrl::Update();
}

void CParamsView::OnBeginEdit()
{
	m_MainAccel = theApp.GetMain()->GetAccelTable();
	// accelerators must be disabled during popup edit, else editing keys have
	// unexpected behavior, because they're also accelerators for history view
	// don't use frame's LoadAccelTable, it asserts if called more than once
	theApp.GetMain()->SetAccelTable(NULL);
	CParamsCtrl::OnBeginEdit();
}

void CParamsView::OnEndEdit(bool Modified)
{
	if (Modified) {
		if (m_ModCount > 1)
			m_LastModIdx = -1;
		switch (m_LastModIdx) {
		case PARM_CYCLE_LEN:
			m_View->SetCycleLength(m_Snap.m_CycleLen);
			break;
		case PARM_COLOR_OFFSET:
			m_View->SetColorOffset(m_Snap.m_ColorOffset);
			break;
		default:
			m_View->SetSnapshot(m_Snap);
			m_View->SetUndoCode(m_UndoCode);
		}
	}
	theApp.GetMain()->SetAccelTable(m_MainAccel);
	CParamsCtrl::OnEndEdit(Modified);
}

BEGIN_MESSAGE_MAP(CParamsView, CParamsCtrl)
	//{{AFX_MSG_MAP(CParamsView)
	ON_WM_MOUSEWHEEL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CParamsView message handlers

BOOL CParamsView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	if (nFlags & MK_CONTROL) {
		const MSG	*pMsg = GetCurrentMessage();
		return BOOL(m_View->SendMessage(pMsg->message, pMsg->wParam, pMsg->lParam));
	}
	return CParamsCtrl::OnMouseWheel(nFlags, zDelta, pt);
}
