// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      16jul05	initial version
        01      19feb09	allow relaunch

		self-correcting timer
 
*/

#include "stdafx.h"
#include <process.h>
#include <math.h>
#include "RealTimer.h"
#include "Benchmark.h"

CRealTimer::CRealTimer()
{
	m_Callback = NULL;
	m_Cookie = NULL;
	Init();
}

CRealTimer::~CRealTimer()
{
	Kill();
}

void CRealTimer::Init()
{
	m_PrevFreq = 0;
	m_Freq = 1;
	m_Reset = 0;
	m_State = STOP;
	m_NewState = STOP;
}

bool CRealTimer::Launch(LPCALLBACK Callback, LPVOID Cookie, int Priority)
{
	if (m_Thread != NULL || Callback == NULL)
		return(FALSE);
	m_Callback = Callback;
	m_Cookie = Cookie;
	Init();
	if (!m_Timer.Create(NULL, 0, NULL))
		return(FALSE);
	if (!m_Event.Create(NULL, FALSE, FALSE, NULL))
		return(FALSE);
	if (!m_Thread.Create(NULL, 0, ThreadFunc, this, CREATE_SUSPENDED, NULL))
		return(FALSE);
	if (!SetThreadPriority(m_Thread, Priority))
		return(FALSE);
	return(ResumeThread(m_Thread) != 0);
}

bool CRealTimer::Kill()
{
	if (m_Thread == NULL)
		return(FALSE);
	m_NewState = DIE;
	SetEvent(m_Event);
	WaitForSingleObject(m_Thread, INFINITE);
	m_Thread.Close();
	m_Event.Close();
	m_Timer.Close();
	return(TRUE);
}

UINT __stdcall CRealTimer::ThreadFunc(LPVOID pParam)
{
	((CRealTimer *)pParam)->DoTimer();
	return(0);
}

void CRealTimer::DoTimer()
{
	CBenchmark	bm;
	double	Error = 0;
	double	Period = m_Freq > 0 ? 1.0f / m_Freq : 0;
	int		PrevReset = 0;
	UINT	Clock = 0;
	HANDLE	ha[2] = {m_Event, m_Timer};
	while (m_State != DIE) {
		if (Period > 0) {
			__int64	Due = round((Period - Error) * -1e7);	// relative UTC
			SetWaitableTimer(m_Timer, (LARGE_INTEGER *)&Due, 0, NULL, NULL, 0);
		} else	// infinite period
			CancelWaitableTimer(m_Timer);
		WaitForMultipleObjects(m_State + 1, ha, FALSE, INFINITE);
		m_State = m_NewState;
		if (m_State == RUN) {
			m_Callback(m_Cookie);
			Clock++;
			Error = double(bm.Elapsed() - Clock * Period);
			if (m_Reset != PrevReset) {
				Period = m_Freq > 0 ? 1.0f / m_Freq : 0;
				PrevReset = m_Reset;
				Clock = 0;
				Error = 0;
				bm.Reset();
			}
		}
	}
}

void CRealTimer::Run(bool Enable)
{
	m_NewState = Enable ? RUN : STOP;
	m_Reset++;
	SetEvent(m_Event);
}

void CRealTimer::SetFreq(double Freq, bool Resync)
{
	m_Freq = Freq;
	m_Reset++;
	if (Resync || fabs(m_PrevFreq - Freq) >= m_PrevFreq * .5)	// difference > 50%
		SetEvent(m_Event);
	m_PrevFreq = Freq;
}

void CRealTimer::Resync()
{
	m_Reset++;
	SetEvent(m_Event);
}
