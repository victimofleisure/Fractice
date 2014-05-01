// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
        01      07jan10	separate from engine

        Newton rendering
 
*/

#include "stdafx.h"
#include "EngineImpl.h"

void CEngine::CRenderer::NewtonFPU()
{
	PROLOGUE(double, m_Engine->m_Job, m_Engine->m_Bounds)
	double	a, zx, zy;
	double	bx1 = m_Engine->m_Bounds.x1;
	for (UINT y = y1; y < y2; y += dy) {
		m_CurRow = y;
		if (IsCanceled())
			break;
		a = bx1;
		for (UINT x = 0; x < Width; x++) {
			zx = a;
			zy = b;
			ESCTIME	i;
			for (i = 0; i < Quality; i++) {
				double	zx2 = zx * zx;
				double	zy2 = zy * zy;
				double	n = 1 / (3 * (zx2 + zy2) * (zx2 + zy2));
				double	zxn = (zx / 3 - (zx2 - zy2) * n);
				double	zyn = (zy / 3 + 2 * zx * zy * n);
				if (zxn * zxn + zyn * zyn < 1e-6)
					break;
				zx -= zxn;
				zy -= zyn;
			}
			*pFrame++ = i;
			a += da;
		}
		pFrame += RowDelta;
		b += db;
	}
}
