// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version

        gradient point container
 
*/

#include "stdafx.h"
#include "GradientPt.h"

void CGradientPt::Invert(bool HLSMode)
{
	if (HLSMode) {
		DHLS	hls(m_Color.hls);
		hls.Invert();
		m_Color.hls = hls;
	} else {
		DRGB	rgb(m_Color.rgb);
		rgb.Invert();
		m_Color.rgb = rgb;
	}
}

void CGradientPt::RotateHue(bool HLSMode, double Rotation)
{
	if (HLSMode) {
		DHLS	hls(m_Color.hls);
		hls.RotateHue(Rotation);
		m_Color.hls = hls;
	} else {
		DRGB	rgb(m_Color.rgb);
		rgb.RotateHue(Rotation);
		m_Color.rgb = rgb;
	}
}
