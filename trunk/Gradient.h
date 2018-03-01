// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		05feb09	handle wraparound

        gradient container
 
*/

#ifndef CGRADIENT_INCLUDED
#define CGRADIENT_INCLUDED

#include "ArrayEx.h"
#include "GradientPt.h"

typedef CArrayEx<CGradientPt, CGradientPt&> CGradientPtArray;

enum {	// color modes
	GRADIENT_RGB,
	GRADIENT_HLS
};

class CGradient : public CGradientPtArray {
	DECLARE_SERIAL(CGradient);
public:
// Construction
	CGradient();
	CGradient(const CGradient& Grad);
	CGradient& operator=(const CGradient& Grad);

// Comparison
	bool	operator==(const CGradient& Grad) const;
	bool	operator!=(const CGradient& Grad) const;

// Operations
	void	Serialize(CArchive& ar);
	int		FindPos(double Pos, int StartIdx = 0) const;
	int		ReverseFindPos(double Pos, int StartIdx = -1) const;
	void	SpaceEvenly(bool SmoothWrap = TRUE);
	void	Invert();
	void	RotateHue(double Rotation);

// Data members
	int		m_ColorMode;	// color mode; RGB or HLS, see enum above
	bool	m_SpaceEvenly;	// true if points should be spaced evenly

// Helpers
	void	Copy(const CGradient& Grad);
};

inline CGradient::CGradient()
{
	m_ColorMode = GRADIENT_RGB;
	m_SpaceEvenly = FALSE;
}

inline bool CGradient::operator!=(const CGradient& Grad) const
{
	return(!(*this == Grad));
}

class CGradientIter {
public:
	CGradientIter(const CGradient& Grad);
	void	Interpolate(double Pos, DRGB& Color);

protected:
	const CGradient& m_Grad;	// array of gradient points
	int		m_Idx;		// current position in point array
};

#endif
