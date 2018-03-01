// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		23jul09	add Mandelbar

        types used by fractal rendering engine
 
*/

#ifndef ENGINETYPES_INCLUDED
#define ENGINETYPES_INCLUDED

enum {	// fractal types
	// This list is APPEND ONLY; do not delete or reorder its entries,
	// else documents containing fractal type will become incompatible.
	//
	FT_MANDELBROT,
	FT_NEWTON,
	FT_MANDELBAR,
	//
	// end of list
	//
	FRACTAL_TYPES,			// total number of types
	FT_NO_RENDER = INT_MAX	// for color mapping only
};

typedef UINT ESCTIME;		// escape time

typedef struct tagRENDER_INFO {	// rendering information
	ESCTIME	*FrameBuf;		// pointer to escape time frame buffer
	UINT	FractalType;	// fractal function
	SIZE	FrameSize;		// frame buffer dimensions, in pixels
	UINT	FrameUID;		// frame's unique indentifier
	UINT	Quality;		// number of iterations before escape
	bool	DeepZoom;		// true if using multi-precision math
	BYTE	Exponent;		// exponent to raise complex Z to
} RENDER_INFO;

#endif
