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

#include "stdafx.h"
#include "ServerInfo.h"

void CServerInfo::Copy(const CServerInfo& Info)
{
	m_HostName			= Info.m_HostName;
	m_IPAddress			= Info.m_IPAddress;
	m_BinIPAddr			= Info.m_BinIPAddr;
	m_Status			= Info.m_Status;
	m_ClientHostName	= Info.m_ClientHostName;
	m_ClientIPAddress	= Info.m_ClientIPAddress;
	m_ClientBinIPAddr	= Info.m_ClientBinIPAddr;
}
