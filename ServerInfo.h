// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda

		rev		date		comments
        00      14feb08	initial version

		container for server info

*/

#ifndef CSERVERINFO_INCLUDED
#define CSERVERINFO_INCLUDED

#include "ArrayEx.h"

class CServerInfo : public CObject {
public:
// Construction
	CServerInfo();
	CServerInfo(const CServerInfo& Info);
	CServerInfo& operator=(const CServerInfo& Info);

// Data members
	CString	m_HostName;			// server host name
	CString	m_IPAddress;		// server IP address
	UINT	m_BinIPAddr;		// server binary IP address, in host order
	DWORD	m_Status;			// server status; non-zero if connected
	CString	m_ClientHostName;	// if connected, client host name
	CString	m_ClientIPAddress;	// if connected, client IP address
	UINT	m_ClientBinIPAddr;	// if connected, client binary IP address, in host order,

// Helpers
	void	Copy(const CServerInfo& Proj);
};

typedef CArrayEx<CServerInfo, CServerInfo&> CServerInfoArray;

inline CServerInfo::CServerInfo()
{
}

inline CServerInfo::CServerInfo(const CServerInfo& Info)
{
	Copy(Info);
}

inline CServerInfo& CServerInfo::operator=(const CServerInfo& Info)
{
	Copy(Info);
	return(*this);
}

#endif
