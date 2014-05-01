// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      12feb09	initial version
		01		06jan10	W64: in MacAddressToStr, cast byte array size to 32-bit

        get information about network adapters
 
*/

#include "stdafx.h"
#include "NetAdapterInfo.h"
#include "iptypes.h"

void CNetAdapterInfo::CIPAddrStr::Copy(const CIPAddrStr& Addr)
{
	m_IpAddress		= Addr.m_IpAddress;
	m_IpMask		= Addr.m_IpMask;
	m_Context		= Addr.m_Context;
}

void CNetAdapterInfo::Copy(const CNetAdapterInfo& Info)
{
	m_AdapterName	= Info.m_AdapterName;
	m_Description	= Info.m_Description;
	m_MacAddress.Copy(Info.m_MacAddress);
	m_Index			= Info.m_Index;
	m_Type			= Info.m_Type;
	m_DhcpEnabled	= Info.m_DhcpEnabled;
	m_IpAddressList.Copy(Info.m_IpAddressList);
	m_GatewayList	= Info.m_GatewayList;
	m_DhcpServer	= Info.m_DhcpServer;
    m_HaveWins		= Info.m_HaveWins;
    m_PrimaryWinsServer		= Info.m_PrimaryWinsServer;
    m_SecondaryWinsServer	= Info.m_SecondaryWinsServer;
    m_LeaseObtained	= Info.m_LeaseObtained;
    m_LeaseExpires	= Info.m_LeaseExpires;
}

CString CNetAdapterInfo::MacAddressToStr(const CByteArray& MacAddr)
{
	CString	s;
	int	maclen = INT64TO32(MacAddr.GetSize());
	for (int i = 0; i < maclen; i++) {
		if (i)
			s += _T("-");
		CString	t;
		t.Format(_T("%02X"), MacAddr[i]);
		s += t;
	}
	return(s);
}

static void GetIPAddrList(CNetAdapterInfo::CNetIPAddrList& List, PIP_ADDR_STRING pas)
{
	while (pas != NULL) {	// for each IP address
		CNetAdapterInfo::CIPAddrStr	addr;
		addr.m_IpAddress = pas->IpAddress.String;
		addr.m_IpMask = pas->IpMask.String;
		addr.m_Context = pas->Context;
		List.Add(addr);
		pas = pas->Next;	// point to next IP address in list
	}
}

typedef DWORD (WINAPI* LPGETADAPTERSINFO)(PIP_ADAPTER_INFO, PULONG);

bool EnumNetAdapterInfo(CNetAdapterInfoArray& List)
{
	bool	retc = FALSE;
	HINSTANCE	hInst = LoadLibrary(_T("IPHlpApi.dll"));
	if (hInst != NULL) {
		LPGETADAPTERSINFO	pGetAdaptersInfo = 
			(LPGETADAPTERSINFO)GetProcAddress(hInst, "GetAdaptersInfo");
		if (pGetAdaptersInfo != NULL) {	// if function was imported
			ULONG	len;
			DWORD	err = pGetAdaptersInfo(NULL, &len);
			if (err == ERROR_BUFFER_OVERFLOW) {	// if we got buffer size
				CByteArray	ba;
				ba.SetSize(len);
				PIP_ADAPTER_INFO	pai = (PIP_ADAPTER_INFO)ba.GetData();
				err = pGetAdaptersInfo(pai, &len);
				if (err == ERROR_SUCCESS) {
					List.RemoveAll();
					while (pai != NULL) {	// for each network adapter
						CNetAdapterInfo	info;
						info.m_AdapterName = pai->AdapterName;
						info.m_Description = pai->Description;
						DWORD	maclen = pai->AddressLength;
						info.m_MacAddress.SetSize(maclen);
						memcpy(info.m_MacAddress.GetData(), pai->Address, maclen);
						info.m_Index = pai->Index;
						info.m_Type = pai->Type;
						info.m_DhcpEnabled = pai->DhcpEnabled;
						GetIPAddrList(info.m_IpAddressList, &pai->IpAddressList);
						GetIPAddrList(info.m_GatewayList, &pai->GatewayList);
						GetIPAddrList(info.m_DhcpServer, &pai->DhcpServer);
						info.m_HaveWins = pai->HaveWins;
						GetIPAddrList(info.m_PrimaryWinsServer, &pai->PrimaryWinsServer);
						GetIPAddrList(info.m_SecondaryWinsServer, &pai->SecondaryWinsServer);
						info.m_LeaseObtained = CTime(pai->LeaseObtained);
						info.m_LeaseExpires = CTime(pai->LeaseExpires);
						List.Add(info);
						pai = pai->Next;	// point to next adapter in list
						retc = TRUE;	// at least one adapter was found
					}
				}
			}
		}
		FreeLibrary(hInst);
	}
	return(retc);
}
