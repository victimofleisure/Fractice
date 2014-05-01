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

// ChildFrm.cpp : implementation of the CChildFrame class
//

#include "stdafx.h"
#include "Fractice.h"

#include "ChildFrm.h"
#include "MainFrm.h"
#include "FracticeDoc.h"
#include "FracticeView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define RK_CHILD_FRAME	_T("ChildFrame")

/////////////////////////////////////////////////////////////////////////////
// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(CChildFrame)
	ON_WM_MDIACTIVATE()
	ON_WM_GETMINMAXINFO()
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_WM_NCPAINT()
	ON_WM_CREATE()
	ON_WM_NCLBUTTONDOWN()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_SETMESSAGESTRING, OnSetMessageString)
	ON_MESSAGE(WM_ENTERSIZEMOVE, OnEnterSizeMove)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChildFrame construction/destruction

CChildFrame::CChildFrame()
{
	m_Main = NULL;
	m_View = NULL;
	m_IsFullScreen = FALSE;
	m_FullScreenRect.SetRectEmpty();
	m_ShowCmd = 0;
	m_IsNCMoving = FALSE;
}

CChildFrame::~CChildFrame()
{
}

void CChildFrame::SetFullScreenRect(CRect *pRect)
{
	if (pRect != NULL) {
		m_FullScreenRect = *pRect;
		ScreenToClient(m_FullScreenRect);
		CalcWindowRect(m_FullScreenRect);
	} else
		m_FullScreenRect.SetRectEmpty();
}

void CChildFrame::FullScreen(bool Enable)
{
	if (Enable == m_IsFullScreen)
		return;	// nothing to do
	m_IsFullScreen = Enable;
	if (Enable) {
		WINDOWPLACEMENT	wp;
		GetWindowPlacement(&wp);
		m_ShowCmd = wp.showCmd;
		ShowWindow(SW_SHOWMAXIMIZED);
	} else
		ShowWindow(m_ShowCmd);
}

BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// override default window class styles CS_HREDRAW and CS_VREDRAW
	// otherwise resizing frame redraws entire view, causing flicker
	cs.lpszClass = AfxRegisterWndClass(	// create our own window class
		CS_DBLCLKS,						// request double-clicks
		theApp.LoadStandardCursor(IDC_ARROW),	// standard cursor
		NULL,									// no background brush
		theApp.LoadIcon(IDR_MAINFRAME));		// app's icon
	ASSERT(cs.lpszClass);
	return CMDIChildWnd::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CChildFrame diagnostics

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CChildFrame message handlers

int CChildFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	m_Main = theApp.GetMain();
	ASSERT(m_Main != NULL);
	return 0;
}

void CChildFrame::OnDestroy() 
{
	CMDIChildWnd::OnDestroy();
	// save maximize setting in registry
	CPersist::SaveWnd(REG_SETTINGS, this, RK_CHILD_FRAME);
	m_Main->GetMixerDlg().RemoveSource(m_View);
}

void CChildFrame::OnClose() 
{
	if (m_Main->IsRecording()) {
		CRecorder&	Recorder = m_Main->GetRecorder();
		if (Recorder.GetView() == m_View && !Recorder.StopCheck())
			return;
	}
	CMDIChildWnd::OnClose();
}

void CChildFrame::ActivateFrame(int nCmdShow) 
{
	if (GetMDIFrame()->MDIGetActive())
		CMDIChildWnd::ActivateFrame(nCmdShow); 
	else {
		int	RegShow = CPersist::GetWndShow(REG_SETTINGS, RK_CHILD_FRAME);
		CMDIChildWnd::ActivateFrame(RegShow == SW_SHOWMAXIMIZED ? RegShow : nCmdShow);
	}
}

void CChildFrame::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd) 
{
	CMDIChildWnd::OnMDIActivate(bActivate, pActivateWnd, pDeactivateWnd);
	if (bActivate) {
		if (m_View == NULL) {
			m_View = DYNAMIC_DOWNCAST(CFracticeView, GetActiveView());
			ASSERT(m_View != NULL);
			m_Main->GetMixerDlg().AddSource(m_View);
		}
		m_Main->SetView(m_View);
	} else {	// deactivating
		if (pActivateWnd == NULL)	// if no document
			m_Main->SetView(NULL);
	}
}

void CChildFrame::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI) 
{
	if (!m_FullScreenRect.IsRectNull()) {
		lpMMI->ptMaxPosition = m_FullScreenRect.TopLeft();
		lpMMI->ptMaxTrackSize = CPoint(m_FullScreenRect.Size());
	}
	CMDIChildWnd::OnGetMinMaxInfo(lpMMI);
}

LRESULT CChildFrame::OnSetMessageString(WPARAM wParam, LPARAM lParam)
{
	switch (wParam) {
	case 0:	// menu separator
		lParam = LPARAM("");	// empty message
		break;
	case AFX_IDS_IDLEMESSAGE:	// display main's status message for these IDs
		wParam = 0;
		lParam = LPARAM(m_Main->GetStatusMsg());
		break;
	case AFX_IDW_PANE_FIRST:	// 0xE900 from control bar; ignore it
		return FALSE;
	}
	return CMDIChildWnd::OnSetMessageString(wParam, lParam);
}

void CChildFrame::OnSize(UINT nType, int cx, int cy) 
{
	// Creating or activating a MDI child window while the app is maximized
	// causes flicker, because Windows restores the deactivated child window
	// before maximizing the new window on top of it. The flicker is visible
	// in XP but not in W2K, and occurs when selecting docs via the Windows
	// menu but not with Ctrl+Tab. The following workaround is adapted from
	// Andrzej Markowski's "A flicker issue in MDI applications."
	CMDIFrameWnd	*pParentFrame = (CMDIFrameWnd*)GetParentFrame();
	if (pParentFrame) {
		BOOL	bMax;
		CMDIChildWnd	*pChildFrame = pParentFrame->MDIGetActive(&bMax);
		if (bMax) {
			if (!(pChildFrame == this 
			&& (nType == SIZE_MAXIMIZED || nType == SIZE_RESTORED))) {
				SetRedraw(FALSE);
				CMDIChildWnd::OnSize(nType, cx, cy);
				SetRedraw(TRUE);
				return;
			}
		}
	}
	CMDIChildWnd::OnSize(nType, cx, cy);
}

void CChildFrame::OnNcPaint()
{
	// part of MDI flicker workaround; see comment above in OnSize
	CMDIFrameWnd	*pParentFrame = (CMDIFrameWnd*)GetParentFrame();
	if (pParentFrame) {
		BOOL	bMax;
		pParentFrame->MDIGetActive(&bMax);
		if (bMax)
			return;
	}
	CMDIChildWnd::OnNcPaint();
}

void CChildFrame::OnNcLButtonDown(UINT nHitTest, CPoint point)
{
	if (m_Main->IsExclusive()) {	// if we're timer-driven
		// default handler blocks message loop, which would freeze display,
		// so we must emulate default handling; see CSizingDockFrame.cpp
		switch (nHitTest) {
		case HTCAPTION:
			m_NCLBDownPos = point;
			m_IsNCMoving = TRUE;
			SetFocus();	// tried MDIActivate but it doesn't always work here
			ShowCursor(FALSE);	// hide cursor, else jump is briefly visible
			SendMessage(WM_SYSCOMMAND, SC_MOVE);
			break;
		case HTCLOSE:
			SendMessage(WM_SYSCOMMAND, SC_CLOSE);
			break;
		case HTMAXBUTTON:
			ShowWindow(IsZoomed() ? SW_RESTORE : SW_MAXIMIZE);
			break;
		case HTMINBUTTON:
			ShowWindow(IsIconic() ? SW_RESTORE : SW_MINIMIZE);
			break;
		default:
			CMDIChildWnd::OnNcLButtonDown(nHitTest, point);
		}
	} else
		CMDIChildWnd::OnNcLButtonDown(nHitTest, point);
}

LRESULT CChildFrame::OnEnterSizeMove(WPARAM wParam, LPARAM lParam)
{
	if (m_IsNCMoving) {	// if move was initiated by left-click in caption bar
		SetCursorPos(m_NCLBDownPos.x, m_NCLBDownPos.y);	// from OnNcLButtonDown
		ShowCursor(TRUE);
		m_IsNCMoving = FALSE;
	}
	return(0);
}
