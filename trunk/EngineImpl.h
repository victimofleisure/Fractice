// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
        01      07jan10 separate from engine

        engine implementation macros
 
*/

#pragma once

#include "Engine.h"
#include <math.h>

#if _MFC_VER > 0x0600
#include "xmmintrin.h"	// for SSE types
#include "intrin.h"		// for __cpuid
#define	__APPLE_API_PRIVATE
#define	ASSEMBLER
#include "cpuid.h"		// for CPUID defs
#endif

#define PROLOGUE(real_t, Job, Bounds)	\
	m_CurRow = 0;						\
	UINT	Quality = Job.Quality;		\
	UINT	Width = Job.FrameSize.cx;	\
	UINT	Height = Job.FrameSize.cy;	\
	UINT	y1 = m_y1;					\
	UINT	y2 = m_y2;					\
	UINT	dy = m_DeltaY;				\
	ESCTIME	*pFrame = m_FrameBuf;		\
	int	RowDelta = (dy - 1) * Job.FrameSize.cx;			\
	real_t	da((Bounds.x2 - Bounds.x1) / (Width - 1));	\
	real_t	db((Bounds.y2 - Bounds.y1) / (Height - 1));	\
	real_t	b(Bounds.y1 + db * real_t(y1));				\
	db *= dy;

#define MAND_EXP_HDR								\
	double	a, zx, zy, zx2, zy2;					\
	double	bx1 = m_Engine->m_Bounds.x1;			\
	for (UINT y = y1; y < y2; y += dy) {			\
		m_CurRow = y;								\
		if (IsCanceled())							\
			break;									\
		a = bx1;									\
		for (UINT x = 0; x < Width; x++) {			\
			zx = a;									\
			zy = b;									\
			ESCTIME	i;								\
			for (i = 0; i < Quality; i++) {			\
				zx2 = zx * zx;						\
				zy2 = zy * zy;						\
				if (zx2 + zy2 >= 4.0)				\
					break;

#define MAND_EXP_FTR								\
			}										\
			*pFrame++ = i;							\
			a += da;								\
		}											\
		pFrame += RowDelta;							\
		b += db;									\
	}

#define SQRC(a, b) zx##a = zx##b * zx; zy##a = zy##b * zy;
