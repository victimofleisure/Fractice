// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*	
		chris korda

		revision history:
		rev		date	comments
		00		16feb09	initial version

		container for DirectDraw enumerated object

*/

#ifndef CDDENUMOBJ_INCLUDED
#define CDDENUMOBJ_INCLUDED

#include "ArrayEx.h"

class CDDEnumObj : public CObject {
public:
// Construction
	CDDEnumObj();
	CDDEnumObj(const CDDEnumObj& Info);
	CDDEnumObj& operator=(const CDDEnumObj& Info);

// Data members
	GUID	m_Guid;			// unique identifier of the DirectDraw object
	CString	m_DriverDescription; // driver name
	CString	m_DriverName;	// driver description
	HMONITOR	m_hMonitor;	// handle to monitor associated with DirectDraw object

// Attributes
	void	GetScreenCoords(CRect& rc);

// Helpers
	void	Copy(const CDDEnumObj& Info);
};

inline CDDEnumObj::CDDEnumObj(const CDDEnumObj& Info)
{
	Copy(Info);
}

inline CDDEnumObj& CDDEnumObj::operator=(const CDDEnumObj& Info)
{
	Copy(Info);
	return(*this);
}

typedef CArrayEx<CDDEnumObj, CDDEnumObj&> CDDEnumObjArray;

#endif
