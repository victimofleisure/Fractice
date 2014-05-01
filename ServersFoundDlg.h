// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version

        server selection dialog
 
*/

#if !defined(AFX_SERVERSFOUNDDLG_H__629190CE_9688_49E5_8313_CC6EFDE4D262__INCLUDED_)
#define AFX_SERVERSFOUNDDLG_H__629190CE_9688_49E5_8313_CC6EFDE4D262__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ServersFoundDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CServersFoundDlg dialog

#include "ServerInfo.h"
#include "ReportCtrl.h"
#include "CtrlResize.h"

class CServersFoundDlg : public CDialog
{
	DECLARE_DYNAMIC(CServersFoundDlg);
// Construction
public:
	CServersFoundDlg(CWnd* pParent = NULL);   // standard constructor

// Attributes
	void	SetServerInfo(const CServerInfoArray& Server);
	void	GetServerInfo(CServerInfoArray& Server) const;
	void	GetSelectedServers(CServerInfoArray& Server) const;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CServersFoundDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CServersFoundDlg)
	enum { IDD = IDD_SERVERS_FOUND };
	CButton	m_ShowAvailOnlyChk;
	CButton	m_CancelBtn;
	CButton	m_OKBtn;
	CReportCtrl	m_List;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CServersFoundDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnGetdispinfoList(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnOK();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnShowAvailOnly();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Types
	typedef	CReportCtrl::RES_COL COL_INFO;

// Constants
	enum {	// columns
		COL_HOST_NAME,
		COL_IP_ADDRESS,
		COL_CLIENT_HOST_NAME,
		COL_CLIENT_IP_ADDRESS,
		COLUMNS
	};
	static const COL_INFO	m_ColInfo[COLUMNS];
	static const CCtrlResize::CTRL_LIST	m_CtrlList[];

// Data members
	CCtrlResize	m_Resize;		// control resizer
	CRect	m_InitRect;			// initial rectangle in screen coords
	CServerInfoArray	m_Server;	// array of servers to choose
	CServerInfoArray	m_SelServ;	// array of selected servers

// Helpers
	void	UpdateList();
	void	SelectAllAvail();
	static	int	CALLBACK SortCompare(LPARAM p1, LPARAM p2, LPARAM This);
	int		SortCompare(int p1, int p2);
};

inline void CServersFoundDlg::SetServerInfo(const CServerInfoArray& Server)
{
	m_Server.Copy(Server);
}

inline void CServersFoundDlg::GetServerInfo(CServerInfoArray& Server) const
{
	Server.Copy(m_Server);
}

inline void CServersFoundDlg::GetSelectedServers(CServerInfoArray& Server) const
{
	Server.Copy(m_SelServ);
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SERVERSFOUNDDLG_H__629190CE_9688_49E5_8313_CC6EFDE4D262__INCLUDED_)
