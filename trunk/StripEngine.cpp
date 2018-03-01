// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		17dec09	replace strip job bool with strip mode
 
        fractal rendering engine with strip support
 
*/

#include "stdafx.h"
#include "StripEngine.h"

CStripEngine::CStripEngine()
{
	m_StripMode = SM_NORMAL_RENDER;
}

void CStripEngine::BeginStripJob(const RENDER_INFO& Info, const BigRect& Bounds)
{
	CancelRender(TRUE);
	m_Job = Info;
	m_Bounds = Bounds;
	m_StripMode = SM_STRIP_RENDER;
	m_CurUID = m_Job.FrameUID;
}

void CStripEngine::RenderStrip(UINT ThreadIdx, UINT y1, UINT y2, ESCTIME *FrameBuf)
{
	CRenderer&	rend = m_Render[ThreadIdx];
	rend.m_y1 = y1;
	rend.m_y2 = y2;
	rend.m_DeltaY = 1;
	rend.m_FrameBuf = FrameBuf;
	rend.StartWork();
}

void CStripEngine::OnRenderDone(CRenderer& Render)
{
	// virtual function override
	if (m_StripMode == SM_STRIP_RENDER) {	// if strip render
		if (!Render.IsCanceled())
			PostMessage(m_hWnd, UWM_STRIPDONE, m_CurUID, Render.m_ThrIdx);
	} else	// normal render or strip colormap
		CDibEngine::OnRenderDone(Render);	// do base class behavior
}
