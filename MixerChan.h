// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      09mar09	initial version

        mixer channel
 
*/

#ifndef CMIXCHAN_INCLUDED
#define CMIXCHAN_INCLUDED

#include "DPoint.h"
#include "RealTimer.h"
#include "MixerInfo.h"

class CMixer;

class CMixerChan : public WObject, public MIXER_CHAN_INFO {
public:
// Construction
	CMixerChan();

// Constants

// Operations
	void	GetBaseInfo(MIXER_CHAN_INFO& Info) const;
	void	SetBaseInfo(const MIXER_CHAN_INFO& Info);
	void	Metronome(bool Enable);
	void	UpdateMetronome();
	void	UpdateOrigin(CSize SrcSize);

// Data members
	CMixer	*m_Mixer;			// pointer to parent mixer
	CPoint	m_Origin;			// origin in screen coordinates
	CPoint	m_CursorPos;		// current cursor position
	CPoint	m_PrevCursor;		// previous cursor position
	DPoint	m_DampedOrg;		// damped origin; normalized from 0..1
	DPoint	m_TargetOrg;		// target origin for damping
	CPoint	m_PongDelta;		// origin delta for pong motion, in pixels
	double	m_LissTheta;		// Lissajous current theta
	CRealTimer	m_Metronome;	// accurate beat clock
	int		m_BeatCount;		// number of beats counted
	int		m_PrevBeatCount;	// previous number of beats

// Helpers
	static	void	TimerCallback(LPVOID Cookie);
};

inline void CMixerChan::GetBaseInfo(MIXER_CHAN_INFO& Info) const
{
	Info = *this;
}

inline void CMixerChan::SetBaseInfo(const MIXER_CHAN_INFO& Info)
{
	MIXER_CHAN_INFO&	mci = *this;	// downcast to base struct
	mci = Info;
}

#endif
