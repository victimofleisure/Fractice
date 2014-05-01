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

#include "stdafx.h"
#include "TimerMsgThread.h"

CTimerMsgThread::CTimerMsgThread()
{
	m_hWnd = NULL;
	m_EventID = 0;
	m_Period = 0;
	m_Repeat = FALSE;
	m_Kill = FALSE;
	m_Running = FALSE;
}

CTimerMsgThread::~CTimerMsgThread()
{
	Destroy();
}

bool CTimerMsgThread::Create(HWND hWnd, UINT EventID, UINT Period, bool Repeat)
{
	Destroy();
	m_hWnd = hWnd;
	m_EventID = EventID;
	m_Kill = FALSE;
	if (!m_Thread.Create(NULL, 0, ThreadFunc, this, 0, NULL))
		return(FALSE);
	if (!m_Timer.Create(NULL, FALSE, NULL))
		return(FALSE);
	if (Period)
		return(Set(Period, Repeat));
	return(TRUE);
}

void CTimerMsgThread::Destroy()
{
	if (m_Thread != NULL) {	// if thread was created
		m_Kill = TRUE;	// set exit flag
		Set(0);	// unblock thread
		WaitForSingleObject(m_Thread, INFINITE);
	}
}

bool CTimerMsgThread::Set(UINT Period, bool Repeat)
{
	LARGE_INTEGER	DueTime;
	DueTime.QuadPart = Period;
	DueTime.QuadPart *= -10000;	// relative time, 100 nanosecond units
	if (!m_Timer.Set(&DueTime, Repeat ? Period : 0, NULL, NULL, FALSE))
		return(FALSE);
	m_Period = Period;
	m_Repeat = Repeat;
	m_Running = TRUE;
	return(TRUE);
}

bool CTimerMsgThread::Run()
{
	return(Set(m_Period, m_Repeat));
}

bool CTimerMsgThread::Stop()
{
	if (!m_Timer.Cancel())
		return(FALSE);
	m_Running = FALSE;
	return(TRUE);
}

void CTimerMsgThread::ThreadLoop()
{
	while (!m_Kill) {
		WaitForSingleObject(m_Timer, INFINITE);
		PostMessage(m_hWnd, WM_TIMER, m_EventID, 0);
	}
}

unsigned __stdcall CTimerMsgThread::ThreadFunc(PVOID arg)
{
	CTimerMsgThread	*This = (CTimerMsgThread *)arg;
	This->ThreadLoop();
	return(0);
}
