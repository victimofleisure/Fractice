// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda

		rev		date		comments
        00      14feb08	initial version

		find server dialog

*/

#if !defined(AFX_FINDSERVERSDLG_H__99AA6B09_36EF_43DD_83CE_426F72015E99__INCLUDED_)
#define AFX_FINDSERVERSDLG_H__99AA6B09_36EF_43DD_83CE_426F72015E99__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FindServersDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFindServersDlg dialog

#include "ServerInfo.h"
#include "FindServers.h"
#include "Event.h"

class WFindServers;

class CFindServersDlg : public CDialog
{
	DECLARE_DYNAMIC(CFindServersDlg);
// Construction
public:
	CFindServersDlg(WORD PortNumber, CWnd* pParent = NULL);
	~CFindServersDlg();

// Attributes
	void	GetServerList(CServerInfoArray&	List) const;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFindServersDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CFindServersDlg)
	enum { IDD = IDD_FIND_SERVERS };
	int		m_Method;
	CString	m_RangeStart;
	CString	m_RangeEnd;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CFindServersDlg)
	afx_msg void OnMethod();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Constants
	enum {	// find server methods
		FSM_BROADCAST,			// UDP broadcast; works on subnet only
		FSM_ADDR_RANGE,			// iterate over range of IP addresses
	};

// Data members
	WORD	m_PortNumber;		// network port number for UDP client
	WFindServers	*m_Finder;	// finds servers on network via UDP
	bool	m_Kill;				// true if receive thread should exit
	WEvent	m_Done;				// set when receive thread exits
	CServerInfoArray	m_ServerList;	// list of found servers

// Helpers
	bool	FindServers();
	bool	FindServersBroadcast();
	bool	FindServersAddressRange();
	bool	RcvFunc();
	static	UINT	RcvThread(LPVOID pParam);
	static	bool	IsBroadcastAddr(DWORD Addr);
	static	bool	GetIPAddrRange(LPCTSTR StartStr, LPCTSTR EndStr, UINT& StartIP, UINT& EndIP);
	static	CString GetHostByAddr(const in_addr& Addr);
	static	void	AddServer(const WFindServers::SERVER_INFO& Info, CServerInfoArray& List);
};

inline void CFindServersDlg::GetServerList(CServerInfoArray& List) const
{
	List.Copy(m_ServerList);
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FINDSERVERSDLG_H__99AA6B09_36EF_43DD_83CE_426F72015E99__INCLUDED_)
