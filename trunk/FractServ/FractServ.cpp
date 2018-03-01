// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
        01      12feb09	add bind IP address flag
        02      09jan10	show bignum version in help

        fractal rendering server
 
*/

// FractServ.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "FractServ.h"
#include "FractServDlg.h"
#include "Shutdown.h"
#include "VersionInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// The one and only application object

CFracticeApp theApp;

const LPCTSTR CMyCmdLineInfo::m_FlagName[CL_FLAGS] = {
	_T("help"),		// CLF_HELP
	_T("?"),		// CLF_QUESTION_MARK
	_T("port"),		// CLF_NETWORK_PORT
	_T("noudp"),	// CLF_NO_UDP_SERVER
	_T("hide"),		// CLF_HIDE_CONSOLE
	_T("client"),	// CLF_CONNECT_CLIENT
	_T("bind"),		// CLF_BIND_IP_ADDRESS
	_T("threads"),	// CLF_THREADS
	_T("fpu"),		// CLF_FPU
};

const LPCTSTR CFracticeApp::m_HelpText = 
"\n"
"/help             display this help\n"
"/port number      use specified port number for TCP/UDP sockets\n"
"/noudp            don't launch UDP server to answer find broadcasts\n"
"/hide             make server invisible by hiding its console window\n"
"/client address   connect to client at specified IP address or hostname\n"
"/bind address     bind server to specified IP address\n"
"/threads count    create specified number of rendering threads\n"
"/fpu              use FPU instead of SSE2 for rendering math\n";

CMyCmdLineInfo::CMyCmdLineInfo()
{
	m_PrevFlag = -1;
	m_Abort = FALSE;
}

CMyCmdLineInfo::~CMyCmdLineInfo()
{
	if (m_Abort)
		theApp.ShowHelp();
}

void CMyCmdLineInfo::ParseParam(LPCTSTR lpszParam, BOOL bFlag, BOOL bLast)
{
	if (bFlag) {
		int	i;
		for (i = 0; i < CL_FLAGS; i++) {
			if (!_tcsicmp(lpszParam, m_FlagName[i]))
				break;
		}
		if (i < CL_FLAGS) {	// if flag name was found
			switch (i) {
			case CLF_HELP:
			case CLF_QUESTION_MARK:
				m_Abort = TRUE;
				break;
			case CLF_NO_UDP_SERVER:
				theApp.m_WantUdpServer = FALSE;
				break;
			case CLF_HIDE_CONSOLE:
				theApp.m_HideConsole = TRUE;
				break;
			case CLF_FPU:
				theApp.m_bUseSSE2 = FALSE;
				break;
			}
			m_PrevFlag = i;
		} else {	// unknown flag
			printf("invalid flag '%s'\n", lpszParam);
			m_PrevFlag = -1;
			m_Abort = TRUE;
		}
	} else {
		switch (m_PrevFlag) {
		case CLF_NETWORK_PORT:
			{
				WORD	port;
				sscanf(lpszParam, "%hu", &port);	// h for short
				theApp.m_NetworkPort = port;
			}
			break;
		case CLF_CONNECT_CLIENT:
			theApp.m_ClientAddress = lpszParam;
			break;
		case CLF_BIND_IP_ADDRESS:
			theApp.m_ServerIP = lpszParam;
			break;
		case CLF_THREADS:
			{
				UINT	count;
				sscanf(lpszParam, "%u", &count);
				theApp.m_ThreadCount = count;
			}
			break;
		}
		m_PrevFlag = -1;
	}
}

CFracticeApp::CFracticeApp()
{
	m_bHaveSSE2 = FALSE;
	m_bUseSSE2 = FALSE;
	m_bExitWindows = FALSE;
	m_uExitFlags = 0;
	m_nMainThreadId = 0;	
	m_NetworkPort = FRACTICE_NET_PORT;
	m_WantUdpServer = TRUE;
	m_HideConsole = FALSE;
	m_ThreadCount = 0;
}

void CFracticeApp::ShowHelp()
{
	VS_FIXEDFILEINFO	AppInfo;
	CVersionInfo::GetFileInfo(AppInfo, NULL);
	printf("FractServ Version %d.%d.%d.%d\n"
		"Copyleft 2010 Chris Korda\n"
		"%s %d.%d.%d %s/%s\n"
		"http://fractice.sourceforge.net\n"
		"%s",
		HIWORD(AppInfo.dwFileVersionMS), LOWORD(AppInfo.dwFileVersionMS),
		HIWORD(AppInfo.dwFileVersionLS), LOWORD(AppInfo.dwFileVersionLS),
		BigNum::GetLibName(), BigNum::GetVersion(), BigNum::GetVersionMinor(),
		BigNum::GetPatchLevel(), BigNum::GetOSName(), BigNum::GetCPUName(),
		m_HelpText);
}

bool CFracticeApp::SetShutdown(bool Enable, UINT Flags)
{
	if (Enable) {
		if (!GetShutdownPrivileges(Flags))	// try to obtain shutdown privileges
			return(FALSE);	// fail if privilege wasn't granted
	}
	// RunApp does the actual shutdown just before the app terminates
	m_bExitWindows = Enable;
	m_uExitFlags = Flags;
	return(TRUE);
}

BOOL WINAPI CFracticeApp::ConsoleCtrlHandler(DWORD dwCtrlType)
{
	switch (dwCtrlType) {
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:
	case CTRL_CLOSE_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		_tprintf(_T("exiting...\n"));
		::PostThreadMessage(theApp.m_nMainThreadId, WM_QUIT, 0, 0);
		WaitForSingleObject(theApp.m_QuitEvent, 5000);
		return(FALSE);
	}
	return(TRUE);
}

#if _MFC_VER < 0x0700
HWND WINAPI GetConsoleWindow()	// not public in MFC 6
{
	typedef	HWND (WINAPI *tGetConsoleWindow)();
	tGetConsoleWindow	pGetConsoleWindow = NULL;
	HWND	hWnd = NULL;
	HINSTANCE	hInst = ::LoadLibrary("Kernel32.dll");
	if (hInst != NULL) {
		pGetConsoleWindow = (tGetConsoleWindow)::GetProcAddress(hInst, 
			"GetConsoleWindow");
		if (pGetConsoleWindow != NULL)
			hWnd = pGetConsoleWindow();
		::FreeLibrary(hInst);
	}
	return(hWnd);
}
#endif

bool CFracticeApp::RunApp()
{
	// query CPU for SSE2 support
	m_bHaveSSE2 = CEngine::QuerySSE2();
	m_bUseSSE2 = m_bHaveSSE2;
#if _MFC_VER >= 0x0700	// SSE2 is mandatory in .NET version
	if (!m_bHaveSSE2) {
		printf("CPU doesn't support SSE2\n");
		return(FALSE);
	}
#endif
	CMyCmdLineInfo	cli;
	ParseCommandLine(cli);
	if (cli.Abort())
		return(FALSE);
	m_nMainThreadId = GetCurrentThreadId();
	m_QuitEvent.Create(NULL, FALSE, FALSE, NULL);
	SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);
	if (m_HideConsole) {
		HWND	hWnd = ::GetConsoleWindow();
		if (hWnd != NULL)
			::ShowWindow(hWnd, SW_HIDE);
	}
	{
		CFractServDlg	dlg((HWND)m_nMainThreadId);
		dlg.DoModal();
	}
	m_QuitEvent.Set();	// unblock console handler
	if (m_bExitWindows)
		ExitWindowsEx(m_uExitFlags, 0);	// asynchronous shutdown
	return(TRUE);
}

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;
	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0)) {
		_tprintf(_T("Fatal Error: MFC initialization failed\n"));
		nRetCode = 1;
	} else {
		theApp.RunApp();
	}
	return nRetCode;
}
