// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version

        thread that posts timer messages
 
*/

#ifndef CTIMERMSGTHREAD_INCLUDED
#define CTIMERMSGTHREAD_INCLUDED

#include "Timer.h"
#include "Thread.h"

class CTimerMsgThread : public WObject {
public:
// Construction
	CTimerMsgThread();
	~CTimerMsgThread();
	bool	Create(HWND hWnd, UINT EventID, UINT Period, bool Repeat = TRUE);
	bool	Set(UINT Period, bool Repeat = TRUE);
	bool	Run();
	bool	Stop();
	void	Destroy();

// Attributes
	UINT	GetPeriod() const;
	UINT	GetEventID() const;
	bool	IsRunning() const;

protected:
// Member data
	WTimer	m_Timer;	// periodic timer
	WThread	m_Thread;	// message-sending thread that blocks on timer
	HWND	m_hWnd;		// destination window for timer messages
	UINT	m_EventID;	// event ID to include in timer message
	UINT	m_Period;	// timer period in milliseconds
	bool	m_Repeat;	// true if timer should trigger repeatedly
	bool	m_Kill;		// true if thread should exit
	bool	m_Running;	// true if timer is running

// Helpers
	void	ThreadLoop();
	static	unsigned __stdcall ThreadFunc(PVOID arg);
};

inline UINT CTimerMsgThread::GetPeriod() const
{
	return(m_Period);
}

inline UINT CTimerMsgThread::GetEventID() const
{
	return(m_EventID);
}

inline bool CTimerMsgThread::IsRunning() const
{
	return(m_Running);
}

#endif
