// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      16jul05	initial version
        00      19feb09	rename included headers

		self-correcting timer
 
*/

#ifndef CREALTIMER_INCLUDED
#define CREALTIMER_INCLUDED

#include "Event.h"
#include "Thread.h"
#include "Timer.h"

class CRealTimer : public WObject {
public:
	typedef	void (*LPCALLBACK)(LPVOID Cookie);
	CRealTimer();
	~CRealTimer();
	bool	Launch(LPCALLBACK Callback, LPVOID Cookie, int Priority);
	bool	Kill();
	void	Run(bool Enable);
	bool	IsRunning() const;
	void	SetFreq(double Freq, bool Resync = FALSE);
	double	GetFreq() const;
	void	Resync();

private:
	enum {	// timer states
		STOP,
		RUN,
		DIE
	};
	LPCALLBACK	m_Callback;	// if non-null, pointer to user's callback function
	LPVOID		m_Cookie;	// user-defined data passed to callback function
	WTimer		m_Timer;	// timer object
	WEvent		m_Event;	// event flag
	WThread		m_Thread;	// thread instance
	volatile	double	m_PrevFreq;	// previous target frequency, in Hz
	volatile	double	m_Freq;		// target frequency, in Hz
	volatile	int		m_Reset;	// if changed, thread resets timer
	volatile	int		m_State;	// timer's current state
	volatile	int		m_NewState;	// timer's new state

	void	Init();
	void	DoTimer();
	static	UINT __stdcall ThreadFunc(LPVOID pParam);
};

inline double CRealTimer::GetFreq() const
{
	return(m_Freq);
}

inline bool CRealTimer::IsRunning() const
{
	return(m_State == RUN);
}

#endif
