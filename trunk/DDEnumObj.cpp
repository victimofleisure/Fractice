// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*	
		chris korda

		revision history:
		rev		date	comments
		00		16feb09	initial version

		container for DirectDraw enumerated object

*/

#include "stdafx.h"
#include "DDEnumObj.h"

CDDEnumObj::CDDEnumObj()
{
	ZeroMemory(&m_Guid, sizeof(GUID));
	m_hMonitor = 0;
}

void CDDEnumObj::Copy(const CDDEnumObj& Info)
{
	m_Guid					= Info.m_Guid;
	m_DriverDescription		= Info.m_DriverDescription;
	m_DriverName			= Info.m_DriverName;
	m_hMonitor				= Info.m_hMonitor;
}

void CDDEnumObj::GetScreenCoords(CRect& rc)
{
	MONITORINFO	mi;
	mi.cbSize = sizeof(MONITORINFO);	// crucial
	if (GetMonitorInfo(m_hMonitor, &mi)) {
		rc = mi.rcMonitor;
	} else {	// assume we're on primary monitor
		rc = CRect(0, 0, GetSystemMetrics(SM_CXSCREEN),
			GetSystemMetrics(SM_CYSCREEN));
	}
}

