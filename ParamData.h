// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		01feb09	add color offset

        parameter data
 
*/

//		name			member			type	defval	minval	maxval		combo box info
PARMDEF(FRACTAL_TYPE,	m_FractalType,	INT,	0,		0,		1,			&m_FractalTypeComboInfo)
PARMDEF(IMAGE_WIDTH,	m_ImageSize.cx,	INT,	320,	2,		INT_MAX,	NULL)
PARMDEF(IMAGE_HEIGHT,	m_ImageSize.cy,	INT,	240,	2,		INT_MAX,	NULL)
PARMDEF(QUALITY,		m_Quality,		INT,	128,	1,		INT_MAX,	NULL)
PARMDEF(ANTIALIAS,		m_Antialias,	INT,	1,		1,		16,			NULL)
PARMDEF(CYCLE_LEN,		m_CycleLen,		INT,	256,	0,		INT_MAX,	NULL)
PARMDEF(COLOR_OFFSET,	m_ColorOffset,	DOUBLE,	0,		0,		1,			NULL)
PARMDEF(MATH_PREC,		m_MathPrec,		INT,	256,	64,		INT_MAX,	NULL)
PARMDEF(DEEP_ZOOM,		m_DeepZoom,		BYTE,	0,		0,		1,			&m_DeepZoomComboInfo)
PARMDEF(EXPONENT,		m_Exponent,		BYTE,	2,		2,		13,			NULL)
