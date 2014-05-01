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

        fractal rendering server
 
*/

#if !defined(AFX_FRACTSERV_H__70B19102_956A_40F9_975D_BE71A15CBD31__INCLUDED_)
#define AFX_FRACTSERV_H__70B19102_956A_40F9_975D_BE71A15CBD31__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Resource.h"
#include "Event.h"

class CMyCmdLineInfo : public CCommandLineInfo {
public:
// Construction
	CMyCmdLineInfo();
	~CMyCmdLineInfo();

// Attributes
	bool	Abort() const;

protected:
// Constants
	enum {	// command line flags; must match m_FlagName
		CLF_HELP,				// display help
		CLF_QUESTION_MARK,		// display help
		CLF_NETWORK_PORT,		// specify TCP/UDP port number
		CLF_NO_UDP_SERVER,		// don't launch UDP server
		CLF_HIDE_CONSOLE,		// hide console window
		CLF_CONNECT_CLIENT,		// connect to specified client
		CLF_BIND_IP_ADDRESS,	// IP address to bind server to
		CLF_THREADS,			// number of rendering threads
		CLF_FPU,				// use FPU instead of SSE2
		CL_FLAGS
	};
	static const LPCTSTR m_FlagName[CL_FLAGS];	// command line flag names

// Data members
	int		m_PrevFlag;	// index of previous flag, or -1 if none
	bool	m_Abort;	// true if app should not continue

// Overrides
	void	ParseParam(LPCTSTR lpszParam, BOOL bFlag, BOOL bLast);
};

class CFracticeApp : public CWinApp {
public:
// Construction
	CFracticeApp();

// Attributes
	bool	HaveSSE2() const;
	bool	UseSSE2() const;
	WORD	GetNetworkPort() const;
	bool	WantUdpServer() const;
	LPCTSTR	GetClientAddress() const;
	LPCTSTR	GetServerIP() const;
	UINT	GetThreadCount() const;

// Operations
	bool	RunApp();
	bool	SetShutdown(bool Enable, UINT Flags);
	void	ShowHelp();

protected:
// Types
	friend class CMyCmdLineInfo;

// Data members
	bool	m_bHaveSSE2;		// true if CPU supports SSE2
	bool	m_bUseSSE2;			// true if using SSE2; false if using FPU
	bool	m_bExitWindows;		// true if closing app should exit windows
	UINT	m_uExitFlags;		// exit windows flags; see ExitWindowsEx
	UINT	m_nMainThreadId;	// thread identifier of the main thread
	WEvent	m_QuitEvent;		// console control handler blocks on this
	WORD	m_NetworkPort;		// network UDP/TCP port number
	bool	m_WantUdpServer;	// if true, launch UDP find server
	bool	m_HideConsole;		// if true, hide console window
	UINT	m_ThreadCount;		// number of rendering threads to launch
	CString	m_ClientAddress;	// if specified, connect to this client
	CString	m_ServerIP;			// if specified, bind server to this IP

// Constants
	static const LPCTSTR m_HelpText;	// command-line help

// Helpers
	static	BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType);
};

inline bool CMyCmdLineInfo::Abort() const
{
	return(m_Abort);
}

inline bool CFracticeApp::HaveSSE2() const
{
	return(m_bHaveSSE2);
}

inline bool CFracticeApp::UseSSE2() const
{
	return(m_bUseSSE2);
}

inline WORD CFracticeApp::GetNetworkPort() const
{
	return(m_NetworkPort);
}

inline bool CFracticeApp::WantUdpServer() const
{
	return(m_WantUdpServer);
}

inline LPCTSTR CFracticeApp::GetClientAddress() const
{
	return(m_ClientAddress.IsEmpty() ? LPCTSTR(NULL) : m_ClientAddress);
}

inline LPCTSTR CFracticeApp::GetServerIP() const
{
	return(m_ServerIP.IsEmpty() ? LPCTSTR(NULL) : m_ServerIP);
}

inline UINT CFracticeApp::GetThreadCount() const
{
	return(m_ThreadCount);
}

extern CFracticeApp theApp;

#endif // !defined(AFX_FRACTSERV_H__70B19102_956A_40F9_975D_BE71A15CBD31__INCLUDED_)
