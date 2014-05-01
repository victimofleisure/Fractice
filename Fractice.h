// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version

        application
 
*/

// Fractice.h : main header file for the FRACTICE application
//

#if !defined(AFX_FRACTICE_H__ED0ED84D_AA43_442C_85CD_A7FA518EBF90__INCLUDED_)
#define AFX_FRACTICE_H__ED0ED84D_AA43_442C_85CD_A7FA518EBF90__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "WinAppEx.h"       // base class
#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CFracticeApp:
// See Fractice.cpp for the implementation of this class
//

class CMainFrame;

class CFracticeApp : public CWinAppEx
{
public:
	CFracticeApp();

// Attributes
	CMainFrame	*GetMain();
	UINT	GetCPUCount() const;
	bool	HaveSSE2() const;
	bool	SetShutdown(bool Enable, UINT Flags);

// Operations
	static	bool	HandleDlgKeyMsg(MSG* pMsg);

// Public data
	CString	m_DocFolder;		// folder for projects and snapshots
	CString	m_PaletteFolder;	// folder for native palette files
	CString	m_ImportFolder;		// folder for imported palettes
	CString	m_ExportFolder;		// folder for exported bitmaps
	CString	m_RecAviFolder;		// folder for recorded AVI files
	CString	m_RecBmpFolder;		// folder for recorded bitmap sequences

// Operations

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFracticeApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual void WinHelp(DWORD dwData, UINT nCmd = HELP_CONTEXT);
	virtual BOOL IsIdleMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CFracticeApp)
	afx_msg void OnAppAbout();
	afx_msg void OnFileOpen();
	afx_msg void OnAppHomePage();
	afx_msg void OnHelpIndex();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
// Data members
	UINT	m_CPUCount;			// number of processors in system
	bool	m_bHaveSSE2;		// true if CPU supports SSE2
	bool	m_bExitWindows;		// if true, exit windows from ExitInstance
	UINT	m_uExitFlags;		// flags to pass to ExitWindowsEx
	DWORD	m_HelpCookie;		// for HTMLHelp API
	int		m_DocFilterIndex;	// file open dialog's filter index
};

inline CMainFrame *CFracticeApp::GetMain()
{
	return((CMainFrame *)m_pMainWnd);
}

inline UINT CFracticeApp::GetCPUCount() const
{
	return(m_CPUCount);
}

inline bool CFracticeApp::HaveSSE2() const
{
	return(m_bHaveSSE2);
}

extern CFracticeApp theApp;

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FRACTICE_H__ED0ED84D_AA43_442C_85CD_A7FA518EBF90__INCLUDED_)
