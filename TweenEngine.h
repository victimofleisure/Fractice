// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      21feb09	initial version
		01		01mar09	cycle old palette during tween
		02		08mar09	add MixDibs

        fractal rendering engine with palette tweening
 
*/

#ifndef CTWEENENGINE_INCLUDED
#define CTWEENENGINE_INCLUDED

#include "NetEngine.h"

class CTweenEngine : public CNetEngine {
public:
// Construction
	CTweenEngine();

// Attributes
	void	SetPalette(const DPalette& Palette, UINT Quality, UINT CycleLen, double Offset);
	bool	IsTweening() const;
	void	SetTweening(bool Enable);

// Operations
	void	ResetPalette();
	void	UpdatePalette(const DPalette& Palette, UINT CycleLen, double Offset);
	void	UpdateTweenPalette(const DPalette& Palette, UINT CycleLen, double Offset);
	void	TweenPalette();
	void	MapColorEx(CDib& Dib, const DIB_INFO& Info, bool Mirror, CPoint Origin);
	static	void	MixDibs(CDib& Dst, CDib& Src, const DIB_INFO& Info, double MixPos);

protected:
// Member data
	CDWordArray	m_OldPal;	// palette we're tweening from
	CDWordArray	m_NewPal;	// palette we're tweening to
	CDWordArray	m_TweenPal;	// tween output palette
	UINT	m_Quality;		// current quality
	bool	m_Tweening;		// true if we're tweening palette
	double	m_TweenPos;		// tweening position; 0 == old, 1 == new
	double	m_TweenDelta;	// tweening amount per frame
	UINT	m_CycleLen;		// current cycle length
	UINT	m_OldCycleLen;	// old palette's cycle length
	double	m_ColorOffset;	// current color offset
	double	m_InitColorOfs;	// color offset at start of tween
};

inline bool CTweenEngine::IsTweening() const
{
	return(m_Tweening);
}

inline void CTweenEngine::ResetPalette()
{
	m_Palette.RemoveAll();
}

#endif
