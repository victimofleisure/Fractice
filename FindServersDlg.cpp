// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda

		rev		date		comments
        00      14feb08	initial version
		01		12feb09	in FindServers, add HostIP

		find server dialog

*/

// FindServersDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Fractice.h"
#include "FindServersDlg.h"
#include "Packets.h"
#include "ProgressDlg.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFindServersDlg dialog

IMPLEMENT_DYNAMIC(CFindServersDlg, CDialog);

#define	RK_METHOD			_T("FSrvMethod")
#define	RK_RANGE_START		_T("FSrvRangeStart")
#define	RK_RANGE_END		_T("FSrvRangeEnd")

CFindServersDlg::CFindServersDlg(WORD PortNumber, CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
	//{{AFX_DATA_INIT(CFindServersDlg)
	//}}AFX_DATA_INIT
	m_PortNumber = PortNumber;
	m_Finder = NULL;
	m_Kill = FALSE;
	m_Method = theApp.RdRegInt(RK_METHOD, FSM_BROADCAST);
	m_RangeStart = theApp.RdRegString(RK_RANGE_START);
	m_RangeEnd = theApp.RdRegString(RK_RANGE_END);
}

CFindServersDlg::~CFindServersDlg()
{
	theApp.WrRegInt(RK_METHOD, m_Method);
	theApp.WrRegString(RK_RANGE_START, m_RangeStart);
	theApp.WrRegString(RK_RANGE_END, m_RangeEnd);
}

bool CFindServersDlg::GetIPAddrRange(LPCTSTR StartStr, LPCTSTR EndStr, UINT& StartIP, UINT& EndIP)
{
	UINT	start = _tinet_addr(StartStr);
	UINT	end = _tinet_addr(EndStr);
	if (!start || !end || start == INADDR_NONE || end == INADDR_NONE)
		return(FALSE);
	start = htonl(start);
	end = htonl(end);
	if (start >= end)
		return(FALSE);
	StartIP = start;
	EndIP = end;
	return(TRUE);
}

bool CFindServersDlg::IsBroadcastAddr(DWORD Addr)
{
	for (int i = 0; i < sizeof(DWORD); i++) {	// look for 255 in any byte
		if (((BYTE *)&Addr)[i] == 255)
			return(TRUE);
	}
	return(FALSE);
}

CString CFindServersDlg::GetHostByAddr(const in_addr& Addr)
{
	CString	s;
	HOSTENT	*he = gethostbyaddr((char *)&Addr,
		sizeof(in_addr), AF_INET);	// try getting host entry
	// if host entry was found and contains a valid host name
	if (he != NULL && he->h_name != NULL && he->h_name[0]) {
		s = he->h_name;	// show host name
		s.MakeUpper();
	} else	// get host name failed
		s = inet_ntoa(Addr);	// show dotted IP instead
	return(s);
}

void CFindServersDlg::AddServer(const WFindServers::SERVER_INFO& Info, CServerInfoArray& List)
{
	CServerInfo	si;
	si.m_HostName = Info.Name;
	si.m_HostName.MakeUpper();
	si.m_IPAddress = Info.IPAddress;
	si.m_BinIPAddr = ntohl(inet_addr(Info.IPAddress));
	si.m_Status = Info.Status;
	if (si.m_Status) {
		si.m_ClientHostName = GetHostByAddr(Info.ExtStatus.ClientIPAddress);
		si.m_ClientBinIPAddr = ntohl(Info.ExtStatus.ClientIPAddress.S_un.S_addr);
		si.m_ClientIPAddress = inet_ntoa(Info.ExtStatus.ClientIPAddress);
	}
	List.Add(si);
}

bool CFindServersDlg::FindServersBroadcast()
{
	WFindServers::SERVER_INFO	Info;
	int	i = 0;
	while (m_Finder->Find(i, Info)) {	// broadcast first time only
		AddServer(Info, m_ServerList);
		i++;
	}
	return(TRUE);
}

bool CFindServersDlg::RcvFunc()
{
	WFindServers::SERVER_INFO	Info;
	while (!m_Kill) {
		if (m_Finder->Find(TRUE, Info))	// don't broadcast
			AddServer(Info, m_ServerList);
	}
	SetEvent(m_Done);
	return(TRUE);
}

UINT CFindServersDlg::RcvThread(LPVOID pParam)
{
	CFindServersDlg	*This = (CFindServersDlg *)pParam;
	This->RcvFunc();
	return(0);
}

bool CFindServersDlg::FindServersAddressRange()
{
	UINT	start, end;
	if (!GetIPAddrRange(m_RangeStart, m_RangeEnd, start, end)) {
		AfxMessageBox(IDS_FSRV_BAD_IP_RANGE);
		return(FALSE);
	}
	if (!m_Done.Create(NULL, FALSE, FALSE, NULL))
		return(FALSE);
	m_Kill = FALSE;
	AfxBeginThread(RcvThread, this);
	bool	retc = TRUE;
	try {
		CProgressDlg	dlg;
		dlg.Create();
		dlg.SetRange(start, end);
		dlg.SetWindowText(LDS(IDS_FSRV_PROGRESS_CAP));
		for (UINT i = start; i <= end; i++) {
			dlg.SetPos(i);
			if (!IsBroadcastAddr(i)) {	// exclude broadcast address
				SOCKADDR_IN	addr;
				addr.sin_family = AF_INET;
				addr.sin_port = htons(m_PortNumber);
				addr.sin_addr.s_addr = ntohl(i);
				m_Finder->RequestReply(addr);	// request reply
			} 
			if (dlg.Canceled()) {
				retc = FALSE;
				break;
			}
		}
		CWaitCursor	wc;
		if (retc)
			Sleep(1000);	// give last replies a chance to come in
		m_Kill = TRUE;
		WaitForSingleObject(m_Done, INFINITE);
	}
	catch (const WError& e) {
		CString	msg(e.what());
		AfxMessageBox(msg);
		return(FALSE);
	}
	return(retc);
}

bool CFindServersDlg::FindServers()
{
	m_ServerList.RemoveAll();
	CMainFrame::CStatusMsg	status(IDS_FSRV_FINDING);
	bool	retc;
	try {
		COptionsDlg&	OptsDlg = theApp.GetMain()->GetOptionsDlg();
		LPCTSTR	HostIP = OptsDlg.GetNicIPAddressOrNull();
#ifdef UNICODE
		USES_CONVERSION;
		m_Finder = new WFindServers(NULL, m_PortNumber, W2CA(HostIP));
#else
		m_Finder = new WFindServers(NULL, m_PortNumber, HostIP);
#endif
		switch (m_Method) {
		case FSM_BROADCAST:
			retc = FindServersBroadcast();
			break;
		case FSM_ADDR_RANGE:
			retc = FindServersAddressRange();
			break;
		default:
			ASSERT(0);	// logic error
			retc = 0;
		}
		delete m_Finder;
	}
	catch (const WError& e) {
		CString	msg(e.what());
		AfxMessageBox(msg);
		return(FALSE);
	}
	return(retc);
}

void CFindServersDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFindServersDlg)
	DDX_Text(pDX, IDC_FSRV_RANGE_END, m_RangeEnd);
	DDX_Text(pDX, IDC_FSRV_RANGE_START, m_RangeStart);
	DDX_Radio(pDX, IDC_FSRV_METHOD, m_Method);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFindServersDlg, CDialog)
	//{{AFX_MSG_MAP(CFindServersDlg)
	ON_BN_CLICKED(IDC_FSRV_METHOD, OnMethod)
	ON_BN_CLICKED(IDC_FSRV_METHOD2, OnMethod)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFindServersDlg message handlers

BOOL CFindServersDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	OnMethod();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFindServersDlg::OnMethod() 
{
	UpdateData();
	GetDlgItem(IDC_FSRV_RANGE_START)->EnableWindow(m_Method);
	GetDlgItem(IDC_FSRV_RANGE_END)->EnableWindow(m_Method);
}

void CFindServersDlg::OnOK() 
{
	UpdateData();
	if (m_Method) {
		UINT	start, end;
		if (!CFindServersDlg::GetIPAddrRange(m_RangeStart, m_RangeEnd, start, end)) {
			AfxMessageBox(IDS_FSRV_BAD_IP_RANGE);
			GotoDlgCtrl(GetDlgItem(IDC_FSRV_RANGE_START));
			return;
		}
	}
	FindServers();
	CDialog::OnOK();
}
