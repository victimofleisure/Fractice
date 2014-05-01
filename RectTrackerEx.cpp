// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version

		rubber-band that ignores first mouse event
 
*/

#include "stdafx.h"
#include "RectTrackerEx.h"

CRectTrackerEx::CRectTrackerEx()
{
	m_First = TRUE;
}

void CRectTrackerEx::AdjustRect(int nHandle, LPRECT lpRect)
{
	if (m_First)
		m_First = FALSE;	// eat first call to avoid spurious rendering
	else
		CRectTracker::AdjustRect(nHandle, lpRect);
}
