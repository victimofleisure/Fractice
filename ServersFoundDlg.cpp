// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		11dec09	add sort compare macro for brevity
		02		06jan10	W64: cast item indices to 32-bit

        server selection dialog
 
*/

// ServersFoundDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "ServersFoundDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CServersFoundDlg dialog

IMPLEMENT_DYNAMIC(CServersFoundDlg, CDialog);

const CServersFoundDlg::COL_INFO CServersFoundDlg::m_ColInfo[COLUMNS] = {
	{IDS_SRV_COL_HOST_NAME,			LVCFMT_LEFT,	120,	CReportCtrl::DIR_ASC},
	{IDS_SRV_COL_IP_ADDR,			LVCFMT_LEFT,	100,	CReportCtrl::DIR_ASC},
	{IDS_SRV_COL_CLIENT_HOST_NAME,	LVCFMT_LEFT,	120,	CReportCtrl::DIR_ASC},
	{IDS_SRV_COL_CLIENT_IP_ADDR,	LVCFMT_LEFT,	100,	CReportCtrl::DIR_ASC},
};

const CCtrlResize::CTRL_LIST CServersFoundDlg::m_CtrlList[] = {
	{IDC_SRVFND_LIST,				BIND_ALL},
	{IDC_SRVFND_SHOW_AVAIL_ONLY,	BIND_BOTTOM | BIND_LEFT},
	{IDOK,							BIND_BOTTOM | BIND_LEFT},
	{IDCANCEL,						BIND_BOTTOM | BIND_LEFT},
	{0, 0}	// list terminator
};

CServersFoundDlg::CServersFoundDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
	//{{AFX_DATA_INIT(CServersFoundDlg)
	//}}AFX_DATA_INIT
	m_InitRect.SetRectEmpty();
}

#define SORT_CMP(x) retc = m_List.SortCmp(srv1.x, srv2.x);

int CServersFoundDlg::SortCompare(int p1, int p2)
{
	const CServerInfo&	srv1 = m_Server[p1];
	const CServerInfo&	srv2 = m_Server[p2];
	int	retc;
	switch (m_List.SortCol()) {
	case COL_HOST_NAME:
		SORT_CMP(m_HostName);
		break;
	case COL_IP_ADDRESS:
		SORT_CMP(m_BinIPAddr);
		break;
	case COL_CLIENT_HOST_NAME:
		SORT_CMP(m_ClientHostName);
		break;
	case COL_CLIENT_IP_ADDRESS:
		SORT_CMP(m_ClientBinIPAddr);
		break;
	default:
		ASSERT(0);	// logic error
		retc = 0;
	}
	return(retc);
}

int CALLBACK CServersFoundDlg::SortCompare(LPARAM p1, LPARAM p2, LPARAM This)
{
	return(((CServersFoundDlg *)This)->SortCompare(INT64TO32(p1), INT64TO32(p2)));
}

void CServersFoundDlg::UpdateList()
{
	int	servs = m_Server.GetSize();
	int	rows = 0;
	m_List.DeleteAllItems();
	bool	ShowAvailOnly = m_ShowAvailOnlyChk.GetCheck() != 0;
	for (int i = 0; i < servs; i++) {
		// if server is disconnected, or if showing all servers
		if (!m_Server[i].m_Status || !ShowAvailOnly)
			m_List.InsertCallbackRow(rows++, i);
	}
	m_List.SortRows();
}

void CServersFoundDlg::SelectAllAvail()
{
	int	items = m_List.GetItemCount();
	for (int i = 0; i < items; i++) {
		int	ServerIdx = INT64TO32(m_List.GetItemData(i));
		m_List.SetItemState(i, m_Server[ServerIdx].m_Status ? 
			0 : LVIS_SELECTED, LVIS_SELECTED);	// select all available servers
	}
}

void CServersFoundDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CServersFoundDlg)
	DDX_Control(pDX, IDC_SRVFND_SHOW_AVAIL_ONLY, m_ShowAvailOnlyChk);
	DDX_Control(pDX, IDCANCEL, m_CancelBtn);
	DDX_Control(pDX, IDOK, m_OKBtn);
	DDX_Control(pDX, IDC_SRVFND_LIST, m_List);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CServersFoundDlg, CDialog)
	//{{AFX_MSG_MAP(CServersFoundDlg)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_SRVFND_LIST, OnGetdispinfoList)
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_BN_CLICKED(IDC_SRVFND_SHOW_AVAIL_ONLY, OnShowAvailOnly)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CServersFoundDlg message handlers

BOOL CServersFoundDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	GetWindowRect(m_InitRect);
	m_List.SetColumns(COLUMNS, m_ColInfo);
	m_List.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_List.InitControl(0, CReportCtrl::SORT_ARROWS);
	m_List.SetSortCallback(SortCompare, this);
	m_List.SortRows(COL_HOST_NAME);	// initial sort
	m_ShowAvailOnlyChk.SetCheck(TRUE);
	UpdateList();
	SelectAllAvail();
	m_Resize.AddControlList(this, m_CtrlList);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CServersFoundDlg::OnOK() 
{
	if (!m_List.GetSelectedCount()) {
		AfxMessageBox(IDS_SRVFND_NO_SELECTION);
		return;
	}
	POSITION	pos = m_List.GetFirstSelectedItemPosition();
	while (pos != NULL) {
		int	nItem = m_List.GetNextSelectedItem(pos);
		int	ServerIdx = INT64TO32(m_List.GetItemData(nItem));
		if (m_Server[ServerIdx].m_Status) {	// if server is connected
			AfxMessageBox(IDS_SRVFND_UNAVAILABLE);
			m_SelServ.RemoveAll();	// reset destination array
			return;
		}
		m_SelServ.Add(m_Server[ServerIdx]);
	}
	CDialog::OnOK();
}

void CServersFoundDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	m_Resize.OnSize();	
}

void CServersFoundDlg::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	if (!m_InitRect.IsRectNull())	// wait until rect is initialized
		lpMMI->ptMinTrackSize = CPoint(m_InitRect.Width(), m_InitRect.Height());
}

void CServersFoundDlg::OnGetdispinfoList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO	*pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM	*pItem= &pDispInfo->item;
	int	ServerIdx = INT64TO32(pItem->lParam);
	const CServerInfo&	srv = m_Server[ServerIdx];
	if (pItem->mask & LVIF_TEXT) {
		switch(pItem->iSubItem){
        case COL_HOST_NAME:
            _tcscpy(pItem->pszText, srv.m_HostName);
            break;
        case COL_IP_ADDRESS:
            _tcscpy(pItem->pszText, srv.m_IPAddress);
            break;
        case COL_CLIENT_HOST_NAME:
            _tcscpy(pItem->pszText, srv.m_ClientHostName);
            break;
        case COL_CLIENT_IP_ADDRESS:
            _tcscpy(pItem->pszText, srv.m_ClientIPAddress);
            break;
	    }
	}
	*pResult = 0;
}

void CServersFoundDlg::OnShowAvailOnly() 
{
	UpdateList();
	SelectAllAvail();
}
