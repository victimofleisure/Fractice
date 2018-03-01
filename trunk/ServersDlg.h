// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		11dec09	make header state persistent
		02		12dec09	add done and queue columns
		03		18dec09	use FastIsVisible
		04		25dec09	in OnStop, add Aborted argument
		05		27dec09	add m_LastRecordUID
		06		06jan10	W64: make OnTimer 64-bit compatible
		07		09jan10	add context menu and properties

        distributed rendering server dialog
 
*/

#if !defined(AFX_SERVERDLG_H__2560F48A_D3D5_4023_A5FF_4F3C9E9131A2__INCLUDED_)
#define AFX_SERVERDLG_H__2560F48A_D3D5_4023_A5FF_4F3C9E9131A2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ServersDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CServersDlg dialog

#include "ToolDlg.h"
#include "Packets.h"
#include "ReportCtrl.h"
#include "ServerInfo.h"
#include "CtrlResize.h"
#include "Server.h"
#include "WndTimer.h"

class CMainFrame;
class CFracticeView;
class CNetEngine;

class CServersDlg : public CToolDlg
{
	DECLARE_DYNAMIC(CServersDlg);
// Construction
public:
	CServersDlg(CWnd* pParent = NULL);   // standard constructor

// Constants
	// also add new columns to m_ColInfo, SortCompare, and OnGetdispinfoList
	enum {	// columns
		COL_HOST_NAME,
		COL_IP_ADDRESS,
		COL_THREAD_COUNT,
		COL_RENDER_TIME,
		COL_PROGRESS,
		COL_DONE,
		COL_QUEUE,
		COLUMNS,
	};
	enum {
		LOCAL_HOST = -1,	// special server index for local host
		DYNAMIC_COL = -1,	// for RefreshRow's Column argument
	};

// Attributes
	void	SetView(CFracticeView *View);
	int		GetServerCount() const;	// number of servers in server array
	CServer&	GetServer(int ServerIdx);
	int		GetConnectionCount() const;	// number of servers we're connected to
	CServer&	GetConnection(int ConnIdx);
	CString	GetHostName(int ServerIdx) const;
	CString	GetIPAddress(int ServerIdx) const;
	bool	GetShowProgress() const;
	void	SetShowProgress(bool Enable);
	CServer&	GetLocalHost();
	void	SetLocalRenderTime(double RenderTime);
	void	SetLocalThreadCount(UINT ThreadCount);
	CNetEngine	*GetLocalEngine();

// Operations
	void	UpdateList();
	void	RefreshRow(int ServerIdx, int Column);
	void	RefreshAll();
	bool	AddServer(LPCTSTR HostName, LPCTSTR IPAddress = NULL, CString *ErrorMsg = NULL, bool DeferUpdate = FALSE, WTcp *Tcp = NULL);
	void	AddServers(CServerInfoArray& ServInfo);
	void	PostMsgToAllServers(UINT Message, WPARAM wParam = 0, LPARAM lParam = 0);
	void	ResetStatistics();
	int		FindConnection(LPCTSTR HostName, UINT BinIPAddr) const;
	void	OnRecord();
	void	OnStop(bool Aborted);
	void	OnFrameDone(int ServerIdx, int FrameID, double RenderTime);
	void	OnBeginStripJob();
	void	OnEndStripJob();
	void	UpdateProgress();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CServersDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CServersDlg)
	enum { IDD = IDD_SERVERS };
	CButton	m_ShowProgressChk;
	CButton	m_AddListBtn;
	CButton	m_ShutdownBtn;
	CButton	m_FindBtn;
	CButton	m_DeleteBtn;
	CButton	m_AddBtn;
	CReportCtrl	m_List;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CServersDlg)
	afx_msg void OnFind();
	virtual BOOL OnInitDialog();
	afx_msg void OnDelete();
	afx_msg void OnItemchangedList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnGetdispinfoList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnAdd();
	afx_msg void OnShutdown();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSrvAddList();
	afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnShowProgress();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(W64UINT nIDEvent);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnResetCols();
	afx_msg void OnProperties();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Types
	typedef CRefPtr<CServer> CServerPtr;
	typedef CArrayEx<CServerPtr, CServerPtr&> CServerArray;
	typedef	CReportCtrl::RES_COL COL_INFO;

// Constants
	enum {
		QUERY_TIMEOUT = 3000,	// interval within which server must reply to
								// initial capabilities query, in milliseconds
		PROGRESS_TIMER = 1,		// progress timer event ID
		PROGRESS_PERIOD = 1000,	// interval between progress reports, in millis
	};
	static const COL_INFO	m_ColInfo[COLUMNS];	// list column data
	static const CCtrlResize::CTRL_LIST	m_CtrlList[];	// resizeable controls
	static const UINT	m_DynColMask;	// columns that contain dynamic data

// Data members
	CMainFrame	*m_Main;		// pointer to main frame window
	CFracticeView	*m_View;	// pointer to current view
	CServerArray	m_Server;	// array of servers
	CServer	m_LocalHost;		// proxy server for local machine
	CServerArray	m_ConnServ;	// array of connected servers
	CCtrlResize	m_Resize;		// control resizer
	CRect	m_InitRect;			// initial rectangle in screen coords
	bool	m_ShowProgress;		// if true, show progress bars
	CBrush	m_ProgressBarBrush;	// brush for drawing progress bar
	CWndTimer	m_ProgressTimer;	// timer for view progress bar
	CString	m_ComputerName;		// current system name, from registry
	UINT	m_LastRecordUID;	// most recent recording's last local UID

// Helpers
	int		SortCompare(int p1, int p2);
	static	int	CALLBACK SortCompare(LPARAM p1, LPARAM p2, LPARAM This);
	bool	Connect(int ServerIdx, CPackets& Packets, LPCTSTR HostName, FRAP_CAPS_REPLY& Caps, CString *ErrorMsg, WTcp *Tcp);
	bool	ReadServerName(int ServerIdx, CPackets& Packets, CString& Name);
	void	HandleConnectError(LPCTSTR ErrorStr, LPCTSTR HostName, CString *DestStr);
	void	HandleConnectError(int ErrorID, LPCTSTR HostName, CString *DestStr);
	void	RemoveAllPending();
	bool	IsDynamicSort() const;
	bool	GetExCaps(CPackets& Packets, int ServerIdx, FRAP_EX_CAPS_REPLY& ExCaps);
	static	CString	GetOSVersion(const OSVERSIONINFOEX& vi);
	static	CString	GetOSTitle(const OSVERSIONINFOEX& vi);
};

inline int CServersDlg::GetServerCount() const
{
	return(m_Server.GetSize());
}

inline CServer& CServersDlg::GetServer(int ServerIdx)
{
	return(m_Server[ServerIdx]);
}

inline int CServersDlg::GetConnectionCount() const
{
	return(m_ConnServ.GetSize());
}

inline CServer& CServersDlg::GetConnection(int ConnIdx)
{
	return(m_ConnServ[ConnIdx]);
}

inline CString CServersDlg::GetHostName(int ServerIdx) const
{
	return(m_Server[ServerIdx]->m_HostName);
}

inline CString CServersDlg::GetIPAddress(int ServerIdx) const
{
	return(m_Server[ServerIdx]->m_IPAddress);
}

inline bool	CServersDlg::GetShowProgress() const
{
	return(m_ShowProgress);
}

inline CServer& CServersDlg::GetLocalHost()
{
	return(m_LocalHost);
}

inline void CServersDlg::SetLocalRenderTime(double RenderTime)
{
	m_LocalHost.m_RenderTime = RenderTime;
	if (FastIsVisible())
		RefreshRow(LOCAL_HOST, COL_RENDER_TIME);
}

inline void CServersDlg::SetLocalThreadCount(UINT ThreadCount)
{
	m_LocalHost.m_Threads = ThreadCount;
	if (FastIsVisible())
		RefreshRow(LOCAL_HOST, COL_THREAD_COUNT);
}

inline bool CServersDlg::IsDynamicSort() const
{
	return(((1 << m_List.SortCol()) & m_DynColMask) != 0);
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SERVERDLG_H__2560F48A_D3D5_4023_A5FF_4F3C9E9131A2__INCLUDED_)
