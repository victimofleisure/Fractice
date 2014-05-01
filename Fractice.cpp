// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		16mar09	add demo
		02		06jan10	W64: in HandleDlgKeyMsg, cast virtual key to 32-bit

        application
 
*/

// Fractice.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "Fractice.h"

#include "MainFrm.h"
#include "ChildFrm.h"
#include "FracticeDoc.h"
#include "FracticeView.h"
#include "AboutDlg.h"
#include "Win32Console.h"
#include "Shutdown.h"
#include "MultiFileDlg.h"
#include "PathStr.h"
#include "htmlhelp.h"	// needed for HTML Help API
#include "FocusEdit.h"	// for GetEdit
#include "MessageBoxCheck.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// macros for making folder paths persistent
#define RK_FOLDERS _T("Folders")
#define GET_FOLDER(name) m_##name = CPersist::GetString(RK_FOLDERS, _T(#name))
#define SET_FOLDER(name) CPersist::WriteString(RK_FOLDERS, _T(#name), m_##name)

#define RK_VIEW_DEMO	_T("FracticeViewDemo")

/////////////////////////////////////////////////////////////////////////////
// CFracticeApp

BEGIN_MESSAGE_MAP(CFracticeApp, CWinAppEx)
	//{{AFX_MSG_MAP(CFracticeApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_APP_HOME_PAGE, OnAppHomePage)
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_HELP_INDEX, OnHelpIndex)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinAppEx::OnFileNew)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinAppEx::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFracticeApp construction

CFracticeApp::CFracticeApp()
{
	// Place all significant initialization in InitInstance
	m_bHaveSSE2 = FALSE;
	m_bExitWindows = FALSE;
	m_uExitFlags = 0;
	m_HelpCookie = 0;
	m_DocFilterIndex = 1;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CFracticeApp object

CFracticeApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CFracticeApp initialization

BOOL CFracticeApp::InitInstance()
{
	AfxEnableControlContainer();

#ifdef _DEBUG
	Win32Console::Create();		// create console window
#endif

	// Standard initialization
#if _MFC_VER < 0x0700
#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif
#endif

	// Change the registry key under which our settings are stored.
	SetRegistryKey(_T("Anal Software"));

	// make sure our app data folder exists
	CString	sAppData;
	GetAppDataFolder(sAppData);
	CreateFolder(sAppData);

	// initialize random seed
	srand(GetTickCount());

	// query CPU for SSE2 support
	m_bHaveSSE2 = CEngine::QuerySSE2();
#if _MFC_VER >= 0x0700	// SSE2 is mandatory in .NET version
	if (!m_bHaveSSE2) {
		AfxMessageBox(IDS_APP_NO_SSE2);
		return(FALSE);
	}
#endif
	SYSTEM_INFO	si;
	GetSystemInfo(&si);
	m_CPUCount = si.dwNumberOfProcessors;	// get number of CPUs

	// get folder paths from registry
	GET_FOLDER(DocFolder);
	GET_FOLDER(PaletteFolder);
	GET_FOLDER(ImportFolder);
	GET_FOLDER(ExportFolder);
	GET_FOLDER(RecAviFolder);
	GET_FOLDER(RecBmpFolder);

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)
	FixMFCDotBitmap();	// replace wimpy SetRadio dot bitmap with a bigger one

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(
		IDR_FRACTICETYPE,
		RUNTIME_CLASS(CFracticeDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CFracticeView));
	AddDocTemplate(pDocTemplate);

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	m_pMainWnd = pMainFrame;

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it.
	m_pMainWnd->ShowWindow(m_nCmdShow);
	m_pMainWnd->UpdateWindow();

	// ask if user wants to view demo
	if (MessageBoxCheck(m_pMainWnd->m_hWnd, LDS(IDS_MF_VIEW_DEMO), 
	m_pszAppName, MB_YESNO | MB_ICONQUESTION, 0, RK_VIEW_DEMO) == IDYES)
		m_pMainWnd->PostMessage(WM_COMMAND, ID_VIEW_DEMO);	// start demo

	return TRUE;
}

bool CFracticeApp::SetShutdown(bool Enable, UINT Flags)
{
	if (Enable) {
		if (!GetShutdownPrivileges(Flags))	// try to obtain shutdown privileges
			return(FALSE);	// fail if privilege wasn't granted
	}
	// ExitInstance does the actual shutdown just before the app terminates
	m_bExitWindows = Enable;
	m_uExitFlags = Flags;
	return(TRUE);
}

bool CFracticeApp::HandleDlgKeyMsg(MSG* pMsg)
{
	static const LPCSTR	EditBoxCtrlKeys = "ACHVXZ";
	CMainFrame	*Main = theApp.GetMain();
	ASSERT(Main != NULL);	// main frame must exist
	switch (pMsg->message) {
	case WM_KEYDOWN:
		{
			int	VKey = INT64TO32(pMsg->wParam);
			CEdit	*pEdit = CFocusEdit::GetEdit();
			bool	bTryMainAccels = FALSE;	// assume failure
			if (pEdit != NULL) {	// if an edit control has focus
				if ((VKey >= VK_F1 && VKey <= VK_F24)			// if function key
				|| ((GetAsyncKeyState(VK_CONTROL) & GKS_DOWN)	// or Ctrl is down
				&& (VKey >= 'A' && VKey <= 'Z')					// and key is alpha
				&& strchr(EditBoxCtrlKeys, VKey) == NULL)		// and unused by edit
				|| (pEdit->IsKindOf(RUNTIME_CLASS(CNumEdit))	// or numeric edit
				&& (GetAsyncKeyState(VK_SHIFT) & GKS_DOWN)		// and Shift is down
				&& (VKey >= 'A' && VKey <= 'Z')))				// and key is alpha
					bTryMainAccels = TRUE;	// give main accelerators a try
			} else {	// non-edit control has focus
				if ((VKey >= VK_F1 && VKey <= VK_F24)			// if function key
				|| (VKey >= 'A' && VKey <= 'Z')					// or alpha key
				|| (GetAsyncKeyState(VK_CONTROL) & GKS_DOWN)	// or Ctrl is down
				|| (GetAsyncKeyState(VK_SHIFT) & GKS_DOWN))		// or Shift is down
					bTryMainAccels = TRUE;	// give main accelerators a try
			}
			if (bTryMainAccels) {
				HACCEL	hAccel = Main->GetAccelTable();
				if (hAccel != NULL
				&& TranslateAccelerator(Main->m_hWnd, hAccel, pMsg))
					return(TRUE);	// message was translated, stop dispatching
			}
		}
		break;
	case WM_SYSKEYDOWN:
		Main->SetFocus();	// causes main frame to display the appropriate menu
		return(TRUE);	// message was translated, stop dispatching
	}
	return(FALSE);	// continue dispatching
}

/////////////////////////////////////////////////////////////////////////////
// CFracticeApp message handlers

void CFracticeApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

int CFracticeApp::ExitInstance() 
{
	SET_FOLDER(DocFolder);	// write folder paths to registry
	SET_FOLDER(PaletteFolder);
	SET_FOLDER(ImportFolder);
	SET_FOLDER(ExportFolder);
	SET_FOLDER(RecAviFolder);
	SET_FOLDER(RecBmpFolder);
	// if HTML help was initialized, uninitialize it
	if (m_HelpCookie) {
		::HtmlHelp(NULL, NULL, HH_CLOSE_ALL, 0);
		::HtmlHelp(NULL, NULL, HH_UNINITIALIZE, m_HelpCookie);
	}
	if (m_bExitWindows)
		ExitWindowsEx(m_uExitFlags, 0);	// asynchronous shutdown
	return CWinAppEx::ExitInstance();
}

void CFracticeApp::OnFileOpen() 
{
	CMultiFileDlg	fd(TRUE, NULL, NULL, OFN_HIDEREADONLY,
		LDS(IDS_DOC_OPEN_FILTER), NULL, NULL, &m_DocFolder);
	fd.m_ofn.nFilterIndex = m_DocFilterIndex;
	if (fd.DoModal() == IDOK) {
		OpenDocumentFile(fd.GetPathName());
		GetMain()->SetFolder(CFilesBar::PANE_PROJECTS, fd.GetPathName());
		m_DocFilterIndex = fd.m_ofn.nFilterIndex;
	}
}

void CFracticeApp::OnAppHomePage() 
{
	CHyperlink::GotoUrl(LDS(IDS_APP_HOME_PAGE_URL));
}

void CFracticeApp::OnHelpIndex()
{
	WinHelp(0);
}

void CFracticeApp::WinHelp(DWORD dwData, UINT nCmd) 
{
	static const LPCTSTR	HELP_FILE_NAME = _T("Fractice.chm");
	// if HTML help hasn't been initialized yet, initialize it
	if (!m_HelpCookie)
		::HtmlHelp(NULL, NULL, HH_INITIALIZE, (DWORD)&m_HelpCookie);
	HWND	hMainWnd = m_pMainWnd->m_hWnd;
	HWND	retc = ::HtmlHelp(hMainWnd, HELP_FILE_NAME, HH_DISPLAY_TOC, 0);
	if (!retc) {	// not found, try appending help file name to app path
		CPathStr	HelpPath(GetAppFolder());
		HelpPath.Append(HELP_FILE_NAME);
		retc = ::HtmlHelp(hMainWnd, HelpPath, HH_DISPLAY_TOC, 0);	// try again
		if (!retc) {	// not found, give up
			CString	s;
			AfxFormatString1(s, IDS_MF_HELP_FILE_MISSING, HELP_FILE_NAME);
			AfxMessageBox(s);
		}
	}
}

// By default, CWinApp::OnIdle is called after WM_TIMER messages.  This isn't
// normally a problem, but if the application uses a short timer, OnIdle will
// be called frequently, seriously degrading performance.  Performance can be
// improved by overriding IsIdleMessage to return FALSE for WM_TIMER messages,
// which prevents them from triggering OnIdle.  This technique can be applied
// to any idle-triggering message that repeats frequently, e.g. WM_MOUSEMOVE.
//
BOOL CFracticeApp::IsIdleMessage(MSG* pMsg)
{
	if (CWinAppEx::IsIdleMessage(pMsg)) {
		switch (pMsg->message) {
		// don't call OnIdle after these messages
		case WM_TIMER:
			return(FALSE);
		default:
			// if keyboard or mouse event
			if ((pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST)
			|| (pMsg->message >= WM_MOUSEFIRST && pMsg->message <= WM_MOUSELAST)) {
				CMainFrame	*main = GetMain();
				CFracticeView	*view = main->GetView();
				if (view != NULL && view->GetCycleColors())	// if cycling colors
					main->UpdateBars();	// update control bars
			}
		}
		return(TRUE);
	} else
		return(FALSE);
}
