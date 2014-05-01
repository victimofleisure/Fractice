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

#ifndef GRADIENTPT_INCLUDED
#define GRADIENTPT_INCLUDED

#include "DColor.h"

typedef union tagGRADIENT_COLOR {
	DRGB_S	rgb;	// RGB color
	DHLS_S	hls;	// HLS color
	bool	operator==(const tagGRADIENT_COLOR& pt) const;
	bool	operator!=(const tagGRADIENT_COLOR& pt) const;
} GRADIENT_COLOR;

typedef struct tagGRADIENT_POINT {
	double	m_Pos;				// position, normalized to [0..1]
	GRADIENT_COLOR	m_Color;	// RGB or HLS color value
} GRADIENT_POINT;

class CGradientPt : public GRADIENT_POINT {
public:
// Construction
	CGradientPt();
	CGradientPt(const CGradientPt& pt);
	CGradientPt(const GRADIENT_POINT& pt);
	CGradientPt(double Pos, const DRGB& Color);
	CGradientPt(double Pos, const DHLS& Color);
	CGradientPt(double Pos, COLORREF Color);
	CGradientPt& operator=(const CGradientPt& pt);

// Attributes
	bool	operator==(const CGradientPt& pt) const;
	bool	operator!=(const CGradientPt& pt) const;

// Operations
	void	Invert(bool HLSMode);
	void	RotateHue(bool HLSMode, double Rotation);

// Helpers
	void	Copy(const CGradientPt& pt);
};

inline CGradientPt::CGradientPt()
{
}

inline void CGradientPt::Copy(const CGradientPt& pt)
{
	memcpy(this, &pt, sizeof(CGradientPt));	// bitwise copy
}

inline CGradientPt::CGradientPt(const CGradientPt& pt)
{
	Copy(pt);
}

inline CGradientPt::CGradientPt(const GRADIENT_POINT& pt)
{
	GRADIENT_POINT&	gp = *this;
	gp = pt;
}

inline CGradientPt::CGradientPt(double Pos, const DRGB& Color)
{
	m_Pos = Pos;
	m_Color.rgb = Color;
}

inline CGradientPt::CGradientPt(double Pos, const DHLS& Color)
{
	m_Pos = Pos;
	m_Color.hls = Color;
}

inline CGradientPt::CGradientPt(double Pos, COLORREF Color)
{
	m_Pos = Pos;
	DRGB	rgb(Color);
	m_Color.rgb = rgb;
}

inline CGradientPt& CGradientPt::operator=(const CGradientPt& pt)
{
	Copy(pt);
	return(*this);
}

inline bool GRADIENT_COLOR::operator==(const GRADIENT_COLOR& pt) const
{
	return(!memcmp(this, &pt, sizeof(GRADIENT_COLOR)));	// bitwise compare
}

inline bool GRADIENT_COLOR::operator!=(const GRADIENT_COLOR& pt) const
{
	return(!(*this == pt));
}

inline bool CGradientPt::operator==(const CGradientPt& pt) const
{
	return(!memcmp(this, &pt, sizeof(CGradientPt)));	// bitwise compare
}

inline bool CGradientPt::operator!=(const CGradientPt& pt) const
{
	return(!(*this == pt));
}

#endif
