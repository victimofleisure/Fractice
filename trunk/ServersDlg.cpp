// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		07jan09	in OnEndStripJob, invalidate list instead of dialog
		02		03feb09	in OnGetMinMaxInfo, verify initial rect
		03		12feb09	add null server IP argument to packets create
		04		21mar09	in OnSysCommand, mask off unreliable low nibble
		05		11dec09	make header state persistent
		06		12dec09	add done and queue columns
		07		17dec09	use engine's GetLocalRowsDone
		08		18dec09	use FastIsVisible
		09		25dec09	in OnStop, if aborted, post cancel to servers
		10		27dec09	more work on done column
		11		06jan10	W64: make OnTimer 64-bit compatible
		12		06jan10	W64: cast item indices to 32-bit
		13		09jan10	add context menu and properties

        distributed rendering server dialog
 
*/

// ServersDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Fractice.h"
#include "ServersDlg.h"
#include "FracticeDoc.h"
#include "FracticeView.h"
#include "MainFrm.h"
#include "FindServersDlg.h"
#include "AddServerDlg.h"
#include "AddServerListDlg.h"
#include "ServerShutdownDlg.h"
#include "ServersFoundDlg.h"
#include "PosterProgressDlg.h"
#include "MsgBoxDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CServersDlg dialog

IMPLEMENT_DYNAMIC(CServersDlg, CToolDlg);

const CServersDlg::COL_INFO CServersDlg::m_ColInfo[COLUMNS] = {
	{IDS_SRV_COL_HOST_NAME,		LVCFMT_LEFT,	120,	CReportCtrl::DIR_ASC},
	{IDS_SRV_COL_IP_ADDR,		LVCFMT_LEFT,	100,	CReportCtrl::DIR_ASC},
	{IDS_SRV_COL_THREAD_COUNT,	LVCFMT_RIGHT,	55,		CReportCtrl::DIR_ASC},
	{IDS_SRV_COL_RENDER_TIME,	LVCFMT_RIGHT,	80,		CReportCtrl::DIR_ASC},
	{IDS_SRV_COL_PROGRESS,		LVCFMT_LEFT,	100,	CReportCtrl::DIR_ASC},
	{IDS_SRV_COL_DONE,			LVCFMT_LEFT,	50,		CReportCtrl::DIR_ASC},
	{IDS_SRV_COL_QUEUE,			LVCFMT_LEFT,	50,		CReportCtrl::DIR_ASC},
};

const CCtrlResize::CTRL_LIST CServersDlg::m_CtrlList[] = {
	{IDC_SRV_LIST,			BIND_ALL},
	{IDC_SRV_FIND,			BIND_RIGHT | BIND_TOP},
	{IDC_SRV_ADD,			BIND_RIGHT | BIND_TOP},
	{IDC_SRV_ADD_LIST,		BIND_RIGHT | BIND_TOP},
	{IDC_SRV_DELETE,		BIND_RIGHT | BIND_TOP},
	{IDC_SRV_SHUTDOWN,		BIND_RIGHT | BIND_TOP},
	{IDC_SRV_SHOW_PROGRESS,	BIND_RIGHT | BIND_TOP},
	{0, 0}	// list terminator
};

const UINT CServersDlg::m_DynColMask =	// bitmask for dynamic columns
	(1 << COL_RENDER_TIME) |
	(1 << COL_PROGRESS) |
	(1 << COL_DONE) |
	(1 << COL_QUEUE);

#define RK_HEADER_STATE	_T("ServersDlgHdrState")

CServersDlg::CServersDlg(CWnd* pParent /*=NULL*/)
	: CToolDlg(IDD, IDR_MAINFRAME, _T("ServersDlg"), pParent)
{
	//{{AFX_DATA_INIT(CServersDlg)
	//}}AFX_DATA_INIT
	m_Main = NULL;
	m_View = NULL;
	m_ShowProgress = FALSE;
	m_InitRect.SetRectEmpty();
	m_LastRecordUID = 0;
}

void CServersDlg::SetView(CFracticeView *View)
{
	if (View == m_View)
		return;	// nothing to do
	m_View = View;
	if (View != NULL) {
		if (!m_Main->IsRecording())
			SetLocalRenderTime(View->GetLastRenderTime());
		if (m_ShowProgress && FastIsVisible())
			UpdateProgress();
	}
}

void CServersDlg::UpdateList()
{
	int	servs = GetServerCount();
	int	rows = 0;
	m_List.DeleteAllItems();
	m_ConnServ.RemoveAll();
	for (int i = 0; i < servs; i++) {
		if (m_Server[i]->IsConnected())	{	// only show servers we're connected to
			m_List.InsertCallbackRow(rows++, i);
			m_ConnServ.Add(m_Server[i]);
		}
	}
	m_List.InsertCallbackRow(rows++, LOCAL_HOST);
	m_List.SortRows();
}

void CServersDlg::RefreshRow(int ServerIdx, int Column)
{
	// if list is sorted by specified column, or dynamic data
	// is being updated and list is sorted by a dynamic column
	if (Column == m_List.SortCol()
	|| (Column == DYNAMIC_COL && IsDynamicSort()))
		m_List.SortRows();	// resort server list
	else {
		int	row = m_List.FindRow(ServerIdx);
		if (row >= 0)
			m_List.RedrawItems(row, row);	// redraw server's row
	}
}

void CServersDlg::RefreshAll()
{
	if (IsDynamicSort())
		m_List.SortRows();
	else
		m_List.Invalidate();
}

#define SORT_CMP(x) retc = m_List.SortCmp(srv1.x, srv2.x);

int CServersDlg::SortCompare(int p1, int p2)
{
	const CServer&	srv1 = p1 < 0 ? m_LocalHost : m_Server[p1];
	const CServer&	srv2 = p2 < 0 ? m_LocalHost : m_Server[p2];
	int	retc;
	switch (m_List.SortCol()) {
	case COL_HOST_NAME:
		SORT_CMP(m_HostName);
		break;
	case COL_IP_ADDRESS:
		SORT_CMP(m_BinIPAddr);
		break;
	case COL_THREAD_COUNT:
		SORT_CMP(m_Threads);
		break;
	case COL_RENDER_TIME:
		SORT_CMP(m_RenderTime);
		break;
	case COL_PROGRESS:
		SORT_CMP(m_Progress);
		break;
	case COL_DONE:
		SORT_CMP(GetRenderedCount());
		break;
	case COL_QUEUE:
		SORT_CMP(GetPendingCount());
		break;
	default:
		ASSERT(0);	// logic error
		retc = 0;
	}
	return(retc);
}

int CALLBACK CServersDlg::SortCompare(LPARAM p1, LPARAM p2, LPARAM This)
{
	return(((CServersDlg *)This)->SortCompare(INT64TO32(p1), INT64TO32(p2)));
}

void CServersDlg::HandleConnectError(LPCTSTR ErrorStr, LPCTSTR HostName, CString *DestStr)
{
	CString	s;
	AfxFormatString1(s, IDS_SRV_CANT_CONNECT, HostName);
	s += CString("\n") + ErrorStr;
	if (DestStr != NULL)
		*DestStr = s;
	else
		AfxMessageBox(s);
}

void CServersDlg::HandleConnectError(int ErrorID, LPCTSTR HostName, CString *DestStr)
{
	CString	msg;
	msg.LoadString(ErrorID);
	HandleConnectError(msg, HostName, DestStr);
}

bool CServersDlg::GetExCaps(CPackets& Packets, int ServerIdx, FRAP_EX_CAPS_REPLY& ExCaps)
{
	bool	retc = FALSE;	// assume failure
	if (Packets.WriteMessage(PMID_EX_CAPS_QUERY)) {
		GENERIC_PACKET	*p = Packets.Read(PMID_EX_CAPS_REPLY, ServerIdx, QUERY_TIMEOUT);
		if (p != NULL) {
			FRAP_EX_CAPS_REPLY *pecr = (FRAP_EX_CAPS_REPLY *)p;
			ExCaps = *pecr;
			retc = TRUE;
			delete p;
		}
	}
	return(retc);
}

bool CServersDlg::Connect(int ServerIdx, CPackets& Packets, LPCTSTR HostName, FRAP_CAPS_REPLY& Caps, CString *ErrorMsg, WTcp *Tcp)
{
	bool	retc = FALSE;	// assume failure
	CString	msg;
	WORD	PortNumber = m_Main->GetOptionsDlg().GetNetworkPort();
	if (Packets.Create(HostName, PortNumber, NULL, m_Main->m_hWnd, ServerIdx, Tcp)) {
		if (Packets.WriteCapsQuery()) {
			GENERIC_PACKET	*p = Packets.Read(PMID_CAPS_REPLY, 
				ServerIdx, QUERY_TIMEOUT);
			if (p != NULL) {	// caps reply was received
				FRAP_CAPS_REPLY *pcr = (FRAP_CAPS_REPLY *)p;
				if (pcr->Signature == FRACTICE_NET_SIGNATURE) {
					if (pcr->Version == FRACTICE_NET_VERSION) {
						if (pcr->SnapVersion == CSnapshot::ARCHIVE_VERSION) {
							Caps = *pcr;
							retc = TRUE;	// success
						} else
							msg.LoadString(IDS_SRV_BAD_SNAP_FORMAT);
					} else
						msg.LoadString(IDS_SRV_BAD_VERSION);
				} else
					msg.LoadString(IDS_SRV_BAD_SERVER);
				delete p;	// delete the packet
			}
		}
	} else
		msg = Packets.GetLastErrorString();
	if (!retc) {
		if (msg.IsEmpty())
			msg.LoadString(IDS_SRV_TIMEOUT);
		HandleConnectError(msg, HostName, ErrorMsg);
	}
	return(retc);
}

bool CServersDlg::ReadServerName(int ServerIdx, CPackets& Packets, CString& Name)
{
	if (!Packets.WriteMessage(PMID_HOST_NAME_QUERY))
		return(FALSE);
	GENERIC_PACKET	*pp = Packets.Read(PMID_HOST_NAME_REPLY, ServerIdx);
	if (pp == NULL)
		return(FALSE);
	Name = (LPCSTR)&pp->Data;
	delete pp;	// we're responsible for deleting packet
	return(TRUE);
}

int CServersDlg::FindConnection(LPCTSTR HostName, UINT BinIPAddr) const
{
	int	conns = GetConnectionCount();
	for (int i = 0; i < conns ; i++) {
		const CServer&	srv = m_ConnServ[i];
		if (!srv.m_HostName.CompareNoCase(HostName) && srv.m_BinIPAddr == BinIPAddr)
			return(i);
	}
	return(-1);
}

bool CServersDlg::AddServer(LPCTSTR HostName, LPCTSTR IPAddress, CString *ErrorMsg, bool DeferUpdate, WTcp *Tcp)
{
	ASSERT(HostName != NULL);
	CServerPtr	srv;
	srv.CreateObj();
	int	ServerIdx = GetServerCount();
	// connect via IP address if available, because it's faster
	LPCTSTR	target = IPAddress != NULL ? IPAddress : HostName;
	FRAP_CAPS_REPLY	caps;
	if (!Connect(ServerIdx, srv, target, caps, ErrorMsg, Tcp))
		return(FALSE);	// connect error was handled and TCP object was deleted
	// if TCP object was passed in, CServer now owns it and handles deletion
	CString	ServName, IPAddrStr;
	if (IPAddress == NULL) {	// HostName could be a dotted IP address
		SOCKADDR_IN	addr = srv->GetRemoteAddr();	// get address from socket
		IPAddrStr = inet_ntoa(addr.sin_addr);		// convert to dotted IP
		IPAddress = IPAddrStr;	// IPAddress arg now points to IPAddrStr
		if (_tinet_addr(HostName) != INADDR_NONE) {	// if HostName is dotted IP
			if (ReadServerName(ServerIdx, srv, ServName))	// get name from server
				HostName = ServName;	// HostName arg now points to ServName
		}
	}
	UINT	BinIPAddr = ntohl(_tinet_addr(IPAddress));	// binary IP in host order
	// if TCP object was passed in, verify connection isn't a duplicate
	if (Tcp != NULL && FindConnection(HostName, BinIPAddr) >= 0) {
		HandleConnectError(IDS_SRV_ALREADY_CONNECTED, HostName, ErrorMsg);
		return(FALSE);
	}
	if (m_ShowProgress)	// if we're showing progress, request reports
		srv->WriteMessage(PMID_PROGRESS_CONFIG, PROGRESS_PERIOD);
	srv->m_HostName = HostName;
	srv->m_HostName.MakeUpper();
	srv->m_IPAddress = IPAddress;
	srv->m_BinIPAddr = BinIPAddr;
	srv->m_CPUCount = caps.CPUCount;
	srv->m_Threads = caps.Threads;
	srv->m_Options = caps.Options;
	m_Server.Add(srv);	// add connected server to server array
	if (!DeferUpdate)
		UpdateList();
	return(TRUE);
}

void CServersDlg::AddServers(CServerInfoArray& ServInfo)
{
	int	servs = ServInfo.GetSize();
	for (int i = 0; i < servs; i++) {
		CServerInfo	si = ServInfo[i];
		if (!si.m_Status)	// if server isn't already connected
			AddServer(si.m_HostName, si.m_IPAddress, NULL, TRUE);	// defer update
	}
	UpdateList();	// update was deferred within loop
}

void CServersDlg::PostMsgToAllServers(UINT Message, WPARAM wParam, LPARAM lParam)
{
	int	conns = GetConnectionCount();
	for (int i = 0; i < conns; i++)
		m_ConnServ[i]->WriteMessage(Message, wParam, lParam);
}

void CServersDlg::ResetStatistics()
{
	int	conns = GetConnectionCount();
	for (int i = 0; i < conns; i++)
		m_ConnServ[i]->ResetStatistics();
	m_LocalHost.ResetStatistics();
	m_List.Invalidate();
}

void CServersDlg::RemoveAllPending()
{
	int	conns = GetConnectionCount();
	for (int i = 0; i < conns; i++)
		m_ConnServ[i]->RemoveAllPending();
	// local host doesn't support pending
	m_List.Invalidate();
}

void CServersDlg::SetShowProgress(bool Enable)
{
	if (Enable == m_ShowProgress)
		return;	// nothing to do
	m_ShowProgress = Enable;
	UINT	Period = Enable ? PROGRESS_PERIOD : 0;
	PostMsgToAllServers(PMID_PROGRESS_CONFIG, Period);
	m_List.Invalidate();
	m_ProgressTimer.Run(Enable);
	if (Enable)
		UpdateProgress();
}

void CServersDlg::OnRecord()
{
	RemoveAllPending();
	ResetStatistics();
}

void CServersDlg::OnStop(bool Aborted)
{
	CFracticeView	*view = m_Main->GetRecorder().GetView();
	if (view != NULL)
		m_LastRecordUID = view->GetEngine().GetCurUID();	// save local UID
	RemoveAllPending();
	if (Aborted)
		PostMsgToAllServers(PMID_CANCEL_RENDER);
}

void CServersDlg::OnFrameDone(int ServerIdx, int FrameID, double RenderTime)
{
	CServer	*srv;
	if (ServerIdx < 0)	// if local host
		srv = &m_LocalHost;
	else {
		srv = m_Server[ServerIdx];
		srv->RemovePending(FrameID);	// remove frame from server's pending list
	}
	srv->SetRenderTime(RenderTime);
	srv->SetProgress(100);	// set progress to 100%
	srv->AddToRenderedCount(1);
	if (FastIsVisible())
		RefreshRow(ServerIdx, DYNAMIC_COL);
}

void CServersDlg::OnBeginStripJob()
{
	ResetStatistics();
}

void CServersDlg::OnEndStripJob()
{
	RemoveAllPending();
	if (m_ShowProgress)
		UpdateProgress();
	else
		RefreshAll();
}

CNetEngine *CServersDlg::GetLocalEngine()
{
	CPosterProgressDlg	*poster = CPosterProgressDlg::GetThis();
	if (poster != NULL)	// if a poster render is in progress
		return(&poster->GetEngine());	// use poster dialog's engine
	if (m_View != NULL)	// if current view exists
		return(&m_View->GetEngine());	// use current view's engine
	return(NULL);	// no engine
}

void CServersDlg::UpdateProgress()
{
	if (m_Main->IsRecording()) {
		CFracticeView	*view = m_Main->GetRecorder().GetView();
		if (view != NULL) {
			int	RowCount = view->GetEngine().GetEscFrameSize().cy;
			int	RowsDone = view->GetEngine().GetCurrentRow();
			int	Progress = round(RowsDone * 100.0 / RowCount);
			if (Progress != m_LocalHost.m_Progress) {
				m_LocalHost.m_Progress = Progress;
				RefreshRow(LOCAL_HOST, COL_PROGRESS);
			}
		}
	} else {	// not recording
		CNetEngine	*engine = GetLocalEngine();
		if (engine != NULL) {	// if valid engine
			int	RowCount;
			// if we were recording, unit is actually frames, not rows
			if (engine->GetCurUID() == m_LastRecordUID)
				RowCount = m_Main->GetRecorder().GetRecordInfo().m_FrameCount;
			else	// unit is rows
				RowCount = engine->GetJob().FrameSize.cy;
			if (RowCount) {	// can be zero if last render was color mapping only
				bool	DirtyLocal = FALSE;
				int	Progress = round(engine->GetLocalRowsDone() * 100.0 / RowCount);
				if (Progress != m_LocalHost.GetProgress()) {
					m_LocalHost.SetProgress(Progress);
					DirtyLocal = TRUE;
				}
				bool	DirtyServer = FALSE;
				int	conns = GetConnectionCount();
				for (int i = 0; i < conns; i++) {
					CServer&	srv = m_ConnServ[i];
					int	Progress = round(srv.GetRenderedCount() * 100.0 / RowCount);
					if (Progress != srv.GetProgress()) {
						srv.SetProgress(Progress);
						DirtyServer = TRUE;
					}
				}
				if (DirtyServer)
					RefreshAll();
				else {
					if (DirtyLocal)
						RefreshRow(LOCAL_HOST, COL_PROGRESS);
				}
			}
		}
	}
}

CString CServersDlg::GetOSVersion(const OSVERSIONINFOEX& vi)
{
#if _MFC_VER > 0x0600
	if (vi.wProductType == VER_NT_SERVER) {
		if (vi.dwMajorVersion == 6 && vi.dwMinorVersion == 1)
			return(_T("Windows Server 2008 R2"));
		if (vi.dwMajorVersion == 6 && vi.dwMinorVersion == 0)
			return(_T("Windows Server 2008"));
		if (vi.dwMajorVersion == 5 && vi.dwMinorVersion == 2)
			return(_T("Windows Server 2003"));
	}
#endif
	if (vi.dwMajorVersion == 6 && vi.dwMinorVersion == 1)
		return(_T("Windows 7"));
	if (vi.dwMajorVersion == 6 && vi.dwMinorVersion == 0)
		return(_T("Windows Vista"));
	if (vi.dwMajorVersion == 5 && vi.dwMinorVersion == 2)
		return(_T("Windows XP x64"));
	if (vi.dwMajorVersion == 5 && vi.dwMinorVersion == 1)
		return(_T("Windows XP"));
	if (vi.dwMajorVersion == 5 && vi.dwMinorVersion == 0)
		return(_T("Windows 2000"));
	if (vi.dwMajorVersion == 4 && vi.dwMinorVersion == 0)
		return(_T("Windows NT"));
	CString	s;
	s.Format(_T("Windows %d.%d"), vi.dwMajorVersion, vi.dwMinorVersion);
	return(s);	// generic version
}

CString CServersDlg::GetOSTitle(const OSVERSIONINFOEX& vi)
{
	CString	s(GetOSVersion(vi));
	CString	ServPack;
	ServPack.Format(_T(" SP%d"), vi.wServicePackMajor);
	s += ServPack;	// append service pack
	return(s);
}

void CServersDlg::DoDataExchange(CDataExchange* pDX)
{
	CToolDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CServersDlg)
	DDX_Control(pDX, IDC_SRV_SHOW_PROGRESS, m_ShowProgressChk);
	DDX_Control(pDX, IDC_SRV_ADD_LIST, m_AddListBtn);
	DDX_Control(pDX, IDC_SRV_SHUTDOWN, m_ShutdownBtn);
	DDX_Control(pDX, IDC_SRV_FIND, m_FindBtn);
	DDX_Control(pDX, IDC_SRV_DELETE, m_DeleteBtn);
	DDX_Control(pDX, IDC_SRV_ADD, m_AddBtn);
	DDX_Control(pDX, IDC_SRV_LIST, m_List);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CServersDlg, CToolDlg)
	//{{AFX_MSG_MAP(CServersDlg)
	ON_BN_CLICKED(IDC_SRV_FIND, OnFind)
	ON_BN_CLICKED(IDC_SRV_DELETE, OnDelete)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_SRV_LIST, OnItemchangedList)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_SRV_LIST, OnGetdispinfoList)
	ON_BN_CLICKED(IDC_SRV_ADD, OnAdd)
	ON_BN_CLICKED(IDC_SRV_SHUTDOWN, OnShutdown)
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_SRV_ADD_LIST, OnSrvAddList)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SRV_LIST, OnCustomDraw)
	ON_BN_CLICKED(IDC_SRV_SHOW_PROGRESS, OnShowProgress)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_SYSCOMMAND()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_RH_RESET_COLS, OnResetCols)
	ON_COMMAND(ID_SRV_DELETE, OnDelete)
	ON_COMMAND(ID_SRV_SHUTDOWN, OnShutdown)
	ON_COMMAND(ID_SRV_PROPERTIES, OnProperties)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CServersDlg message handlers

BOOL CServersDlg::OnInitDialog() 
{
	CToolDlg::OnInitDialog();

	m_Main = theApp.GetMain();
	SetIcon(theApp.LoadIcon(IDR_MAINFRAME), FALSE);
	m_List.SetColumns(COLUMNS, m_ColInfo);
	m_List.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP);
	m_List.InitControl(0, CReportCtrl::SORT_ARROWS);
	m_List.SetSortCallback(SortCompare, this);
	m_List.SortRows(COL_HOST_NAME);	// initial sort
	m_List.LoadHeaderState(REG_SETTINGS, RK_HEADER_STATE);

	GetWindowRect(m_InitRect);
	m_Resize.AddControlList(this, m_CtrlList);
	m_ProgressBarBrush.CreateSolidBrush(RGB(0, 0, 255));

	theApp.GetComputerName(m_LocalHost.m_HostName);
	m_LocalHost.m_IPAddress.LoadString(IDS_SRV_LOCAL_HOST);
	COptionsDlg&	OptsDlg = m_Main->GetOptionsDlg();
	m_LocalHost.m_CPUCount = theApp.GetCPUCount();
	m_LocalHost.m_Threads = OptsDlg.GetThreadCount();
	UpdateList();

	m_ProgressTimer.Create(m_hWnd, PROGRESS_TIMER, PROGRESS_PERIOD, FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CServersDlg::OnDestroy() 
{
	m_List.StoreHeaderState(REG_SETTINGS, RK_HEADER_STATE);
	CToolDlg::OnDestroy();
}

void CServersDlg::OnSize(UINT nType, int cx, int cy) 
{
	CToolDlg::OnSize(nType, cx, cy);
	m_Resize.OnSize();	
}

void CServersDlg::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	if (!m_InitRect.IsRectNull())	// wait until rect is initialized
		lpMMI->ptMinTrackSize = CPoint(m_InitRect.Width(), m_InitRect.Height());
}

void CServersDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CToolDlg::OnShowWindow(bShow, nStatus);
	if (bShow && m_ShowProgress)
		UpdateProgress();
}

void CServersDlg::OnFind() 
{
	WORD	PortNumber = m_Main->GetOptionsDlg().GetNetworkPort();
	CFindServersDlg	FindDlg(PortNumber);
	if (FindDlg.DoModal() == IDOK) {
		CServerInfoArray	ServInfo;
		FindDlg.GetServerList(ServInfo);
		if (ServInfo.GetSize()) {
			CServersFoundDlg	FoundDlg;
			FoundDlg.SetServerInfo(ServInfo);
			if (FoundDlg.DoModal() == IDOK) {
				FoundDlg.GetSelectedServers(ServInfo);
				CMainFrame::CStatusMsg	status(IDS_SRV_ADDING_SERVERS);
				AddServers(ServInfo);
			}
		} else
			AfxMessageBox(IDS_SRV_NO_SERVERS_FOUND);
	}
}

void CServersDlg::OnDelete() 
{
	POSITION	pos = m_List.GetFirstSelectedItemPosition();
	while (pos != NULL) {
		int item = m_List.GetNextSelectedItem(pos);
		int	ServerIdx = INT64TO32(m_List.GetItemData(item));
		if (ServerIdx < 0)	// ignore non-server rows
			continue;
		// We don't actually delete the server from the array, because that
		// would invalidate stored server indices. Instead we disconnect it,
		// which hides it from the list, and let it stay in the array forever.
		// This means the array can become bloated with dead elements, but it
		// shouldn't be a problem for reasonable numbers of deletions.
		m_Server[ServerIdx]->Disconnect();
	}
	UpdateList();
}

void CServersDlg::OnItemchangedList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// if a non-server row was selected, unselect it
	if (pNMListView->lParam < 0	// if non-server row
	&& (pNMListView->uChanged & LVIF_STATE)	// and state changed
	&& (pNMListView->uNewState & LVIS_SELECTED))	// and row is selected
		m_List.SetItemState(pNMListView->iItem, 0, LVIS_SELECTED);	// unselect it
	bool	HaveSel	= m_List.GetSelectedCount() > 0;
	GetDlgItem(IDC_SRV_DELETE)->EnableWindow(HaveSel);
	GetDlgItem(IDC_SRV_SHUTDOWN)->EnableWindow(HaveSel);
	*pResult = 0;
}

void CServersDlg::OnGetdispinfoList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO	*pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM	*pItem= &pDispInfo->item;
	int	ServerIdx = INT64TO32(pItem->lParam);
	const CServer&	srv = ServerIdx < 0 ? m_LocalHost : m_Server[ServerIdx];
	if (pItem->mask & LVIF_TEXT) {
		switch (pItem->iSubItem) {
		case COL_HOST_NAME:
			_tcscpy(pItem->pszText, srv.m_HostName);
			break;
		case COL_IP_ADDRESS:
			_tcscpy(pItem->pszText, srv.m_IPAddress);
			break;
		case COL_THREAD_COUNT:
			_stprintf(pItem->pszText, _T("%d"), srv.m_Threads);
			break;
		case COL_RENDER_TIME:
			if (srv.m_RenderTime)	// only show valid times
				_stprintf(pItem->pszText, _T("%.3f"), srv.m_RenderTime);
			break;
		case COL_DONE:
			{
				int	done;
				// if local host and not recording
				if (ServerIdx < 0 && !m_Main->IsRecording()) {
					CNetEngine	*engine = GetLocalEngine();
					if (engine != NULL)
						done = engine->GetLocalRowsDone();
					else
						done = 0;
				} else	// server and/or recording
					done = srv.GetRenderedCount();	// use rendered count
				_stprintf(pItem->pszText, _T("%d"), done);
			}
			break;
		case COL_QUEUE:
			{
				int	queue;
				if (ServerIdx < 0) {	// if local host
					// pending count not supported, use busy count instead
					CNetEngine	*engine = GetLocalEngine();
					if (engine != NULL) {
						queue = engine->GetBusyCount();
						if (m_Main->IsRecording())	// if recording
							queue = queue > 0;	// unit is frames, not strips
					} else
						queue = 0;
				} else	// server
					queue = srv.GetPendingCount();	// use pending count
				_stprintf(pItem->pszText, _T("%d"), queue);
			}
			break;
		}
	}
	*pResult = 0;
}

void CServersDlg::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLVCUSTOMDRAW  pcd = (LPNMLVCUSTOMDRAW)pNMHDR;
	switch (pcd->nmcd.dwDrawStage) {
	case CDDS_PREPAINT:
		if (m_ShowProgress)
			*pResult = CDRF_NOTIFYSUBITEMDRAW;	// request subitem notifications
		else
			*pResult = CDRF_DODEFAULT;
		break;
	case CDDS_ITEMPREPAINT:
		*pResult = CDRF_NOTIFYSUBITEMDRAW;	// request subitem notifications
		break;
	case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
		{     
			int iCol = pcd->iSubItem;
			if (iCol == COL_PROGRESS) {
				int iRow = INT64TO32(pcd->nmcd.dwItemSpec);
				int	ServerIdx = INT64TO32(m_List.GetItemData(iRow));
				const CServer&	srv = ServerIdx < 0 ? 
					m_LocalHost : m_Server[ServerIdx];
				int	Progress = srv.GetProgress();
				HDC	dc = pcd->nmcd.hdc;
				CRect	r;
				m_List.GetSubItemRect(iRow, iCol, LVIR_BOUNDS, r);
				CRect	pr(r);	// bar rect
				pr.DeflateRect(2, 2);
				pr.right = pr.left + round(Progress / 100.0 * pr.Width());
				FillRect(dc, pr, m_ProgressBarBrush);	// draw bar
				CString	s;
				s.Format(_T("%d%%"), Progress);	// create percentage text
				CSize	sz;
				GetTextExtentPoint32(dc, s, s.GetLength(), &sz);
				int	x = r.left + (r.Width() - sz.cx) / 2;
				int	y = r.top + (r.Height() - sz.cy) / 2;
				// draw text in white, so it's visible over bar
				SetTextColor(dc, RGB(255, 255, 255));
				TextOut(dc, x, y, s, s.GetLength());
				// mask off bar and fill background area with white
				ExcludeClipRect(dc, pr.left, pr.top, pr.right, pr.bottom);
				FillRect(dc, r, (HBRUSH)GetStockObject(WHITE_BRUSH));
				// draw text again in black, so it's visible over background
				SetTextColor(dc, RGB(0, 0, 0));
				TextOut(dc, x, y, s, s.GetLength());
				// draw black frame around bar
				r.DeflateRect(1, 1);
				FrameRect(dc, r, (HBRUSH)GetStockObject(BLACK_BRUSH));
				*pResult = CDRF_SKIPDEFAULT;
			} else
				*pResult = CDRF_DODEFAULT;
		}
		break;
	default:
		*pResult = CDRF_DODEFAULT;
	}
}

void CServersDlg::OnAdd() 
{
	CAddServerDlg	dlg;
	if (dlg.DoModal() == IDOK && !dlg.m_HostName.IsEmpty()) {
		CMainFrame::CStatusMsg	status(IDS_SRV_ADDING_SERVERS);
		AddServer(dlg.m_HostName);
	}
}

void CServersDlg::OnSrvAddList() 
{
	CAddServerListDlg	dlg;
	if (dlg.DoModal() == IDOK && !dlg.GetListPath().IsEmpty()) {
		TRY {
			CStdioFile	fp(dlg.GetListPath(), CFile::modeRead | CFile::shareDenyWrite);
			CString	s;
			while (fp.ReadString(s)) {
				CMainFrame::CStatusMsg	status(IDS_SRV_ADDING_SERVERS);
				CString	msg;
				if (!AddServer(s, NULL, &msg, TRUE)) {	// defer update
					UpdateList();	// error: show whatever we've got so far
					if (AfxMessageBox(msg, MB_OKCANCEL) == IDCANCEL)
						break;
				}
			}
		}
		CATCH(CFileException, e)
		{
			e->ReportError();
		}
		END_CATCH
		UpdateList();	// update was deferred within loop
	};
}

void CServersDlg::OnShutdown() 
{
	CServerShutdownDlg	dlg;
	if (dlg.DoModal() == IDOK) {
		POSITION	pos = m_List.GetFirstSelectedItemPosition();
		while (pos != NULL) {
			int item = m_List.GetNextSelectedItem(pos);
			int	ServerIdx = INT64TO32(m_List.GetItemData(item));
			if (ServerIdx < 0)	// ignore non-server rows
				continue;
			m_Server[ServerIdx]->WriteMessage(PMID_SHUTDOWN, dlg.m_Action);
		}
	}
}

void CServersDlg::OnShowProgress() 
{
	SetShowProgress(m_ShowProgressChk.GetCheck() != 0);
}

void CServersDlg::OnTimer(W64UINT nIDEvent) 
{
	if (FastIsVisible())
		UpdateProgress();
	CToolDlg::OnTimer(nIDEvent);
}

void CServersDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	switch (nID & 0xFFF0) {	// mask off unreliable low nibble
	case SC_RESTORE:
		if (m_ShowProgress)
			UpdateProgress();
		break;
	}
	CToolDlg::OnSysCommand(nID, lParam);
}

void CServersDlg::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	CMenu	menu;
	CPoint	pt(point);
	m_List.ScreenToClient(&pt);
	int	item = m_List.HitTest(pt);
	if (item >= 0) {
		int	ServerIdx = INT64TO32(m_List.GetItemData(item));
		if (ServerIdx >= 0)
			menu.LoadMenu(IDR_SERVER_CTX);
	} else {
		CRect	r;
		m_List.GetHeaderCtrl()->GetWindowRect(r);
		if (r.PtInRect(point))
			menu.LoadMenu(IDR_REPORT_HDR);
	}
	if (menu.m_hMenu != NULL) {
		CMenu	*mp = menu.GetSubMenu(0);
		mp->TrackPopupMenu(0, point.x, point.y, this);
	}
}

void CServersDlg::OnResetCols()
{
	m_List.ResetHeaderState();
}

void CServersDlg::OnProperties()
{
	POSITION	pos = m_List.GetFirstSelectedItemPosition();
	CWaitCursor	wc;
	while (pos != NULL) {
		int item = m_List.GetNextSelectedItem(pos);
		int	ServerIdx = INT64TO32(m_List.GetItemData(item));
		if (ServerIdx < 0)	// ignore non-server rows
			continue;
		CServer& srv = m_Server[ServerIdx];
		CString	msg;
		msg.Format(IDS_SRV_PROPS, 
			srv.GetHostName(),
			srv.GetIPAddress(),
			srv.GetCPUCount(),
			srv.GetThreadCount()
		);
		UINT	opts = srv.GetOptions();
		if (opts & FRAP_OCF_HAS_EX_CAPS) {
			FRAP_EX_CAPS_REPLY	ExCaps;
			if (GetExCaps(srv, ServerIdx, ExCaps)) {
				CString	s;
				OSVERSIONINFOEX	vi;
				SYSTEM_INFO	si;
				CPackets::GetOSInfo(ExCaps.OSInfo, vi, si);
				CString	OSVersion(GetOSTitle(vi));
				s.Format(IDS_SRV_PROPS_EX_CAPS,
					HIWORD(ExCaps.ServerVersionMS), LOWORD(ExCaps.ServerVersionMS),
					HIWORD(ExCaps.ServerVersionLS), LOWORD(ExCaps.ServerVersionLS),
					OSVersion,
					BigNum::GetLibName(ExCaps.BigNum.LibType),
					ExCaps.BigNum.Version, ExCaps.BigNum.VersionMinor, 
					ExCaps.BigNum.PatchLevel, 
					BigNum::GetOSName(ExCaps.BigNum.OSType),
					BigNum::GetCPUName(ExCaps.BigNum.CPUType)
				);
				msg += s;
			}
		}
		CMsgBoxDlg::Do(msg, LDS(IDS_SRV_PROPS_TITLE), MB_ICONINFORMATION);
	}
}
