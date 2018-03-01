// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      12feb09	initial version

        get information about network adapters
 
*/

#ifndef CNETADAPTERINFO_INCLUDED
#define CNETADAPTERINFO_INCLUDED

#include "ArrayEx.h"

class CNetAdapterInfo : public CObject {
public:
// Construction
	CNetAdapterInfo();
	CNetAdapterInfo(const CNetAdapterInfo& Info);
	CNetAdapterInfo& operator=(const CNetAdapterInfo& Info);

// Nested classes
	class CIPAddrStr : public CObject {
	public:
	// Construction
		CIPAddrStr();
		CIPAddrStr(const CIPAddrStr& Addr);
		CIPAddrStr& operator=(const CIPAddrStr& Addr);

	// Data members
		CString	m_IpAddress;	// IPv4 address
		CString	m_IpMask;		// IPv4 subnet mask
		DWORD	m_Context;		// network table entry (NTE)

	protected:
	// Helpers
		void	Copy(const CIPAddrStr& Info);
	};
	typedef CArrayEx<CIPAddrStr, CIPAddrStr&> CNetIPAddrList;

// Data members
	CString	m_AdapterName;		// adapter name (typically a GUID)
	CString	m_Description;		// description of adapter
	CByteArray	m_MacAddress;	// adapter's binary MAC address
	DWORD	m_Index;			// adapter index
	UINT	m_Type;				// adapter type
	UINT	m_DhcpEnabled;		// true if DHCP enabled for this adapter
	CNetIPAddrList	m_IpAddressList;	// list of adapter's IP addresses
	CNetIPAddrList	m_GatewayList;		// list of adapter's gateways
	CNetIPAddrList	m_DhcpServer;		// list of adapter's DHCP servers
    BOOL	m_HaveWins;			// true if adapter uses WINS
    CNetIPAddrList	m_PrimaryWinsServer;	// list of primary WINS servers
    CNetIPAddrList	m_SecondaryWinsServer;	// list of secondary WINS servers
    CTime	m_LeaseObtained;	// when current DHCP lease was obtained
    CTime	m_LeaseExpires;		// when current DHCP lease expires

// Operations
	static	CString	MacAddressToStr(const CByteArray& MacAddr);

protected:
// Helpers
	void	Copy(const CNetAdapterInfo& Info);
};

inline CNetAdapterInfo::CIPAddrStr::CIPAddrStr()
{
}

inline CNetAdapterInfo::CIPAddrStr::CIPAddrStr(const CIPAddrStr& Addr)
{
	Copy(Addr);
}

inline CNetAdapterInfo::CIPAddrStr& CNetAdapterInfo::CIPAddrStr::operator=(const CIPAddrStr& Addr)
{
	Copy(Addr);
	return(*this);
}

inline CNetAdapterInfo::CNetAdapterInfo()
{
}

inline CNetAdapterInfo::CNetAdapterInfo(const CNetAdapterInfo& Info)
{
	Copy(Info);
}

inline CNetAdapterInfo& CNetAdapterInfo::operator=(const CNetAdapterInfo& Info)
{
	Copy(Info);
	return(*this);
}

typedef CArrayEx<CNetAdapterInfo, CNetAdapterInfo&> CNetAdapterInfoArray;

bool EnumNetAdapterInfo(CNetAdapterInfoArray& List);

#endif
