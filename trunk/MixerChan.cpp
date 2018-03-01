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

#include "stdafx.h"
#include "MixerChan.h"
#include "Mixer.h"
#include <math.h>

CMixerChan::CMixerChan()
{
	MIXER_CHAN_INFO	*mci = this;
	ZeroMemory(mci, sizeof(MIXER_CHAN_INFO));
	m_Mixer = NULL;
	m_Origin = CPoint(0, 0);
	m_CursorPos = CPoint(0, 0);
	m_PrevCursor = CPoint(0, 0);
	m_DampedOrg = DPoint(0, 0);
	m_TargetOrg = DPoint(0, 0);
	m_PongDelta = CPoint(1, 1);
	m_LissTheta = 0;
	m_BeatCount = 0;
	m_PrevBeatCount = 0;
}

void CMixerChan::Metronome(bool Enable)
{
	if (Enable) {
		if (!m_Metronome.IsRunning()) {
			m_Metronome.Launch(TimerCallback, this, THREAD_PRIORITY_ABOVE_NORMAL);
			m_Metronome.Run(TRUE);
		}
		double	Freq = m_Tempo / 60;
		if (Freq != m_Metronome.GetFreq())	// if frequency changed
			m_Metronome.SetFreq(Freq, TRUE);	// set frequency and resync timer
	} else
		m_Metronome.Kill();
}

void CMixerChan::UpdateMetronome()
{
	Metronome(m_OrgMotion == CMixer::OM_RANDOM_JUMP);
}

void CMixerChan::TimerCallback(LPVOID Cookie)
{
	CMixerChan	*p = (CMixerChan *)Cookie;
	p->m_BeatCount++;	// trigger random jump
}

void CMixerChan::UpdateOrigin(CSize SrcSize)
{
	switch (m_OrgMotion) {
	case CMixer::OM_PING_PONG:
		m_Origin.x += m_PongDelta.x * m_PongSpeed;
		m_Origin.y += m_PongDelta.y * m_PongSpeed;
		if (m_Origin.x < 0) {
			m_Origin.x = -m_Origin.x;
			m_PongDelta.x = -m_PongDelta.x;
		} else if (m_Origin.x >= SrcSize.cx) {
			m_Origin.x = SrcSize.cx - 1 - (m_Origin.x - SrcSize.cx);
			m_PongDelta.x = -m_PongDelta.x;
		}
		if (m_Origin.y < 0) {
			m_Origin.y = -m_Origin.y;
			m_PongDelta.y = -m_PongDelta.y;
		} else if (m_Origin.y >= SrcSize.cy) {
			m_Origin.y = SrcSize.cy - 1 - (m_Origin.y - SrcSize.cy);
			m_PongDelta.y = -m_PongDelta.y;
		}
		// clamp origin just to be safe
		m_Origin.x = CLAMP(m_Origin.x, 0, SrcSize.cx - 1);
		m_Origin.y = CLAMP(m_Origin.y, 0, SrcSize.cy - 1);
		break;
	case CMixer::OM_CURSOR_DRAG:
		{
			CPoint	cp;
			GetCursorPos(&cp);	// get cursor position
			m_CursorPos += cp - m_PrevCursor;	// add delta to our position
			m_PrevCursor = cp;	// use delta to avoid cursor wind-up
			// keep position within area of current display device
			m_CursorPos.x = CLAMP(m_CursorPos.x, 0, m_Mixer->m_DispSize.cx);
			m_CursorPos.y = CLAMP(m_CursorPos.y, 0, m_Mixer->m_DispSize.cy);
			// compute new target origin in normalized coords
			m_TargetOrg.x = double(m_CursorPos.x) / m_Mixer->m_DispSize.cx;
			m_TargetOrg.y = double(m_CursorPos.y) / m_Mixer->m_DispSize.cy;
		}
		goto UpdateDampOrg;
	case CMixer::OM_MIDI_DRAG:
		goto UpdateDampOrg;
	case CMixer::OM_RANDOM_JUMP:
		if (m_BeatCount != m_PrevBeatCount) {
			m_PrevBeatCount = m_BeatCount;
			m_TargetOrg.x = double(rand()) / RAND_MAX;
			m_TargetOrg.y = double(rand()) / RAND_MAX;
		}
UpdateDampOrg:
		// update damped origin
		m_DampedOrg -= (m_DampedOrg - m_TargetOrg) * m_Damping;
		m_Origin.x = round(m_DampedOrg.x * (SrcSize.cx - 1));
		m_Origin.y = round(m_DampedOrg.y * (SrcSize.cy - 1));
		break;
	case CMixer::OM_LISSAJOUS:
		{
			double delta = m_LissSpeed;
			DPoint	p(
				sin(m_LissTheta * m_LissXFreq + m_LissPhase * PI),
				sin(m_LissTheta * m_LissYFreq)
			);
			m_LissTheta += delta;
			m_Origin.x = round((p.x + 1) / 2 * (SrcSize.cx - 1));
			m_Origin.y = round((p.y + 1) / 2 * (SrcSize.cy - 1));
		}
		break;
	default:
		m_Origin = CPoint(0, 0);
	}
	ASSERT(m_Origin.x >= 0 && m_Origin.x <= SrcSize.cx - 1);
	ASSERT(m_Origin.y >= 0 && m_Origin.y <= SrcSize.cy - 1);
}
