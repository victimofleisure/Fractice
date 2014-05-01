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

#ifndef CRECTTRACKEREX_INCLUDED
#define CRECTTRACKEREX_INCLUDED

class CRectTrackerEx : public CRectTracker {
public:
// Construction
	CRectTrackerEx();

protected:
// Overrides
	void	AdjustRect(int nHandle, LPRECT lpRect);	// override

// Member data
	bool	m_First;	// true the first time AdjustRect is called
};

#endif
