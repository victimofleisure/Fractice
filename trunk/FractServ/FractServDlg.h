// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		10jan09	move GetBounds to CSnapshot

        fractal rendering server dialog
 
*/

// FractServDlg.h : header file
//

#if !defined(AFX_FRACTICESERVERDLG_H__0C8B5FF5_AEA7_46E1_A68D_D2C62D52414A__INCLUDED_)
#define AFX_FRACTICESERVERDLG_H__0C8B5FF5_AEA7_46E1_A68D_D2C62D52414A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CFractServDlg dialog

#include "StripEngine.h"
#include "MySnap.h"
#include "Packets.h"
#include "FindServers.h"
#include "Benchmark.h"
#include "TimerMsgThread.h"

class WFindServers;

class CFractServDlg : public WObject
{
// Construction
public:
	CFractServDlg(HWND hWnd);
	~CFractServDlg();

// Operations
	bool	DoModal();

// Implementation
protected:
// Message handlers
	void	OnRenderDone(UINT FrameUID);
	void	OnStripDone(UINT FrameUID, UINT ThreadIdx);
	void	OnRcvPacket(GENERIC_PACKET *pPacket, int Cookie);
	void	OnTcpConnect(BOOL Connected, int Cookie);
	void	OnTimer(UINT nIDEvent);

// Types
	typedef struct tagSTRIP_INFO {
		UINT	y1;			// first row to render
		UINT	y2;			// last row to render, plus one
	} STRIP_INFO;
	typedef	CArrayEx<STRIP_INFO, STRIP_INFO&> CStripInfoArray;
	class CThreadInfo : public WObject {
	public:
		CStripInfoArray	m_StripQueue;	// strips waiting to be rendered
		bool	m_Rendering;	// true if thread is rendering
	};
	typedef	CArrayEx<CThreadInfo, CThreadInfo&> CThreadInfoArray;

// Constants
	enum {
		PROGRESS_TIMER = 1,		// progress timer's event ID
	};
	static const UINT m_ExitFlags[SSDA_ACTIONS];	// ExitWindowEx flags

// Dialog data
	HICON	m_hIcon;

// Data members
	HWND	m_hWnd;				// thread ID of message-handling thread
	CStripEngine	m_Engine;	// fractal rendering engine
	UINT	m_FrameUID;			// unique frame identifier
	CSize	m_ImageSize;		// image dimensions, in pixels
	UINT	m_Antialias;		// oversampling factor; 1 = no antialiasing
	UINT	m_MathPrec;			// floating-point precision, in bits
	UINT	m_CPUCount;			// number of CPUs we have
	UINT	m_ThreadCount;		// number of rendering threads we have
	CBenchmark	m_RenderTimer;	// measures rendering throughput
	bool	m_Rendering;		// true if we're rendering
	CMySnapPtr	m_Snap;			// pointer to current snapshot
	CMySnapList	m_SnapQueue;	// snapshots waiting to be rendered
	CPackets	m_Packets;		// network interface
	WFindServers	*m_FindServer;	// UDP server so client can find us
	CTimerMsgThread	m_ProgressTimer;	// timer to trigger progress reports
	int		m_Progress;			// percentage of current frame completed
	CEscTimeArray	m_Strip;	// strip destination buffer
	int		m_StripSize;		// size of a strip in elements
	CThreadInfoArray	m_ThreadInfo;	// info about each rendering thread

// Helpers
	bool	Create();
	void	Destroy();
	void	SetMathPrec(UINT Bits);
	bool	SetImageSize(CSize Size, UINT Antialias);
	void	SetSnapshot(CMySnapPtr& Snap);
	void	RenderFrame();
	void	BeginStripJob(CMySnapPtr& Snap, UINT StripRows);
	void	CancelRender();
	void	RenderStrip(UINT ThreadIdx, UINT y1, UINT y2);
	static	DWORD	FindServerCallback(LPCSTR ClientAddress, void *UserData, WFindServers::EXT_STATUS& ExtStatus);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FRACTICESERVERDLG_H__0C8B5FF5_AEA7_46E1_A68D_D2C62D52414A__INCLUDED_)
