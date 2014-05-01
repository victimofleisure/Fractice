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

#ifndef CWORKERTHREAD_INCLUDED
#define CWORKERTHREAD_INCLUDED

#include <afxmt.h>

class CWorkerThread : public WObject {
public:
// Construction
	CWorkerThread();
	~CWorkerThread();

// Attributes
	bool	IsRunning() const;
	bool	IsStarting() const;
	bool	IsWorking() const;
	bool	IsCanceled() const;
	bool	IsDead() const;

// Operations
	CWinThread	*Launch(AFX_THREADPROC pfnThreadProc, LPVOID pParam,
		int nPriority = THREAD_PRIORITY_NORMAL, UINT nStackSize = 0,
		DWORD dwCreateFlags = 0, LPSECURITY_ATTRIBUTES lpSecurityAttrs = NULL);
	void	Kill();
	bool	WaitForWork();
	void	StartWork();
	void	Cancel(bool WaitForIdle = FALSE);

protected:
// Member data
	CEvent	m_Start;			// thread blocks on this while idle
	volatile bool	m_Running;	// true if thread is running
	volatile bool	m_Starting;	// true if thread is starting work
	volatile bool	m_Working;	// true if thread is working
	volatile bool	m_Canceled;	// true if work is canceled
	volatile bool	m_Dead;		// true if thread is dead
};

inline bool CWorkerThread::IsRunning() const
{
	return(m_Running);
}

inline bool CWorkerThread::IsStarting() const
{
	return(m_Starting);
}

inline bool CWorkerThread::IsWorking() const
{
	return(m_Working);
}

inline bool CWorkerThread::IsCanceled() const
{
	return(m_Canceled);
}

inline bool CWorkerThread::IsDead() const
{
	return(m_Dead);
}

inline void CWorkerThread::StartWork()
{
	m_Canceled = FALSE;
	m_Starting = TRUE;
	m_Start.SetEvent();
}

#endif
