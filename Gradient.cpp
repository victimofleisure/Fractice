// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		01feb09	in Interpolate, handle descending positions
		02		05feb09	handle wraparound

        gradient container
 
*/

#include "stdafx.h"
#include "Gradient.h"

IMPLEMENT_SERIAL(CGradient, CObject, 1)

void CGradient::Copy(const CGradient& Grad)
{
	CArrayEx<CGradientPt, CGradientPt&>::Copy(Grad);	// copy base class
	m_ColorMode = Grad.m_ColorMode;
	m_SpaceEvenly = Grad.m_SpaceEvenly;
}

void CGradient::Serialize(CArchive& ar)
{
	CArrayEx<CGradientPt, CGradientPt&>::Serialize(ar);	// serialize base class
	if (ar.IsStoring()) {
		ar << m_ColorMode;
		StoreBool(ar, m_SpaceEvenly);
	} else {
		ar >> m_ColorMode;
		LoadBool(ar, m_SpaceEvenly);
	}
}

bool CGradient::operator==(const CGradient& Grad) const
{
	int	pts = GetSize();
	if (Grad.GetSize() != pts)
		return(FALSE);
	for (int i = 0; i < pts; i++) {
		if (Grad[i] != ElementAt(i))
			return(FALSE);
	}
	return(Grad.m_ColorMode == m_ColorMode
		&& Grad.m_SpaceEvenly == m_SpaceEvenly);
}

int CGradient::FindPos(double Pos, int StartIdx) const
{
	int	pts = GetSize();
	for (int i = StartIdx; i < pts; i++) {
		if (ElementAt(i).m_Pos >= Pos)
			return(i);
	}
	return(pts);	// position not found
}

int CGradient::ReverseFindPos(double Pos, int StartIdx) const
{
	int	pts = GetSize();
	if (StartIdx < 0)
		StartIdx = pts - 1;
	for (int i = StartIdx; i >= 0; i--) {
		if (ElementAt(i).m_Pos <= Pos)
			return(i + 1);
	}
	return(0);	// position not found
}

void CGradient::SpaceEvenly(bool SmoothWrap)
{
	int	pts = GetSize();
	if (pts > 0) {
		if (!SmoothWrap) {	// if not smooth wraparound
			pts--;
			ElementAt(pts).m_Pos = 1;	// move last point to end of gradient
		}
		for (int i = 0; i < pts; i++)
			ElementAt(i).m_Pos = i / double(pts);
	}
}

void CGradient::Invert()
{
	int	pts = GetSize();
	bool	HLSMode = m_ColorMode == GRADIENT_HLS;
	for (int i = 0; i < pts; i++)
		ElementAt(i).Invert(HLSMode);
}

void CGradient::RotateHue(double Rotation)
{
	int	pts = GetSize();
	bool	HLSMode = m_ColorMode == GRADIENT_HLS;
	for (int i = 0; i < pts; i++)
		ElementAt(i).RotateHue(HLSMode, Rotation);
}

CGradientIter::CGradientIter(const CGradient& Grad) :
	m_Grad(Grad)
{
	m_Idx = 1;
}

void CGradientIter::Interpolate(double Pos, DRGB& Color)
{
	int	pts = m_Grad.GetSize();
	if (pts <= 0)	// if no points
		return;	// nothing to do
	GRADIENT_COLOR	ci;
	if (pts > 1) {
		// search for pair of points that contains given position
		if (Pos < m_Grad[m_Idx].m_Pos) {	// if we're too high
			while (m_Idx > 1 && Pos < m_Grad[m_Idx - 1].m_Pos)	// search down
				m_Idx--;
		} else {	// we're too low
			while (Pos >= m_Grad[m_Idx].m_Pos && m_Idx < pts - 1)	// search up
				m_Idx++;
		}
		const CGradientPt	*p0 = &m_Grad[m_Idx - 1];	// start of pair
		const CGradientPt	*p1 = &m_Grad[m_Idx];	// end of pair
		double	pos0 = p0->m_Pos;
		double	pos1 = p1->m_Pos;
		// if given position isn't within pair, handle wraparound
		if (Pos < pos0) {	// if below pair
			p1 = p0;
			pos1 = pos0;
			p0 = &m_Grad[pts - 1];	// wrap start to last point
			pos0 = p0->m_Pos - 1;
		} else if (Pos > pos1) {	// if above pair
			p0 = p1;
			pos0 = pos1;
			p1 = &m_Grad[0];	// wrap end to first point
			pos1 = p1->m_Pos + 1;
		}
		// compute scaling factor
		double	scale;
		double	delta = pos1 - pos0;
		if (delta)	// avoid divide by zero
			scale = (Pos - pos0) / delta;
		else
			scale = 0;
		// interpolate color
		const DRGB_S&	c0 = p0->m_Color.rgb; 
		const DRGB_S&	c1 = p1->m_Color.rgb; 
		ci.rgb.m_r = c0.m_r + (c1.m_r - c0.m_r) * scale;
		ci.rgb.m_g = c0.m_g + (c1.m_g - c0.m_g) * scale;
		ci.rgb.m_b = c0.m_b + (c1.m_b - c0.m_b) * scale;
	} else
		ci.rgb = m_Grad[0].m_Color.rgb;
	if (m_Grad.m_ColorMode == GRADIENT_RGB)
		Color = ci.rgb;
	else	// HLS mode
		hls2rgb(ci.hls, Color);	// convert to RGB
}
