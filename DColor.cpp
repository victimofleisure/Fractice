// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
		00		17oct07	initial version
		01		08nov07	move 360 degree normalization here
		02		09mar08	wrap out-of-range hues

		double-precision RGB and HLS color containers
 
*/

#include "stdafx.h"
#include "DColor.h"
#include "hls.h"
#include <math.h>

DRGB::DRGB(COLORREF Color)
{
	m_r = GetRValue(Color) / 255.0;
	m_g = GetGValue(Color) / 255.0;
	m_b = GetBValue(Color) / 255.0;
}

DRGB::operator COLORREF() const
{
	return(RGB(round(m_r * 255), round(m_g * 255), round(m_b * 255)));
}

void DRGB::Invert()
{
	m_r = 1 - m_r;
	m_g = 1 - m_g;
	m_b = 1 - m_b;
}

void DRGB::RotateHue(double Rotation)
{
	DHLS	hls;
	rgb2hls(*this, hls);
	hls.RotateHue(Rotation);
	hls2rgb(hls, *this);
}

DHLS::DHLS(COLORREF Color)
{
	DRGB	rgb;
	rgb.m_r = GetRValue(Color) / 255.0;
	rgb.m_g = GetGValue(Color) / 255.0;
	rgb.m_b = GetBValue(Color) / 255.0;
	rgb2hls(rgb, *this);
}

DHLS::operator COLORREF() const
{
	DRGB	rgb;
	hls2rgb(*this, rgb);
	return(RGB(round(rgb.m_r * 255), round(rgb.m_g * 255), round(rgb.m_b * 255)));
}

void DHLS::Invert()
{
	m_h = fmod(m_h + .5, 2);	// preserve hue deltas up to 720 degrees
	m_l = 1 - m_l;
}

void DHLS::RotateHue(double Rotation)
{
	m_h = fmod(m_h + Rotation, 2);	// preserve hue deltas up to 720 degrees
}

void hls2rgb(const DHLS_S& hls, DRGB_S& rgb)
{
	double	h = fmod(hls.m_h, 1);
	if (h < 0)
		h = h + 1;
	CHLS::hls2rgb(h * 360, hls.m_l, hls.m_s, rgb.m_r, rgb.m_g, rgb.m_b);
}

void rgb2hls(const DRGB_S& rgb, DHLS_S& hls)
{
	CHLS::rgb2hls(rgb.m_r, rgb.m_g, rgb.m_b, hls.m_h, hls.m_l, hls.m_s);
	hls.m_h /= 360;
}
