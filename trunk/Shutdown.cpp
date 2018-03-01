// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version

		shutdown the computer
 
*/

#include "stdafx.h"
#include "Shutdown.h"

bool GetShutdownPrivileges(UINT Flags)
{
	if (!(Flags & (EWX_POWEROFF | EWX_REBOOT | EWX_SHUTDOWN)))
		return(TRUE);	// no special privileges needed
	// try to obtain privileges needed for shutdown
	HANDLE	hToken;
	TOKEN_PRIVILEGES tkp;
	// get access token for this process
	OpenProcessToken(GetCurrentProcess(), 
		TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);
	// get LUID for shutdown privilege
	LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid); 
	tkp.PrivilegeCount = 1; // one privilege to set
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;   
	// request shutdown privilege
	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
	// AdjustTokenPrivileges return value gives false positives
	if (GetLastError() != ERROR_SUCCESS)
		return(FALSE);	// fail if privilege wasn't granted
	return(TRUE);
}