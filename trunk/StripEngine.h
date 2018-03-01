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

#ifndef CSTRIPENGINE_INCLUDED
#define CSTRIPENGINE_INCLUDED

#include "DibEngine.h"

class CStripEngine : public CDibEngine {
public:
// Construction
	CStripEngine();

// Constants
	enum {	// strip modes
		SM_NORMAL_RENDER,
		SM_STRIP_RENDER,
		SM_STRIP_COLORMAP,
	};

// Attributes
	int		GetStripMode() const;
	bool	IsStripJob() const;

// Operations
	void	RenderFrame(const RENDER_INFO& Info, const BigRect& Bounds);
	void	BeginStripJob(const RENDER_INFO& Info, const BigRect& Bounds);
	void	RenderStrip(UINT ThreadIdx, UINT y1, UINT y2, ESCTIME *FrameBuf);

protected:
// Member data
	int		m_StripMode;	// strip mode; see enum above

// Overrides
	void	OnRenderDone(CRenderer& Render);
};

inline int CStripEngine::GetStripMode() const
{
	return(m_StripMode);
}

inline bool CStripEngine::IsStripJob() const
{
	return(m_StripMode != SM_NORMAL_RENDER);
}

inline void CStripEngine::RenderFrame(const RENDER_INFO& Info, const BigRect& Bounds)
{
	m_StripMode = SM_NORMAL_RENDER;
	CDibEngine::RenderFrame(Info, Bounds);	// do base class behavior
}

#endif
