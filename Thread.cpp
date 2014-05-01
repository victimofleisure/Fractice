// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda

		rev		date		comments
		00		15aug04		initial version
		01		18oct04		ck: add debug name to create
		02		19oct04		ck: include process.h for portability
		03		19oct04		ck: exclude set debug name in non-ETS platform

		wrap Win32 thread object

*/

#include "StdAfx.h"
#include <process.h>
#include "Thread.h"

WThread::WThread()
{
	m_hThread = NULL;
}

WThread::~WThread()
{
	Close();
}

bool WThread::Create(void *Security, unsigned StackSize, THRFUNCP StartAddress, 
					 void *ArgList, unsigned InitFlag, unsigned *ThreadID, LPCSTR DebugName)
{
	Close();
	m_hThread = (WHANDLE)_beginthreadex(Security, StackSize, 
		StartAddress, ArgList, InitFlag, ThreadID);
#ifdef EMBKERN_H_INCLUDED	// if we're running under PharLap ETS
	if (m_hThread != NULL && DebugName != NULL)
		EtsSetThreadDebugName(m_hThread, const_cast<char *>(DebugName));
#endif
	return(m_hThread != NULL);
}

void WThread::Close()
{
	if (m_hThread != NULL) {
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
}
