// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
        01      10jan09	in OnRecord, enable/disable palette dialog
		02		24jan09	allow palette files to be dropped
		03		02feb09	add color mapping bar
		04		06feb09	add color cycling bar
		05		10feb09	in HideChrome, don't delay showing bars
		06		12feb09	if network adapter changes, restart listener
		07		15feb09	add Exclusive mode
		08		23feb09	add non-blocking mode
		09		24feb09	add files bar
		10		03mar09	add MIDI
		11		11mar09	move OnImgCancel here
		12		14mar09	add slide show bar
		13		16mar09	add demo
		14		18mar09	rename toggle channel back to select channel
		15		21mar09	in OnSysCommand, mask off unreliable low nibble
		16		21mar09	make sizing bar dock styles persistent
		17		22mar09	handle initially maximized case explicitly
		18		23mar09	support channel MIDI properties
		19		25mar09	in Exclusive, eliminate maximized flicker
		20		28mar09	in OnEditOptions, don't restore MIDI defaults
		21		29mar09	in SetMidiProperty, clamp damping and origin motion
		22		31mar09	add OnDisplayChange
		23		05jul09	allow safe accelerators in record
		24		16dec09	in SetView, call servers dialog's SetView
		25		06jan10	W64: cast message params to 32-bit
		26		06jan10	W64: cast RegDockStyle array size to 32-bit

        main frame
 
*/

// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "Fractice.h"
#include "MainFrm.h"
#include "ChildFrm.h"

#include "FracticeDoc.h"
#include "FracticeView.h"
#include "afxpriv.h"	// needed for VerifyDockState
#include "RecordDlg.h"
#include "ProgressDlg.h"
#include "PosterProgressDlg.h"
#include "PathStr.h"
#include "SizingDockFrame.h"
#include "MidiInfo.h"
#include "RenderingDlg.h"
#include "MessageBoxCheck.h"
#include "MsgBoxDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CMDIFrameWnd)

#define RK_MAIN_FRAME		_T("MainFrame")
#define RK_SHOW_TOOL_DLG	_T("ShowToolDlg")
#define RK_CUR_TOOL			_T("CurTool")
#define RK_BAR_DOCK_STYLE	_T("SizingBarDockStyle")

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_FRAME_SIZE,
	ID_INDICATOR_QUALITY,
	ID_INDICATOR_ANTIALIAS,
};

// this list must match the sizing control bars enum
const CMainFrame::SIZING_BAR_INFO CMainFrame::m_SizingBarInfo[SIZING_BARS] = {
//	BarIdx				TitleID					DockStyle		DockBarID				InitShow	InitFloat
	{CBI_HISTORY,		IDS_CBT_HISTORY,		CBRS_ALIGN_ANY,	AFX_IDW_DOCKBAR_RIGHT,	TRUE,		{0, 0}},
	{CBI_LEVELS,		IDS_CBT_LEVELS,			CBRS_ALIGN_ANY,	AFX_IDW_DOCKBAR_BOTTOM,	FALSE,		{0, 0}},
	{CBI_PARAMETERS,	IDS_CBT_PARAMETERS,		CBRS_ALIGN_ANY,	AFX_IDW_DOCKBAR_LEFT,	FALSE,		{0, 0}},
	{CBI_PALETTE,		IDS_CBT_PALETTE,		0,				AFX_IDW_DOCKBAR_BOTTOM,	FALSE,		{0, 0}},
	{CBI_COLOR_MAPPING,	IDS_CBT_COLOR_MAPPING,	0,				AFX_IDW_DOCKBAR_LEFT,	FALSE,		{425, 0}},
	{CBI_COLOR_CYCLING,	IDS_CBT_COLOR_CYCLING,	0,				AFX_IDW_DOCKBAR_LEFT,	FALSE,		{425, 130}},
	{CBI_FILES,			IDS_CBT_FILES,			CBRS_ALIGN_ANY,	AFX_IDW_DOCKBAR_LEFT,	FALSE,		{0, 0}},
	{CBI_MIXER,			IDS_CBT_MIXER,			0,				AFX_IDW_DOCKBAR_LEFT,	FALSE,		{425, 225}},
	{CBI_SLIDE_SHOW,	IDS_CBT_SLIDE_SHOW,		0,				AFX_IDW_DOCKBAR_LEFT,	FALSE,		{0, 260}},
};

#define MAKEBARID(idx) (AFX_IDW_CONTROLBAR_FIRST + 32 + idx)

const CMainFrame::ACCEL_EDIT CMainFrame::m_RecordAccelEdit[] = {
	{TRUE,	ID_FILE_RECORD},
	{TRUE,	ID_IMG_FIT_IN_WINDOW},
	{TRUE,	ID_VIEW_COLOR_CYCLING, ID_VIEW_SERVERS},	// enable all view commands
	{TRUE,	ID_HELP_INDEX},
	{0}	// list terminator
};

const CMainFrame::ACCEL_EDIT CMainFrame::m_SMExclAccelEdit[] = {
	{TRUE,	ID_EDIT_UNDO},
	{TRUE,	ID_EDIT_REDO},
	{TRUE,	ID_IMG_ANTIALIAS_0, ID_IMG_ZOOM_OUT},	// enable all image commands
	{FALSE,	ID_IMG_SLIDE_SHOW},	// except slide show
	{TRUE,	ID_WND_EXCLUSIVE},
	{0}	// list terminator
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame() :
	m_OptsDlg(IDS_OPTIONS),
	m_MRUPalette(0, _T("MRUPalette"), _T("%d"), m_OptsDlg.GetMaxMRUPalettes()),
	m_MRUImgSize(0, _T("MRUImgSize"), _T("%d"), m_OptsDlg.GetMaxMRUParams()),
	m_MRUQuality(0, _T("MRUQuality"), _T("%d"), m_OptsDlg.GetMaxMRUParams()),
	m_MRUAntialias(0, _T("MRUAntialias"), _T("%d"), m_OptsDlg.GetMaxMRUParams()),
	m_MixerDlg(IDS_CBT_MIXER)
{
	m_View = NULL;
	m_WasShown = FALSE;
	m_HaveWndPlace = FALSE;
	m_DocMenu.LoadMenu(IDR_FRACTICETYPE);
	m_RecMenu.LoadMenu(IDR_RECORDING);
	m_ChildFrameMenu = m_DocMenu.m_hMenu;
	// these inits must match the tool dialogs enum
	m_ToolDlg[TD_RECORD]		= &m_RecordDlg;
	m_ToolDlg[TD_RECORD_STATUS]	= &m_Recorder.GetRecordStatusDlg();
	m_ToolDlg[TD_SERVERS]		= &m_Recorder.GetServersDlg();
	m_ToolDlg[TD_JOB_CONTROL]	= &m_Recorder.GetJobControlDlg();
	m_ToolDlg[TD_MIDI_SETUP]	= &m_MidiSetupDlg;
	// these inits must match the sizing control bars enum
	m_SizingBar[SCB_HISTORY]		= &m_HistoryBar;
	m_SizingBar[SCB_LEVELS]			= &m_LevelsBar;
	m_SizingBar[SCB_PARAMETERS]		= &m_ParamsBar;
	m_SizingBar[SCB_PALETTE]		= &m_PaletteBar;
	m_SizingBar[SCB_COLOR_MAPPING]	= &m_ColorMappingBar;
	m_SizingBar[SCB_COLOR_CYCLING]	= &m_ColorCyclingBar;
	m_SizingBar[SCB_FILES]			= &m_FilesBar;
	m_SizingBar[SCB_MIXER]			= &m_MixerBar;
	m_SizingBar[SCB_SLIDE_SHOW]		= &m_SlideShowBar;
	// attach dialogs to their host dialog bars
	m_PaletteBar.SetDlg(&m_PaletteDlg, IDD_PALETTE);
	m_ColorMappingBar.SetDlg(&m_ColorMappingDlg, IDD_COLOR_MAPPING);
	m_ColorCyclingBar.SetDlg(&m_ColorCyclingDlg, IDD_COLOR_CYCLING);
	m_MixerBar.SetDlg(&m_MixerDlg);
	m_SlideShowBar.SetDlg(&m_SlideShowDlg, IDD_SLIDE_SHOW);
	m_PreFullScrnRect.SetRectEmpty();
	m_FullScreenRect.SetRectEmpty();
	m_ExclusiveRect.SetRectEmpty();
	m_KeybdHook = NULL;
	m_ShowToolBar = FALSE;
	m_ShowStatusBar = FALSE;
	ZeroMemory(m_ShowSizingBar, sizeof(m_ShowSizingBar));
	m_ShowToolDlg = theApp.RdRegInt(RK_SHOW_TOOL_DLG, 0);
	m_HideChrome = FALSE;
	m_IsFullScreen = FALSE;
	m_CurTool = theApp.RdRegInt(RK_CUR_TOOL, CFracticeView::TOOL_MARQUEE);
	m_PrevMainAccel = NULL;
	m_FullScrnDualMon = TRUE;
	m_MRUPalette.ReadList();
	m_MRUImgSize.ReadList();
	m_MRUQuality.ReadList();
	m_MRUAntialias.ReadList();
	m_MidiIO.SetInputCallback(MidiCallback, this);
	m_MidiDev = CMidiIO::NO_DEVICE;
	ZeroMemory(&m_NoteToggle, sizeof(m_NoteToggle));
	m_DemoMode = FALSE;
	m_PrevFitInWindow = FALSE;
}

CMainFrame::~CMainFrame()
{
	theApp.WrRegInt(RK_SHOW_TOOL_DLG, m_ShowToolDlg);
	theApp.WrRegInt(RK_CUR_TOOL, m_CurTool);
	m_MRUPalette.WriteList();
	m_MRUImgSize.WriteList();
	m_MRUQuality.WriteList();
	m_MRUAntialias.WriteList();
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if (CPersist::GetWndPlacement(REG_SETTINGS, RK_MAIN_FRAME, m_WndPlace)) {
		CRect	r;
		// if maximizing, we oversize initial window, which helps us restore
		// docked sizing bars to their original sizes; see also OnShowWindow
		if (m_WndPlace.showCmd == SW_MAXIMIZE) {
			r = CRect(0, 0, GetSystemMetrics(SM_CXSCREEN), 
				GetSystemMetrics(SM_CYSCREEN));	// assume primary monitor
		} else
			r = m_WndPlace.rcNormalPosition;
		cs.x = r.left;
		cs.y = r.top;
		cs.cx = r.Width();
		cs.cy = r.Height();
		m_HaveWndPlace = TRUE;
	}
	return CMDIFrameWnd::PreCreateWindow(cs);
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	theApp.m_pMainWnd = this;	// so components can use AfxGetMainWnd
	if (!m_ToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_ToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
	if (!m_StatusBar.Create(this) ||
		!m_StatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	if (!CreateSizingBars())
	{
		TRACE0("Failed to create sizing control bars\n");
		return -1;      // fail to create
	}
	// replacing CBRS_ALIGN_ANY with the following MAGIC prevents horizontally
	// docked bars from taking over the whole frame width; thanks Cristi Posea
	EnableDocking(CBRS_ALIGN_TOP);
	EnableDocking(CBRS_ALIGN_LEFT);
	EnableDocking(CBRS_ALIGN_RIGHT);
	EnableDocking(CBRS_ALIGN_BOTTOM);
	m_ToolBar.EnableDocking(CBRS_ALIGN_ANY);
	// use a custom dock frame that can handle zero idle time
	// set m_pFloatingFrameClass after ALL EnableDocking calls,
	// because EnableDocking also sets m_pFloatingFrameClass
	m_pFloatingFrameClass = RUNTIME_CLASS(CSizingDockFrame);
	DockControlBar(&m_ToolBar);
	DockSizingBars();
	if (VerifyBarState(REG_SETTINGS)) {
		for (int i = 0; i < SIZING_BARS; i++)
			m_SizingBar[i]->LoadState(REG_SETTINGS);
		LoadBarState(REG_SETTINGS);
	}
	for (int i = 0; i < SIZING_BARS; i++)
		m_SizingBar[i]->SendMessage(WM_INITDIALOG);	// initialize sizing bars
	DragAcceptFiles();
	m_Recorder.Create();
	m_RecordDlg.Create(IDD_RECORD);
	m_MidiSetupDlg.Create(IDD_MIDI_SETUP);
	UpdateMidiDevice();
	LoadMidiSetup();

	return 0;
}

bool CMainFrame::CreateSizingBars()
{
	static const int BAR_STYLE = CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC;
	static const int STYLE = WS_CHILD | CBRS_TOP;	// invisible to avoid flicker
	CString	Title;
	for (int i = 0; i < SIZING_BARS; i++) {
		const SIZING_BAR_INFO& info = m_SizingBarInfo[i];
		CSizingControlBar	*bar = m_SizingBar[i];
		Title.LoadString(info.TitleID);
		if (!bar->Create(Title, this, MAKEBARID(info.BarIdx), STYLE))
			return(FALSE);
		bar->SetBarStyle(bar->GetBarStyle() | BAR_STYLE);
	}
	return(TRUE);
}

void CMainFrame::DockSizingBars()
{
	CDWordArray	RegDockStyle;
	RegDockStyle.SetSize(SIZING_BARS);
	DWORD	ValSize = INT64TO32(RegDockStyle.GetSize()) * sizeof(DWORD);
	BOOL	GotRegDockStyle = CPersist::GetBinary(REG_SETTINGS, 
		RK_BAR_DOCK_STYLE, RegDockStyle.GetData(), &ValSize);
	for (int i = 0; i < SIZING_BARS; i++) {
		const SIZING_BAR_INFO& info = m_SizingBarInfo[i];
		CSizingControlBar	*bar = m_SizingBar[i];
		// use dock style from registry if we have it, else take default value
		DWORD	DockStyle = GotRegDockStyle ? RegDockStyle[i] : info.DockStyle;
		bar->EnableDocking(DockStyle);
		if (DockStyle) {	// if bar is dockable
			DockControlBar(bar, info.DockBarID);
			if (info.InitShow)	// if bar is initially shown
				ShowControlBar(bar, TRUE, 0);	// show it
		} else {	// bar is undockable
			CPoint	pt(info.InitFloat);
			ClientToScreen(&pt);
			FloatControlBar(bar, pt);
		}
	}
}

BOOL CMainFrame::VerifyBarState(LPCTSTR lpszProfileName)
{
	CDockState	state;
	state.LoadState(lpszProfileName);
	return(VerifyDockState(state, this));
}

BOOL CMainFrame::VerifyDockState(const CDockState& state, CFrameWnd *Frm)
{
	// thanks to Cristi Posea at codeproject.com
	for (int i = 0; i < state.m_arrBarInfo.GetSize(); i++) {
		CControlBarInfo* pInfo = (CControlBarInfo*)state.m_arrBarInfo[i];
		ASSERT(pInfo != NULL);
		int nDockedCount = INT64TO32(pInfo->m_arrBarID.GetSize());
		if (nDockedCount > 0) {
			// dockbar
			for (int j = 0; j < nDockedCount; j++)
			{
				UINT	nID = (UINT) pInfo->m_arrBarID[j];
				if (nID == 0)
					continue; // row separator
				if (nID > 0xFFFF)
					nID &= 0xFFFF; // placeholder - get the ID
				if (Frm->GetControlBar(nID) == NULL)
					return FALSE;
			}
		}
		if (!pInfo->m_bFloating) // floating dockbars can be created later
			if (Frm->GetControlBar(pInfo->m_nBarID) == NULL)
				return FALSE; // invalid bar ID
	}
    return TRUE;
}

void CMainFrame::UpdateBars()
{
	m_ToolBar.OnUpdateCmdUI(this, TRUE);
	m_StatusBar.OnUpdateCmdUI(this, TRUE);
}

UINT CMainFrame::GetToolDlgState() const
{
	UINT	State = 0;
	for (int i = 0; i < TOOL_DLGS; i++) {
		if (m_ToolDlg[i]->IsWindowVisible())
			State |= (1 << i);
	}
	return(State);
}

void CMainFrame::SetToolDlgState(UINT State)
{
	for (int i = 0; i < TOOL_DLGS; i++)
		m_ToolDlg[i]->ShowWindow((State & (1 << i)) ? SW_SHOWNA : SW_HIDE);
}

void CMainFrame::SetView(CFracticeView *View)
{
	if (View == m_View)
		return;	// nothing to do
	m_View = View;
	CHistoryView	*pHistView = View != NULL ? View->GetHistoryView() : NULL;
	m_HistoryBar.SetView(pHistView);
	m_ParamsBar.SetView(View);
	m_LevelsBar.SetView(View);
	m_PaletteDlg.SetView(View);
	m_RecordDlg.SetView(View);
	m_ColorMappingDlg.SetView(View);
	m_ColorCyclingDlg.SetView(View);
	m_SlideShowDlg.SetView(View);
	GetServersDlg().SetView(View);
	if (View != NULL) {
		OnParamChange();
	} else {	// no current view; all documents closed
		m_StatusMsg.Empty();
		m_StatusBar.SetPaneText(0, NULL);
	}
}

void CMainFrame::OnParamChange()
{
	ASSERT(m_View);
	CString	sx, sy, sz;
	BigNum	Zoom;
	m_View->GetZoom(Zoom);
	int	prec = CFracticeView::COORDS_PRECISION;
	Zoom.Format(prec, sz);
	BigPoint	Origin;
	m_View->GetOrigin(Origin);
	Origin.x.Format(prec, sx);
	Origin.y.Format(prec, sy);
	m_StatusMsg.Format(_T("X: %s  Y: %s  Z: %s"), sx, sy, sz);
	m_StatusBar.SetPaneText(0, m_StatusMsg);
	m_ParamsBar.Update();
	m_ColorMappingBar.Update();
	m_ColorCyclingBar.Update();
}

void CMainFrame::EditAccels(CAccelArray& AccArr, const ACCEL_EDIT *EditList, bool Default)
{
	int	accs = AccArr.GetSize();
	for (int i = 0; i < accs; i++) {
		int	cmd = AccArr[i].cmd;
		bool	enab = Default;
		for (int j = 0; EditList[j].StartID; j++) {
			const ACCEL_EDIT&	ed = EditList[j];
			if (cmd == ed.StartID 
			|| ed.EndID && (cmd >= ed.StartID && cmd <= ed.EndID))
				enab = ed.Enable;
		}
		if (!enab)
			AccArr[i].cmd = 0;	// no-op this command
	}
}

void CMainFrame::OnRecord(bool Recording)
{
	// switch menus; document's GetDefaultMenu calls our GetChildMenu
	m_ChildFrameMenu = Recording ? m_RecMenu.m_hMenu : m_DocMenu.m_hMenu;
	OnUpdateFrameMenu(NULL);
	DrawMenuBar();
	// switch accelerators and toolbar
	if (Recording) {
		if (m_RecordAccel == NULL) {	// if record accelerators not created yet
			m_RecordAccel.LoadFromRes(IDR_MAINFRAME);
			CAccelArray	AccArr;
			m_RecordAccel.GetArray(AccArr);
			EditAccels(AccArr, m_RecordAccelEdit, FALSE);
			m_RecordAccel.LoadFromArray(AccArr);	// create record accelerators
		}
		m_hAccelTable = m_RecordAccel;	// set record accelerators
	} else {	// not recording
		m_hAccelTable = LoadAccelerators(theApp.m_hInstance, 
			MAKEINTRESOURCE(IDR_MAINFRAME));	// set main accelerators
	}
	m_ToolBar.LoadToolBar(Recording ? IDR_RECORDING : IDR_MAINFRAME);
	RecalcLayout();	// notify frame of toolbar size change
	// lock out user interfaces that could interfere with recording
	theApp.UpdateAllViews(NULL, CFracticeView::UVH_ENABLEWND, (CObject *)!Recording);
	m_ParamsBar.GetList().EnableWindow(!Recording);
	m_RecordDlg.EnableCtrls(!Recording);
	m_PaletteDlg.EnableCtrls(!Recording);
	m_ColorMappingDlg.EnableCtrls(!Recording);
	m_ColorCyclingDlg.EnableCtrls(!Recording);
	m_SlideShowDlg.EnableCtrls(!Recording);
}

bool CMainFrame::ShowWarning(int TemplateID, int TimeoutSecs)
{
	CProgressDlg	dlg(TemplateID);
	if (!dlg.Create())
		return(FALSE);
 	dlg.SendDlgItemMessage(IDC_WARNING_ICON, STM_SETICON,	// set warning icon
		(WPARAM)theApp.LoadStandardIcon(IDI_EXCLAMATION), 0);
	dlg.SetWindowPos(&CWnd::wndTopMost,	// make topmost so user sees warning
		0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	clock_t start = clock();
	clock_t	end = start + TimeoutSecs * CLOCKS_PER_SEC;
	dlg.SetRange(start, end);
	clock_t	now;
	do {
		if (dlg.Canceled())
			return(FALSE);
		now = clock();
		dlg.SetPos(now);
		Sleep(100);
	} while (now < end);
	return(TRUE);
}

bool CMainFrame::Shutdown(bool Warning)
{
	static const int WARN_SECS = 10;
	if (Warning && !ShowWarning(IDD_SHUTDOWN_WARN, WARN_SECS))
		return(FALSE);
	if (!theApp.SetShutdown(TRUE, EWX_SHUTDOWN | EWX_POWEROFF)) {
		CString	msg;
		msg.Format(IDS_MF_CANT_SHUTDOWN, GetLastError());
		AfxMessageBox(msg);
		return(FALSE);
	}
	PostMessage(WM_CLOSE);	// exit application gracefully
	return(TRUE);	// ExitInstance does actual shutdown
}

void CMainFrame::HideChrome(bool Enable)
{
	if (Enable == m_HideChrome)
		return;	// nothing to do
	// delay looks nicer, but only works if there's idle time
	static const bool	bDelay = FALSE;
	if (Enable) {	// order matters here
		m_ShowToolBar = m_ToolBar.IsWindowVisible();
		m_ShowStatusBar = m_StatusBar.IsWindowVisible();
		SetRedraw(FALSE);	// defer painting until we're ready
		ShowControlBar(&m_ToolBar, FALSE, bDelay);	// hide toolbar
		ShowControlBar(&m_StatusBar, FALSE, bDelay);	// hide status bar
		int	i;
		for (i = 0; i < SIZING_BARS; i++) {
			m_ShowSizingBar[i] = m_SizingBar[i]->IsVisible();
			ShowControlBar(m_SizingBar[i], FALSE, bDelay);
		}
		m_ShowToolDlg = GetToolDlgState();
		SetToolDlgState(0);	// hide all tool dialogs
		ModifyStyle(WS_CAPTION | WS_THICKFRAME, 0);	// hide caption and frame
		HMENU	hMenu = GetMenu()->Detach();
		m_Menu.Attach(hMenu);
		SetMenu(NULL);	// hide menus
		SetRedraw(TRUE);	// deferred paint
	} else {
		ShowControlBar(&m_ToolBar, m_ShowToolBar, bDelay);	// restore toolbar
		ShowControlBar(&m_StatusBar, m_ShowStatusBar, bDelay);	// retore status bar
		int	i;
		for (i = 0; i < SIZING_BARS; i++)
			ShowControlBar(m_SizingBar[i], m_ShowSizingBar[i], bDelay);
		SetToolDlgState(m_ShowToolDlg);
		ModifyStyle(0, WS_CAPTION | WS_THICKFRAME);	// restore caption and frame
		HMENU	hMenu = m_Menu.Detach();
		::SetMenu(m_hWnd, hMenu);	// restore menus
		m_ToolBar.OnUpdateCmdUI(this, bDelay);
		m_StatusBar.OnUpdateCmdUI(this, bDelay);
	}
	m_HideChrome = Enable;
}

void CMainFrame::FullScreen(bool Enable)
{
	if (Enable == m_IsFullScreen)
		return;	// nothing to do
	if (IsExclusive())
		SetFocus();	// otherwise we lose exclusive if a control has focus
	CRect	rc;
	if (Enable) {
		GetWindowRect(m_PreFullScrnRect);
		CBackBufDD::GetFullScreenRect(*this, rc);
		rc.InflateRect(2, 2);	// hide thin frame
		m_FullScreenRect = rc;
	} else {
		rc = m_PreFullScrnRect;
		m_FullScreenRect.SetRectEmpty();
	}
	CChildFrame	*pChildFrm;
	if (m_View != NULL) {
		pChildFrm = DYNAMIC_DOWNCAST(CChildFrame, m_View->GetParent());
		// set child frame's min/max limits before doing any resizing
		pChildFrm->SetFullScreenRect(Enable ? &rc : NULL);
	} else
		pChildFrm = NULL;
	if (Enable)		// hide chrome BEFORE setting window pos
		HideChrome(TRUE);
	// send WM_NCCALCSIZE to the window, even if the window's size isn't
	// being changed, and discard the entire contents of the client area
	SetWindowPos(NULL, rc.left, rc.top, rc.Width(), rc.Height(), 
		SWP_FRAMECHANGED | SWP_NOCOPYBITS | SWP_NOZORDER);
	if (!Enable)	// restore chrome AFTER setting window pos
		HideChrome(FALSE);
	if (pChildFrm != NULL)
		pChildFrm->FullScreen(Enable);	// resize child frame
	m_IsFullScreen = Enable;
}

bool CMainFrame::SetSingleMonitorExclusiveAccels(bool Enable)
{
	if (Enable == (m_PrevMainAccel != NULL))
		return(TRUE);	// nothing to do
	if (Enable) {
		if (m_SMExclAccel == NULL) {
			CAccelArray	AccArr;
			CAccelTable::GetArray(m_hAccelTable, AccArr);
			EditAccels(AccArr, m_SMExclAccelEdit, FALSE);
			m_SMExclAccel.LoadFromArray(AccArr);
			if (m_SMExclAccel == NULL)
				return(FALSE);	// error creating accelerator table
		}
		m_PrevMainAccel = m_hAccelTable;
		m_hAccelTable = m_SMExclAccel;
	} else {
		m_hAccelTable = m_PrevMainAccel;
		m_PrevMainAccel = NULL;
		m_SMExclAccel.Destroy();
	}
	return(TRUE);
}

bool CMainFrame::Exclusive(bool Enable, const DISPLAY_MODE_INFO *ModeInfo)
{
	if (Enable == IsExclusive())
		return(TRUE);	// nothing to do
	if (!m_Mixer.Exclusive(Enable, ModeInfo))
		return(FALSE);	// mode switch failed
	// set non-blocking mode for tool dialogs and sizable control bars,
	// so non-client actions (close, move, resize) don't block message loop
	int	i;
	for (i = 0; i < TOOL_DLGS; i++)
		m_ToolDlg[i]->SetNonBlockingMode(Enable);
	for (i = 0; i < SIZING_BARS; i++) {
		CSizingDockFrame	*pDockFrm = DYNAMIC_DOWNCAST(
			CSizingDockFrame, m_SizingBar[i]->GetDockingFrame());
		if (pDockFrm != NULL)
			pDockFrm->SetNonBlockingMode(Enable);
	}
	if (Enable) {	// if entering exclusive mode
		// if we're dual-monitor, and we should go full-screen in dual monitor
		if (IsDualMonitor()) {
			if (m_FullScrnDualMon) {
				// make main frame full-screen, covering taskbar; otherwise it's
				// too easy to accidentally switch tasks and exit exclusive mode
				GetWindowPlacement(&m_PreExclWndPlace);
				CRect	rc;
				CBackBufDD::GetFullScreenRect(m_hWnd, rc);
				rc.InflateRect(2, 2);	// hide thin frame
				m_ExclusiveRect = rc;
				// if we're maximized, SetWindowPos won't cover the taskbar, so
				// un-maximize first, setting full-screen rect to avoid flicker
				if (IsZoomed())	{	// if we're maximized
					WINDOWPLACEMENT	wp = m_PreExclWndPlace;
					wp.rcNormalPosition = rc;	// full-screen rect
					wp.showCmd = SW_SHOWNORMAL;
					SetWindowPlacement(&wp);
				}
				ModifyStyle(WS_THICKFRAME, 0);	// remove thick frame style
				// go full-screen; force layout recalculation and total repaint
				SetWindowPos(&CWnd::wndTop, rc.left, rc.top, rc.Width(),
					rc.Height(), SWP_FRAMECHANGED | SWP_NOCOPYBITS);
				HookKeybd(TRUE);	// prevent task switching
			}
		} else {
			SetSingleMonitorExclusiveAccels(TRUE);	// set safe accelerators
		}
	} else {	// exiting exclusive mode
		if (!m_ExclusiveRect.IsRectNull()) {	// if our placement changed
			m_ExclusiveRect.SetRectEmpty();
			ModifyStyle(0, WS_THICKFRAME);	// restore thick frame style
			if (!m_HideChrome) {	// if our chrome is enabled
				if (m_PreExclWndPlace.showCmd == SW_SHOWMAXIMIZED)
					ShowWindow(SW_MAXIMIZE);	// prevents nasty flicker
				SetWindowPlacement(&m_PreExclWndPlace);	// restore placement
			}
		}
		HookKeybd(FALSE);
		SetSingleMonitorExclusiveAccels(FALSE);	// restore normal accelerators
		if (m_DemoMode)
			CleanupDemo();
	}
	return(TRUE);
}

LRESULT CALLBACK CMainFrame::KeybdProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	CMainFrame	*frm = theApp.GetMain();
	if (nCode == HC_ACTION) {	// ignore peek
		LPKBDLLHOOKSTRUCT	khs = (LPKBDLLHOOKSTRUCT)lParam;
		switch (khs->vkCode) {	// disable task-switching keys
		case VK_LWIN:	// Windows keys
		case VK_RWIN:
			return(TRUE);	// eat key
		case VK_ESCAPE:
			if ((khs->flags & LLKHF_ALTDOWN)	// Alt+Escape 
			|| (GetAsyncKeyState(VK_CONTROL) & GKS_DOWN))	// Ctrl+Escape
				return(TRUE);	// eat key
			break;
		case VK_TAB:
			if (khs->flags & LLKHF_ALTDOWN)	// Alt+Tab
				return(TRUE);	// eat key
			break;
		case VK_LMENU:	// Alt keys
		case VK_RMENU:
			// if exclusive on a single monitor
			if (frm->IsSingleMonitorExclusive())
				return(TRUE);	// eat key to avoid modal menu state
		}
	}
	return(CallNextHookEx(frm->m_KeybdHook, nCode, wParam, lParam));
}

bool CMainFrame::HookKeybd(bool Enable)
{
	bool	retc = TRUE;
	if (Enable != (m_KeybdHook != NULL)) {	// if not in requested state
		if (Enable) {
			m_KeybdHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeybdProc, 
				theApp.m_hInstance, 0);
			retc = m_KeybdHook != NULL;
		} else {
			retc = UnhookWindowsHookEx(m_KeybdHook) != 0;
			m_KeybdHook = NULL;
		}
	}
	return(retc);
}

void CMainFrame::ToggleWindow(CWnd& Wnd)
{
	Wnd.ShowWindow(Wnd.IsIconic() ? SW_RESTORE : 
		Wnd.IsWindowVisible() ? SW_HIDE : SW_SHOW);
}

void CMainFrame::CStatusMsg::Create(LPCTSTR Msg, bool WaitCursor)
{
	CMainFrame	*Main = theApp.GetMain();
	if (Main != NULL) {
		CStatusBar&	StatusBar = Main->GetStatusBar();
		StatusBar.GetPaneText(0, m_PrevMsg);	// save main's status message
		StatusBar.SetPaneText(0, Msg);	// override status message
		m_WaitCursor = WaitCursor;
		if (WaitCursor)
			theApp.BeginWaitCursor();
	}
}

CMainFrame::CStatusMsg::~CStatusMsg()
{
	CMainFrame	*Main = theApp.GetMain();
	if (Main != NULL) {
		CStatusBar&	StatusBar = Main->GetStatusBar();
		StatusBar.SetPaneText(0, m_PrevMsg);	// restore main's status message
		if (m_WaitCursor)
			theApp.EndWaitCursor();
	}
}

void CMainFrame::SetFolder(int PaneIdx, LPCTSTR Path)
{
	CPathStr	Folder(Path);
	Folder.RemoveFileSpec();
	m_FilesBar.SetFolder(PaneIdx, Folder);
}

void CMainFrame::UpdateMidiDevice()
{
	int	Dev = m_OptsDlg.GetMidiDevice();
	if (Dev != m_MidiDev) {
		m_MidiIO.EnableInput(FALSE);	// close previous MIDI device if any
		if (Dev != CMidiIO::NO_DEVICE) {
			if (!m_MidiIO.EnableInput(TRUE, Dev)) {
				CString	s, msg;
				m_MidiIO.GetLastErrorString(s);
				AfxFormatString1(msg, IDS_MIDI_CANT_CREATE, s); 
				AfxMessageBox(msg);
			}
		}
		m_MidiDev = Dev;
	}
}

void CALLBACK CMainFrame::MidiCallback(HMIDIIN handle, UINT uMsg, DWORD dwInstance,
									   DWORD dwParam1, DWORD dwParam2)
{
	// This function runs in a system thread, so beware of data corruption;
	// also avoid calling any system functions except for those listed in the
	// SDK under MidiInProc, and keep processing time to the absolute minimum.
	if (uMsg != MIM_DATA)
		return;
	CMainFrame	*frm = (CMainFrame *)dwInstance;
	CMidiIO::MSG	msg;
	msg.dw = dwParam1;
	if (msg.s.cmd >= 0xf0) {	// system message
	} else 	// channel message
		::PostMessage(frm->m_hWnd, UWM_MIDIIN, msg.dw, 0);
}

void CMainFrame::SetMidiProperty(int PropIdx, int Val, bool Toggle)
{
	m_MidiSetupDlg.SetValue(PropIdx, Val);	// update property's shadow
	float	r;
	CMidiSetupDlg::DRANGE	MSRange;
	m_MidiSetupDlg.GetRange(PropIdx, MSRange);
	double	Range = MSRange.End - MSRange.Start;
	if (Toggle) {	// alternate between extremes
		r = float(m_NoteToggle[PropIdx] ? MSRange.Start : MSRange.End);
		m_NoteToggle[PropIdx] ^= 1;
	} else {	// pan style: 0 = hard left, 64 = center, 127 = hard right
		int		n = Val - 64;	// convert from offset to signed
		double	HalfRange = Range / 2;
		r = float(MSRange.Start + HalfRange + n / float(n < 0 ? 64 : 63) * HalfRange);
	}
	int	ChanIdx;
	if (PropIdx < GLOBAL_MIDI_PROPS + CHAN_MIDI_PROPS)
		ChanIdx = 0;
	else {	// property of channel B
		ChanIdx = 1;
		PropIdx -= CHAN_MIDI_PROPS;
	}
	CFracticeView	*View = m_Mixer.GetView(ChanIdx);
	switch (PropIdx) {	// keep cases in MidiProps.h order for clarity
	case MP_FADER_POS:
		m_MixerDlg.SetFaderPos(r);
		break;
	case MP_AUTOFADE_TIME:
		m_MixerDlg.SetAutofadeTimeNorm(max(r, 0));
		break;
	case MP_AUTOFADE_PLAY:
		m_MixerDlg.SetAutofadePlay(Toggle ? !m_Mixer.GetPlay() : r >= .5);
		break;
	case MP_AUTOFADE_LOOP:
		m_MixerDlg.SetAutofadeLoop(Toggle ? !m_Mixer.GetLoop() : r >= .5);
		break;
	case MP_MIRROR:
		m_MixerDlg.SetMirror(Toggle ? !m_Mixer.GetMirror() : r >= .5);
		break;
	case MP_PAL_TWEEN_TIME:
		m_MixerDlg.SetPalTweenTimeNorm(max(r, 0));
		break;
	case MP_CYCLE_LENGTH:
		if (View != NULL)
			m_ColorMappingDlg.SetCycleLengthNorm(View, max(r, 0));
		break;
	case MP_COLOR_OFFSET:
		if (View != NULL)
			m_ColorMappingDlg.SetColorOffsetNorm(View, max(r, 0));
		break;
	case MP_COLOR_CYCLE_RATE:
		if (View != NULL)
			m_ColorCyclingDlg.SetCycleRateNorm(View, r);
		break;
	case MP_COLOR_CYCLE_ENABLE:
		if (View != NULL)
			View->SetCycleColors(Toggle ? !View->GetCycleColors() : r >= .5);
		break;
	case MP_ZOOM_IN:
		if (View != NULL) {
			View->ZoomCenter(m_OptsDlg.GetZoomStep());
			View->SetAddToHistory(FALSE);
		}
		break;
	case MP_ZOOM_OUT:
		if (View != NULL) {
			View->ZoomCenter(1 / m_OptsDlg.GetZoomStep());
			View->SetAddToHistory(FALSE);
		}
		break;
	case MP_SCROLL_UP:
		if (View != NULL) {
			View->Scroll(DPoint(0, -m_OptsDlg.GetScrollDelta()));
			View->SetAddToHistory(FALSE);
		}
		break;
	case MP_SCROLL_DOWN:
		if (View != NULL) {
			View->Scroll(DPoint(0, m_OptsDlg.GetScrollDelta()));
			View->SetAddToHistory(FALSE);
		}
		break;
	case MP_SCROLL_LEFT:
		if (View != NULL) {
			View->Scroll(DPoint(-m_OptsDlg.GetScrollDelta(), 0));
			View->SetAddToHistory(FALSE);
		}
		break;
	case MP_SCROLL_RIGHT:
		if (View != NULL) {
			View->Scroll(DPoint(m_OptsDlg.GetScrollDelta(), 0));
			View->SetAddToHistory(FALSE);
		}
		break;
	case MP_ORIGIN_MOTION:
		m_MixerDlg.SetOriginMotion(ChanIdx, ParmToEnum(r, CMixer::OM_TYPES));
		break;
	case MP_ORIGIN_X:
		if (m_MixerDlg.GetOriginMotion(ChanIdx) == CMixer::OM_MIDI_DRAG)
			m_Mixer.SetTargetOriginX(ChanIdx, r);
		break;
	case MP_ORIGIN_Y:
		if (m_MixerDlg.GetOriginMotion(ChanIdx) == CMixer::OM_MIDI_DRAG)
			m_Mixer.SetTargetOriginY(ChanIdx, r);
		break;
	case MP_DAMPING:
		m_MixerDlg.SetDampingNorm(ChanIdx, CLAMP(r, 0, 1));	// else all hell breaks loose
		break;
	case MP_TEMPO:
		m_MixerDlg.SetTempoNorm(ChanIdx, max(r, 0));
		break;
	case MP_PONG_SPEED:
		m_MixerDlg.SetPongSpeedNorm(ChanIdx, r);
		break;
	case MP_LISS_SPEED:
		m_MixerDlg.SetLissSpeedNorm(ChanIdx, r);
		break;
	case MP_LISS_PHASE:
		m_MixerDlg.SetLissPhaseNorm(ChanIdx, r);
		break;
	default:
		ASSERT(0);
	}
}

bool CMainFrame::LoadMidiSetup()
{
	static const LPCTSTR MIDI_SETUP_FILE_NAME = _T("MIDISetup.txt");
	CPathStr	path;
	theApp.GetAppDataFolder(path);
	path.Append(MIDI_SETUP_FILE_NAME);
	m_MidiSetupPath = path;
	if (PathFileExists(path)) {
		CMidiInfo	mi;
		m_MidiSetupDlg.GetDefaults(mi);
		if (!mi.Read(path))
			return(FALSE);
		m_MidiSetupDlg.SetInfo(mi);
	}
	return(TRUE);
}

bool CMainFrame::StoreMidiSetup()
{
	if (m_MidiSetupDlg.GetCount()) {
		CMidiInfo	mi;
		m_MidiSetupDlg.GetInfo(mi);
		if (!mi.Write(m_MidiSetupPath))
			return(FALSE);
	}
	return(TRUE);
}

bool CMainFrame::ShowDemo()
{
	static const MIXER_CHAN_INFO DemoChanInfo = {
		CMixer::OM_LISSAJOUS,	// m_OrgMotion
		1,		// m_PongSpeed
		.19,	// m_Damping
		60,		// m_Tempo
		.001,	// m_LissSpeed
		8,		// m_LissXFreq
		9,		// m_LissYFreq
		0,		// m_LissPhase
	};
	static const MIXER_MISC_INFO DemoMiscInfo = {
		0,		// m_FaderPos
		TRUE,	// m_AutofadePlay
		TRUE,	// m_AutofadeLoop
		15,		// m_AutofadeTime
		10,		// m_PalTweenTime
		TRUE,	// m_Mirror
		0,		// m_CurChan
	};
	enum {	// pair of documents
		A,	// demo project
		B,	// copy of demo project with different palettes
		DOCS
	};
	enum {
		DEMO_PROJECT_ITEMS = 17,
		SLIDE_SHOW_ITEMS = DEMO_PROJECT_ITEMS - 1,	// last item is for B palette
		SLIDE_INTERVAL = 60,	// in seconds
	};
	static const double COLOR_CYCLE_RATE = .02;
	static const int InitialSlide[DOCS] = {0, 6};
	static const LPCTSTR DemoProjectName = _T("FracticeDemo.frp");
	static const DISPLAY_MODE_INFO DisplayModeInfo = {
		1024,	// Width
		768,	// Height
		32,		// BPP
	};
	CPathStr	path(theApp.GetAppFolder());
	path.Append(DemoProjectName);
	CFracticeView	*View[DOCS];
	if (theApp.OpenDocumentFile(path) == NULL)	// open demo project
		return(FALSE);
	View[A] = m_View;
	SendMessage(WM_COMMAND, ID_FILE_NEW);	// create B view
	View[B] = m_View;
	CHistoryView	*Hist[DOCS];
	Hist[A] = View[A]->GetHistoryView();
	Hist[B] = View[B]->GetHistoryView();
	int	items = Hist[A]->GetItemCount();
	if (items < DEMO_PROJECT_ITEMS) {	// verify demo project's item count
		AfxMessageBox(IDS_BAD_DEMO_PROJECT);
		return(FALSE);
	}
	DPalette	BPal = Hist[A]->GetItem(SLIDE_SHOW_ITEMS)->m_Palette;
	int	i;
	// build B project; same as demo project except different palettes
	for (i = 0; i < items; i++) {	// for each item in A view
		CSnapshot&	SrcItem = Hist[A]->GetItem(i);
		CHistoryItem	DstItem;
		DstItem.CreateObj(SrcItem);	// clone item
		DstItem->m_Palette = BPal;	// override item's palette
		Hist[B]->AddItem(DstItem);	// add item to B view
	}
	// disable saving escape times
	COptionsInfo	Opts;
	m_OptsDlg.GetInfo(Opts);
	COptionsInfo	PrevOpts(Opts);	// make backup of options first
	Opts.m_SaveEscTimes = FALSE;
	m_OptsDlg.SetInfo(Opts);
	// render initial slides
	for (i = 0; i < DOCS; i++) {	// for both projects
		View[i]->ApplyOptions();	// disable saving escape times
		MDIActivate(View[i]->GetParent());	// activate project
		Hist[i]->SelectItem(InitialSlide[i], TRUE);	// select first slide
		CRenderingDlg	dlg;
		dlg.SetCaption(LDS(IDS_MF_PREPARING_DEMO));
		if (dlg.DoModal() != IDOK) {	// render first slide
			m_OptsDlg.SetInfo(PrevOpts);	// restore previous options
			return(FALSE);	// user canceled
		}
	}
	// disable image caching too, to avoid needless memory usage; this is done
	// separately because rendering dialog considers no image caching an error
	Opts.m_CacheImages = FALSE;
	m_OptsDlg.SetInfo(Opts);
	CMixerInfo	mi;
	mi.SetMiscInfo(DemoMiscInfo);
	m_PrevFitInWindow = View[A]->GetFitInWindow();	// save fit in window state
	for (i = 0; i < DOCS; i++) {	// for both projects
		mi.SetChan(i, DemoChanInfo);	// set mixer channel info
		CFracticeView	*vp = View[i];
		vp->SetColorCycleRate(COLOR_CYCLE_RATE);	// set color cycle rate
		vp->SetCycleColors(TRUE);	// and enable color cycling
		vp->SetFitInWindow(FALSE);	// avoid stretch blit, improves performance
		vp->ApplyOptions();			// disable image caching
		m_Mixer.SetView(i, vp);		// set mixer channel to view
		for (int j = 0; j < SLIDE_SHOW_ITEMS; j++)	// for each item
			Hist[i]->SelectItem(j, TRUE);	// select item for slide show
		CSlideShowMgr&	sm = Hist[i]->GetSlideShowMgr();
		sm.SetInterval(SLIDE_INTERVAL + i);
		sm.SetOrder(CSlideShowMgr::OT_RANDOM);
		sm.SetLoop(TRUE);
		sm.Play();	// start slide show
	}
	m_OptsDlg.SetInfo(PrevOpts);	// restore previous options
	mi.m_Chan[B].m_LissPhase = .5;	// 90 degree phase difference between A and B
	m_MixerDlg.SetInfo(mi);
	if (!Exclusive(TRUE, &DisplayModeInfo))	// enter exclusive mode
		return(FALSE);
	m_DemoMode = TRUE;
	return(TRUE);
}

void CMainFrame::CleanupDemo()
{
	if (!m_DemoMode)
		return;
	for (int i = 0; i < MIXER_CHANS; i++) {
		CFracticeView	*View = m_Mixer.GetView(i);
		if (View != NULL) {
			View->SetFitInWindow(m_PrevFitInWindow);	// restore fit in window
			View->GetParent()->PostMessage(WM_CLOSE);
		}
	}
	m_DemoMode = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message map

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_APP_CANCEL, OnAppCancel)
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_DROPFILES()
	ON_COMMAND(ID_EDIT_OPTIONS, OnEditOptions)
	ON_COMMAND(ID_FILE_RECORD, OnFileRecord)
	ON_COMMAND(ID_FILE_RECORD_STOP, OnFileRecordStop)
	ON_WM_GETMINMAXINFO()
	ON_COMMAND(ID_IMG_CANCEL, OnImgCancel)
	ON_COMMAND(ID_IMG_SLIDE_SHOW, OnImgSlideShow)
	ON_WM_SHOWWINDOW()
	ON_WM_SYSCOMMAND()
	ON_UPDATE_COMMAND_UI(ID_FILE_RECORD, OnUpdateFileRecord)
	ON_UPDATE_COMMAND_UI(ID_IMG_CANCEL, OnUpdateImgCancel)
	ON_UPDATE_COMMAND_UI(ID_IMG_SLIDE_SHOW, OnUpdateImgSlideShow)
	ON_UPDATE_COMMAND_UI(ID_VIEW_COLOR_CYCLING, OnUpdateViewColorCycling)
	ON_UPDATE_COMMAND_UI(ID_VIEW_COLOR_MAPPING, OnUpdateViewColorMapping)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FILES, OnUpdateViewFiles)
	ON_UPDATE_COMMAND_UI(ID_VIEW_HISTORY, OnUpdateViewHistoryBar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_JOB_CONTROL, OnUpdateViewJobControl)
	ON_UPDATE_COMMAND_UI(ID_VIEW_LEVELS, OnUpdateViewLevels)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MIDI_SETUP, OnUpdateViewMidiSetup)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MIXER, OnUpdateViewMixer)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PALETTE, OnUpdateViewPalette)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PARAMS, OnUpdateViewParams)
	ON_UPDATE_COMMAND_UI(ID_VIEW_RECORD, OnUpdateViewRecord)
	ON_UPDATE_COMMAND_UI(ID_VIEW_RECORD_STATUS, OnUpdateViewRecordStatus)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SERVERS, OnUpdateViewServers)
	ON_UPDATE_COMMAND_UI(ID_WND_EXCLUSIVE, OnUpdateWndExclusive)
	ON_COMMAND(ID_VIEW_COLOR_CYCLING, OnViewColorCycling)
	ON_COMMAND(ID_VIEW_COLOR_MAPPING, OnViewColorMapping)
	ON_COMMAND(ID_VIEW_FILES, OnViewFiles)
	ON_COMMAND(ID_VIEW_HISTORY, OnViewHistoryBar)
	ON_COMMAND(ID_VIEW_JOB_CONTROL, OnViewJobControl)
	ON_COMMAND(ID_VIEW_LEVELS, OnViewLevels)
	ON_COMMAND(ID_VIEW_MIDI_SETUP, OnViewMidiSetup)
	ON_COMMAND(ID_VIEW_MIXER, OnViewMixer)
	ON_COMMAND(ID_VIEW_PALETTE, OnViewPalette)
	ON_COMMAND(ID_VIEW_PARAMS, OnViewParams)
	ON_COMMAND(ID_VIEW_RECORD, OnViewRecord)
	ON_COMMAND(ID_VIEW_RECORD_STATUS, OnViewRecordStatus)
	ON_COMMAND(ID_VIEW_SERVERS, OnViewServers)
	ON_COMMAND(ID_WND_EXCLUSIVE, OnWndExclusive)
	ON_COMMAND(ID_WND_FULL_SCREEN, OnWndFullScreen)
	ON_COMMAND(ID_VIEW_DEMO, OnViewDemo)
	//}}AFX_MSG_MAP
	ON_WM_ACTIVATEAPP()
	ON_MESSAGE(WM_SETMESSAGESTRING, OnSetMessageString)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_FRAME_SIZE, OnUpdateIndicatorFrameSize)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_QUALITY, OnUpdateIndicatorQuality)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_ANTIALIAS, OnUpdateIndicatorAntialias)
	ON_MESSAGE(UWM_RCVPACKET, OnRcvPacket)
	ON_MESSAGE(UWM_TCPCONNECT, OnTcpConnect)
	ON_MESSAGE(UWM_TCPACCEPT, OnTcpAccept)
	ON_MESSAGE(UWM_ABORTRECORD, OnAbortRecord)
	ON_MESSAGE(UWM_SHUTDOWN, OnShutdown)
	ON_COMMAND_RANGE(ID_EDIT_TOOL_HAND, ID_EDIT_TOOL_ZOOM, OnEditTool)
	ON_UPDATE_COMMAND_UI_RANGE(ID_EDIT_TOOL_HAND, ID_EDIT_TOOL_ZOOM, OnUpdateEditTool)
	ON_MESSAGE(UWM_MIDIIN, OnMidiIn)
	ON_MESSAGE(UWM_HANDLEDLGKEY, OnHandleDlgKey)
	ON_MESSAGE(WM_DISPLAYCHANGE, OnDisplayChange)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

void CMainFrame::OnDestroy() 
{
	m_Recorder.Destroy();
	CPersist::SaveWnd(REG_SETTINGS, this, RK_MAIN_FRAME);
	CMDIFrameWnd::OnDestroy();
}

BOOL CMainFrame::DestroyWindow() 
{
	StoreMidiSetup();
	CDWordArray	RegDockStyle;
	RegDockStyle.SetSize(SIZING_BARS);
	for (int i = 0; i < SIZING_BARS; i++) {
		CSizingControlBar	*bar = m_SizingBar[i];
		bar->SaveState(REG_SETTINGS);
		RegDockStyle[i] = bar->m_dwDockStyle;
	}
	SaveBarState(REG_SETTINGS);
	CPersist::WriteBinary(REG_SETTINGS, RK_BAR_DOCK_STYLE, 
		RegDockStyle.GetData(), INT64TO32(RegDockStyle.GetSize()) * sizeof(DWORD));
	return CMDIFrameWnd::DestroyWindow();
}

void CMainFrame::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	if (bShow && !m_WasShown && !IsWindowVisible()) {
		m_WasShown = TRUE;
		if (m_HaveWndPlace) {
			switch (m_WndPlace.showCmd) {
			case SW_SHOWMINIMIZED:
				m_WndPlace.showCmd = SW_SHOWNORMAL;
				break;
			case SW_SHOWMAXIMIZED:
				// maximize explicitly instead of relying on SetWindowPlacement,
				// otherwise sizing control bars may be expanded unneccessarily,
				// due to SetWindowPlacement calling CalcFixedLayout twice, once
				// for normal window, and again for maximized window; this fix
				// also requires PreCreateWindow setting oversize initial rect
				ShowWindow(SW_MAXIMIZE);
				break;
			}
			SetWindowPlacement(&m_WndPlace);
		}
		SetToolDlgState(m_ShowToolDlg);
	}
}

void CMainFrame::OnClose() 
{
	if (m_View != NULL) {
		if (!m_Recorder.StopCheck())
			return;
	}
	if (CPosterProgressDlg::GetThis() != NULL) {
		if (!CPosterProgressDlg::GetThis()->StopCheck())
			return;
	}
	Exclusive(FALSE);	// exit exclusive before exiting full screen
	FullScreen(FALSE);	// exit full screen before saving tool dialog states
	if (!IsIconic())	// if iconic, OnSysCommand already saved states
		m_ShowToolDlg = GetToolDlgState();
	CMDIFrameWnd::OnClose();
}

#if _MSC_VER < 1300
void CMainFrame::OnActivateApp(BOOL bActive, HTASK hTask) 
#else
void CMainFrame::OnActivateApp(BOOL bActive, DWORD hTask) 
#endif
{
	CMDIFrameWnd::OnActivateApp(bActive, hTask);
	if (!bActive) {	// if we're being deactivated
		Exclusive(FALSE);
		FullScreen(FALSE);
	}
}

void CMainFrame::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI) 
{
	if (!m_FullScreenRect.IsRectNull()) {
		lpMMI->ptMaxPosition = m_FullScreenRect.TopLeft();
		lpMMI->ptMaxTrackSize = CPoint(m_FullScreenRect.Size());
	} else if (!m_ExclusiveRect.IsRectNull()) {
		lpMMI->ptMaxPosition = m_ExclusiveRect.TopLeft();
		lpMMI->ptMaxTrackSize = CPoint(m_ExclusiveRect.Size());
	}
	CMDIFrameWnd::OnGetMinMaxInfo(lpMMI);
}

void CMainFrame::OnSysCommand(UINT nID, LPARAM lParam)
{
	// if minimizing, grab tool dialog state before OnSysCommand changes it
	if ((nID & 0xFFF0) == SC_MINIMIZE)	// mask off unreliable low nibble
		m_ShowToolDlg = GetToolDlgState();
	CMDIFrameWnd::OnSysCommand(nID, lParam);
	switch (nID & 0xFFF0) {	// mask off unreliable low nibble
	case SC_MAXIMIZE:
	case SC_MINIMIZE:
	case SC_RESTORE:
		// hide/show any iconic tool dialogs; see note in CToolDlg::OnShowWindow
		{
			BOOL	bShow = !IsIconic();
			for (int i = 0; i < TOOL_DLGS; i++) {
				if (m_ToolDlg[i]->IsIconic())
					m_ToolDlg[i]->ShowWindow(bShow ? SW_SHOWNA : SW_HIDE);
			}
		}
		break;
	}
}

void CMainFrame::OnDropFiles(HDROP hDropInfo) 
{
	SetActiveWindow();	// activate us first!
	UINT	nFiles = DragQueryFile(hDropInfo, (UINT)-1, NULL, 0);	// get file count
	for (UINT iFile = 0; iFile < nFiles; iFile++) {
		TCHAR	szPath[MAX_PATH];
		DragQueryFile(hDropInfo, iFile, szPath, MAX_PATH);
		if (!_tcsicmp(PathFindExtension(szPath), PALETTE_EXT)) {	// if palette
			if (m_View != NULL && !IsRecording())
				m_View->OpenPalette(szPath);
		} else	// assume document
			theApp.OpenDocumentFile(szPath);
	}
	DragFinish(hDropInfo);	// release memory
}

bool CMainFrame::OpenPalette(LPCTSTR Path, CPoint *DropPoint)
{
	CFracticeView	*View = NULL;
	if (DropPoint != NULL) {	// if palette is being dropped
		CWnd	*wp = WindowFromPoint(*DropPoint);
		View = DYNAMIC_DOWNCAST(CFracticeView, wp);
		if (View == NULL) {	// if cursor not in view, try child frame
			CChildFrame	*ChildFrm = DYNAMIC_DOWNCAST(CChildFrame, wp);
			if (ChildFrm != NULL)	// if cursor in child frame
				View = DYNAMIC_DOWNCAST(CFracticeView, ChildFrm->GetActiveView());
		}
	}
	if (View == NULL) {	// if not dropping, or cursor not in view
		View = m_View;	// try current view
		if (View == NULL)	// if no current view
			return(FALSE);	// we're out of options
	}
	if (IsRecording()) {
		AfxMessageBox(IDS_REC_CANT_CHANGE_PALETTE);
		return(FALSE);
	}
	bool	Import = !DPalette::IsNativeFormat(Path);
	return(View->OpenPalette(Path, Import));
}

LRESULT CMainFrame::OnSetMessageString(WPARAM wParam, LPARAM lParam)
{
	switch (wParam) {
	case 0:	// menu separator
		lParam = LPARAM("");	// empty message
		break;
	case AFX_IDS_IDLEMESSAGE:	// display our status message for these IDs
	case ID_HISTORY_VIEW:
	case ID_LEVELS_VIEW:
	case IDC_PARAMS_CTRL:
	case IDC_FB_TAB:
	case IDC_FB_LIST:
		wParam = 0;
		lParam = LPARAM(LPCTSTR(m_StatusMsg));
		break;
	case AFX_IDW_PANE_FIRST:	// 0xE900 from control bar; ignore it
		return FALSE;
	}
	return CMDIFrameWnd::OnSetMessageString(wParam, lParam);
}

void CMainFrame::OnUpdateIndicatorFrameSize(CCmdUI *pCmdUI)
{
	CString	s;
	if (m_View != NULL) {
		CSize	sz = m_View->GetImageSize();
		s.Format(_T("%d x %d"), sz.cx, sz.cy);
	}
	pCmdUI->SetText(s);
}

void CMainFrame::OnUpdateIndicatorQuality(CCmdUI *pCmdUI)
{
	CString	s;
	if (m_View != NULL)
		s.Format(_T("%d"), m_View->GetQuality());
	pCmdUI->SetText(s);
}

void CMainFrame::OnUpdateIndicatorAntialias(CCmdUI *pCmdUI)
{
	CString	s;
	if (m_View != NULL)
		s.Format(_T("%dx"), m_View->GetAntialias());
	pCmdUI->SetText(s);
}

LRESULT CMainFrame::OnRcvPacket(WPARAM wParam, LPARAM lParam)
{
	// wParam: packet pointer, lParam: cookie
	m_Recorder.OnRcvPacket((GENERIC_PACKET *)wParam, INT64TO32(lParam));
	return(0);
}

LRESULT CMainFrame::OnTcpConnect(WPARAM wParam, LPARAM lParam)
{
	// wParam: connected flag, lParam: cookie
	m_Recorder.OnTcpConnect(INT64TO32(wParam), INT64TO32(lParam));
	return(0);
}

LRESULT CMainFrame::OnTcpAccept(WPARAM wParam, LPARAM lParam)
{
	// wParam: WTcp pointer, lParam: client address
	m_Recorder.OnTcpAccept((WTcp *)wParam, (LPSTR)lParam);
	return(0);
}

LRESULT CMainFrame::OnAbortRecord(WPARAM wParam, LPARAM lParam)
{
	// wParam: error string resource ID, lParam: none
	m_Recorder.Abort(UINT64TO32(wParam));
	return(0);
}

LRESULT CMainFrame::OnShutdown(WPARAM wParam, LPARAM lParam)
{
	// wParam: warning flag, lParam: none
	Shutdown(wParam != 0);	// if non-zero, warn user first
	return(0);
}

LRESULT CMainFrame::OnMidiIn(WPARAM wParam, LPARAM lParam)
{
	CMidiIO::MSG	msg;
	msg.dw = UINT64TO32(wParam);
	// pass to setup dialog first, so learned assignments take effect immediately
	if (m_MidiSetupDlg.FastIsVisible())
		m_MidiSetupDlg.OnMidiIn(msg);
	int	idx;
	int	chan = msg.s.cmd & 0x0f;
	switch (msg.s.cmd & 0xf0) {
	case MC_NOTE_ON:
		if (!msg.s.p2)	// ignore zero velocity note off
			return(0);
		idx = m_MidiSetupDlg.GetNoteMapping(chan, msg.s.p1);
		if (idx >= 0)	// if message is mapped
			SetMidiProperty(idx, msg.s.p2, TRUE);	// toggle property
		break;
	case MC_KEY_AFT:
		break;
	case MC_CTRL_CHG:
		idx = m_MidiSetupDlg.GetCtrlMapping(chan, msg.s.p1);
		if (idx >= 0) 	// if message is mapped
			SetMidiProperty(idx, msg.s.p2, FALSE);	// set property
		break;
	case MC_PROG_CHG:
		break;
	case MC_CHAN_AFT:
		break;
	case MC_PITCH_BEND:
		break;
	}
	return(0);
}

LRESULT	CMainFrame::OnHandleDlgKey(WPARAM wParam, LPARAM lParam)
{
	return(theApp.HandleDlgKeyMsg((MSG *)wParam));
}

LRESULT	CMainFrame::OnDisplayChange(WPARAM wParam, LPARAM lParam)
{
	m_OptsDlg.UpdateDisplayList();
	return 0;
}

void CMainFrame::OnAppCancel() 
{
	m_FilesBar.GetCurList()->CancelDrag();
}

void CMainFrame::OnFileRecord() 
{
	if (IsRecording())
		m_Recorder.StopCheck();
	else
		m_RecordDlg.Show();
}

void CMainFrame::OnFileRecordStop() 
{
	m_Recorder.StopCheck();
}

void CMainFrame::OnUpdateFileRecord(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_Recorder.IsRecording());
}

void CMainFrame::OnEditTool(UINT nID) 
{
	nID -= ID_EDIT_TOOL_HAND;
	m_CurTool = nID;
	if (m_View != NULL)
		m_View->SendMessage(UWM_TOOLCHANGE, m_CurTool);
}

void CMainFrame::OnUpdateEditTool(CCmdUI* pCmdUI) 
{
	UINT	nID = pCmdUI->m_nID;
	nID -= ID_EDIT_TOOL_HAND;
	pCmdUI->SetRadio(m_CurTool == nID);
}

void CMainFrame::OnEditOptions() 
{
	COptionsInfo	Prev;
	m_OptsDlg.GetInfo(Prev);
	CString	PrevNicDesc = m_OptsDlg.GetNicDescription();
	if (m_OptsDlg.DoModal() == IDOK) {
		theApp.UpdateAllViews(NULL, CFracticeView::UVH_EDITOPTIONS);
		if (m_OptsDlg.GetListenServers() != (Prev.m_ListenServers != 0)
		|| m_OptsDlg.GetNetworkPort() != Prev.m_NetworkPort 
		|| m_OptsDlg.GetNicDescription() != PrevNicDesc) {
			m_Recorder.ListenServers(m_OptsDlg.GetListenServers());
		}
		if (m_OptsDlg.GetThreadCount() != Prev.m_ThreadCount)
			GetServersDlg().SetLocalThreadCount(m_OptsDlg.GetThreadCount());
		m_Mixer.ApplySettings();	// mixer uses frame rate for its timer
		m_PaletteDlg.SetHueRotation(m_OptsDlg.GetHueRotation() / 360.0);
		UpdateMidiDevice();
	}
}

void CMainFrame::OnImgCancel() 
{
	if (IsFullScreen())	// escape key also exits full screen mode
		FullScreen(FALSE);
	else if (IsSingleMonitorExclusive())
		Exclusive(FALSE);
	else {
		if (m_View != NULL)
			m_View->CancelRender();
	}
}

void CMainFrame::OnUpdateImgCancel(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable((m_View != NULL && m_View->IsRendering()) 
		|| IsFullScreen() || IsSingleMonitorExclusive());
}

void CMainFrame::OnImgSlideShow() 
{
	ShowControlBar(&m_SlideShowBar, !m_SlideShowBar.FastIsVisible(), FALSE);
}

void CMainFrame::OnUpdateImgSlideShow(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_SlideShowBar.FastIsVisible());
}

void CMainFrame::OnViewHistoryBar() 
{
	ShowControlBar(&m_HistoryBar, !m_HistoryBar.FastIsVisible(), FALSE);
}

void CMainFrame::OnUpdateViewHistoryBar(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_HistoryBar.FastIsVisible());
}

void CMainFrame::OnViewLevels() 
{
	ShowControlBar(&m_LevelsBar, !m_LevelsBar.FastIsVisible(), FALSE);
}

void CMainFrame::OnUpdateViewLevels(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_LevelsBar.FastIsVisible());
}

void CMainFrame::OnViewParams() 
{
	ShowControlBar(&m_ParamsBar, !m_ParamsBar.FastIsVisible(), FALSE);
}

void CMainFrame::OnUpdateViewParams(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_ParamsBar.FastIsVisible());
}

void CMainFrame::OnViewFiles() 
{
	ShowControlBar(&m_FilesBar, !m_FilesBar.FastIsVisible(), FALSE);
}

void CMainFrame::OnUpdateViewFiles(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_FilesBar.FastIsVisible());
}

void CMainFrame::OnViewPalette() 
{
	ShowControlBar(&m_PaletteBar, !m_PaletteBar.FastIsVisible(), FALSE);
}

void CMainFrame::OnUpdateViewPalette(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_PaletteBar.FastIsVisible());
}

void CMainFrame::OnViewColorMapping() 
{
	ShowControlBar(&m_ColorMappingBar, !m_ColorMappingBar.FastIsVisible(), FALSE);
}

void CMainFrame::OnUpdateViewColorMapping(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_ColorMappingBar.FastIsVisible());
}

void CMainFrame::OnViewColorCycling() 
{
	ShowControlBar(&m_ColorCyclingBar, !m_ColorCyclingBar.FastIsVisible(), FALSE);
}

void CMainFrame::OnUpdateViewColorCycling(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_ColorCyclingBar.FastIsVisible());
}

void CMainFrame::OnViewMixer() 
{
	ShowControlBar(&m_MixerBar, !m_MixerBar.FastIsVisible(), FALSE);
}

void CMainFrame::OnUpdateViewMixer(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_MixerBar.FastIsVisible());
}

void CMainFrame::OnViewMidiSetup() 
{
	ToggleWindow(m_MidiSetupDlg);
}

void CMainFrame::OnUpdateViewMidiSetup(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_MidiSetupDlg.FastIsVisible());
}

void CMainFrame::OnViewRecord() 
{
	ToggleWindow(m_RecordDlg);
}

void CMainFrame::OnUpdateViewRecord(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_RecordDlg.FastIsVisible());
}

void CMainFrame::OnViewRecordStatus() 
{
	ToggleWindow(GetRecordStatusDlg());
}

void CMainFrame::OnUpdateViewRecordStatus(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(GetRecordStatusDlg().FastIsVisible());
}

void CMainFrame::OnViewServers() 
{
	ToggleWindow(GetServersDlg());
}

void CMainFrame::OnUpdateViewServers(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(GetServersDlg().FastIsVisible());
}

void CMainFrame::OnViewJobControl() 
{
	ToggleWindow(GetJobControlDlg());
}

void CMainFrame::OnUpdateViewJobControl(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(GetJobControlDlg().FastIsVisible());
}

void CMainFrame::OnViewDemo() 
{
	ShowDemo();
}

void CMainFrame::OnWndFullScreen() 
{
	FullScreen(!m_IsFullScreen);
}

void CMainFrame::OnWndExclusive() 
{
	Exclusive(!IsExclusive());
}

void CMainFrame::OnUpdateWndExclusive(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(IsExclusive());
}
