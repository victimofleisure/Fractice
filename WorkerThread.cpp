// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version

		worker thread
 
*/

#include "stdafx.h"
#include "WorkerThread.h"

CWorkerThread::CWorkerThread()
{
	m_Running = FALSE;
	m_Starting = FALSE; 
	m_Working = FALSE;
	m_Canceled = FALSE;
	m_Dead = FALSE;
}

CWorkerThread::~CWorkerThread()
{
	Kill();
}

CWinThread *CWorkerThread::Launch(AFX_THREADPROC pfnThreadProc, LPVOID pParam,
								  int nPriority, UINT nStackSize, DWORD dwCreateFlags,
								  LPSECURITY_ATTRIBUTES lpSecurityAttrs)
{
	Kill();
	m_Running = TRUE;	// assume success
	m_Starting = FALSE;
	m_Working = FALSE;
	m_Canceled = FALSE;
	m_Dead = FALSE;
	CWinThread	*pThread = AfxBeginThread(pfnThreadProc, pParam, nPriority, 
		nStackSize, dwCreateFlags, lpSecurityAttrs);
	if (pThread == NULL)
		m_Running = FALSE;	// failure
	return(pThread);
}

void CWorkerThread::Kill()
{
	if (m_Running) {
		m_Running = FALSE;
		m_Starting = FALSE;
		m_Canceled = TRUE;
		m_Start.SetEvent();
		while (!m_Dead)
			Sleep(1);
	}
}

bool CWorkerThread::WaitForWork()
{
	do {
		m_Working = FALSE;
		WaitForSingleObject(m_Start, INFINITE);
		if (!m_Running) {
			m_Dead = TRUE;
			return(FALSE);
		}
		m_Working = TRUE;	// order matters; see Cancel
		m_Starting = FALSE;
	} while (m_Canceled);
	return(TRUE);
}

void CWorkerThread::Cancel(bool WaitForIdle)
{
	m_Canceled = TRUE;
	if (WaitForIdle) {
		// We need a double handshake here, because otherwise a race can occur
		// if the worker thread just woke up and we preempt WaitForWork before
		// it gets a chance to set m_Working. By blocking on m_Starting BEFORE
		// blocking on m_Working, we avoid a race, because WaitForWork doesn't
		// reset m_Starting until AFTER it sets m_Working.
		while (m_Starting)
			Sleep(1);
		while (m_Working)
			Sleep(1);
	}
}
