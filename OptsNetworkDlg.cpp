// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      08feb09	initial version
		01		12feb09	add network adapter combo box

        network options page
 
*/

// OptsNetworkDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Fractice.h"
#include "OptionsDlg.h"
#include "OptsNetworkDlg.h"
#include "Packets.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptsNetworkDlg property page

#define RK_NIC_DESCRIP		_T("NicDescrip")
#define RK_NETWORK_PORT		_T("NetworkPort")
#define RK_LISTEN_SERVERS	_T("ListenServers")

COptsNetworkDlg::COptsNetworkDlg(COptionsInfo& Info)
	: CPropertyPage(IDD),
	m_oi(Info)
{
	//{{AFX_DATA_INIT(COptsNetworkDlg)
	//}}AFX_DATA_INIT
	m_NicDescrip = theApp.RdRegString(RK_NIC_DESCRIP);
	EnumNics();
	UpdateNicInfo();
	theApp.RdReg2UInt(RK_NETWORK_PORT, m_oi.m_NetworkPort);
	m_oi.m_NetDefPort = m_oi.m_NetworkPort == FRACTICE_NET_PORT;
	theApp.RdReg2Int(RK_LISTEN_SERVERS, m_oi.m_ListenServers);
}

COptsNetworkDlg::~COptsNetworkDlg()
{
	theApp.WrRegString(RK_NIC_DESCRIP, m_NicDescrip);
	theApp.WrRegInt(RK_NETWORK_PORT, m_oi.m_NetworkPort);
	theApp.WrRegInt(RK_LISTEN_SERVERS, m_oi.m_ListenServers);
}

void COptsNetworkDlg::UpdateUI()
{
	UpdateData();	// get controls
	GetDlgItem(IDC_OPTS_NET_PORT_NUM_EDIT)->EnableWindow(!m_oi.m_NetDefPort);
	GetDlgItem(IDC_OPTS_NET_ADAPTER_PROPS)->EnableWindow(m_NicCombo.GetCurSel() > 0);
}

bool COptsNetworkDlg::EnumNics()
{
	m_CurNic = DEFAULT_NIC;
	if (!EnumNetAdapterInfo(m_NicInfo))
		return(FALSE);
	int	adapters = m_NicInfo.GetSize();
	if (!adapters)
		return(FALSE);
	for (int i = 0; i < adapters; i++) {
		if (m_NicInfo[i].m_Description == m_NicDescrip)
			m_CurNic = i;
	}
	return(TRUE);
}

void COptsNetworkDlg::UpdateNicInfo()
{
	if (m_CurNic >= 0) {
		const CNetAdapterInfo&	info = m_NicInfo[m_CurNic];
		m_NicDescrip = info.m_Description;
		if (info.m_IpAddressList.GetSize())
			m_NicIPAddr = info.m_IpAddressList[0].m_IpAddress;
		else
			m_NicIPAddr.Empty();
	} else {	// current selection is invalid
		m_NicDescrip.Empty();
		m_NicIPAddr.Empty();
	}
}

void COptsNetworkDlg::InitNicCombo()
{
	EnumNics();
	m_NicCombo.AddString(LDS(IDS_OPTS_DEFAULT_ITEM));
	int	adapters = m_NicInfo.GetSize();
	for (int i = 0; i < adapters; i++)
		m_NicCombo.AddString(m_NicInfo[i].m_Description);
	m_NicCombo.SetCurSel(m_CurNic + 1);
}

void COptsNetworkDlg::ReadNicCombo()
{
	int	sel = m_NicCombo.GetCurSel();
	m_CurNic = sel > 0 ? sel - 1 : DEFAULT_NIC;
	UpdateNicInfo();
}

void COptsNetworkDlg::OnResetAll()
{
	m_CurNic = DEFAULT_NIC;
	UpdateNicInfo();
}

CString COptsNetworkDlg::FormatIPAddrList(const CNetAdapterInfo::CNetIPAddrList& List)
{
	CString	s;
	for (int i = 0; i < List.GetSize(); i++) {
		const CNetAdapterInfo::CIPAddrStr&	adr = List[i];
		if (i)
			s += _T(", ");
		CString	t;
		t.Format(_T("%s"), adr.m_IpAddress);
		s += t;
		if (!adr.m_IpMask.IsEmpty() && adr.m_IpMask != _T("0.0.0.0")) {
			t.Format(_T("/%s"), adr.m_IpMask);
			s += t;
		}
	}
	return(s);
}

CString COptsNetworkDlg::FormatNetAdapterProps(const CNetAdapterInfo& Info)
{
	CString	s;
	s.Format(IDS_NET_ADAPTER_PROPS,
		Info.m_Description,
		CNetAdapterInfo::MacAddressToStr(Info.m_MacAddress),
		Info.m_Type,
		Info.m_DhcpEnabled,
		FormatIPAddrList(Info.m_IpAddressList),
		FormatIPAddrList(Info.m_GatewayList),
		FormatIPAddrList(Info.m_DhcpServer),
		Info.m_HaveWins,
		FormatIPAddrList(Info.m_PrimaryWinsServer),
		FormatIPAddrList(Info.m_SecondaryWinsServer),
		Info.m_LeaseObtained.Format(_T("%c")),
		Info.m_LeaseExpires.Format(_T("%c"))
	);
	return(s);
}

void COptsNetworkDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptsNetworkDlg)
	DDX_Control(pDX, IDC_OPTS_NET_ADAPTER_COMBO, m_NicCombo);
	//}}AFX_DATA_MAP
	DDX_Text(pDX, IDC_OPTS_NET_PORT_NUM_EDIT, m_oi.m_NetworkPort);
	DDV_MinMaxUInt(pDX, m_oi.m_NetworkPort, 1, USHRT_MAX);
	DDX_Check(pDX, IDC_OPTS_NET_DEF_PORT_CHK, m_oi.m_NetDefPort);
	DDX_Check(pDX, IDC_OPTS_LISTEN_SERVERS_CHK, m_oi.m_ListenServers);
}

BEGIN_MESSAGE_MAP(COptsNetworkDlg, CPropertyPage)
	//{{AFX_MSG_MAP(COptsNetworkDlg)
	ON_BN_CLICKED(IDC_OPTS_NET_DEF_PORT_CHK, OnNetDefPortChk)
	ON_BN_CLICKED(IDC_OPTS_NET_ADAPTER_PROPS, OnNetAdapterProps)
	ON_CBN_SELCHANGE(IDC_OPTS_NET_ADAPTER_COMBO, OnSelchangeNicCombo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptsNetworkDlg message handlers

BOOL COptsNetworkDlg::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	InitNicCombo();
	UpdateUI();
	return TRUE;
}

void COptsNetworkDlg::OnOK() 
{
	ReadNicCombo();
	CPropertyPage::OnOK();
}

void COptsNetworkDlg::OnNetDefPortChk() 
{
	UpdateData();	// get controls
	if (m_oi.m_NetDefPort)
		m_oi.m_NetworkPort = FRACTICE_NET_PORT;
	UpdateData(FALSE);	// init controls
	UpdateUI();
}

void COptsNetworkDlg::OnNetAdapterProps() 
{
	int	sel = m_NicCombo.GetCurSel();
	if (sel > 0) {
		CString	s = FormatNetAdapterProps(m_NicInfo[sel - 1]);
		AfxMessageBox(s, MB_ICONINFORMATION);
	}
}

void COptsNetworkDlg::OnSelchangeNicCombo() 
{
	UpdateUI();
}
