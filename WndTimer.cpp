// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version

		window timer
 
*/

#include "stdafx.h"
#include "WndTimer.h"

CWndTimer::CWndTimer()
{
	m_Timer = 0;
	m_hWnd = NULL;
	m_Event = 0;
	m_Period = 0;
	m_Callback = NULL;
}

CWndTimer::~CWndTimer()
{
	Destroy();
}

void CWndTimer::Create(HWND hWnd, UINT Event, UINT Period, bool Enable, TIMER_CALLBACK Callback)
{
	Destroy();
	m_hWnd = hWnd;
	m_Event = Event;
	m_Period = Period;
	m_Callback = Callback;
	if (Enable)
		m_Timer = SetTimer(hWnd, Event, Period, Callback);
}

void CWndTimer::Destroy()
{
	if (m_Timer) {
		KillTimer(m_hWnd, m_Timer);
		m_Timer = 0;
	}
}

void CWndTimer::SetPeriod(UINT Period)
{
	if (Period == m_Period)
		return;
	if (m_Timer) {
		KillTimer(m_hWnd, m_Timer);
		m_Timer = SetTimer(m_hWnd, m_Event, Period, m_Callback);
	}
	m_Period = Period;
}

void CWndTimer::Run(bool Enable)
{
	if (Enable == IsRunning())
		return;
	if (Enable)
		m_Timer = SetTimer(m_hWnd, m_Event, m_Period, m_Callback);
	else {
		KillTimer(m_hWnd, m_Timer);
		m_Timer = 0;
	}
}
