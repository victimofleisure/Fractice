// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version

        derived snapshot container
 
*/

#ifndef CMYSNAP_INCLUDED
#define CMYSNAP_INCLUDED

#include "Snapshot.h"

class CMySnap : public CSnapshot {
public:
// Construction
	CMySnap();
	CMySnap(const CMySnap& Snap);
	CMySnap& operator=(const CMySnap& Snap);

// Member data
	UINT	m_JobID;		// identifies this rendering job
	UINT	m_FrameID;		// identifies frame within this job

protected:
// Helpers
	void	Copy(const CMySnap& Snap);
};

inline CMySnap::CMySnap(const CMySnap& Snap)
{
}

inline CMySnap& CMySnap::operator=(const CMySnap& Snap)
{
	Copy(Snap);
	return(*this);
}

typedef CRefPtr<CMySnap> CMySnapPtr;
typedef	CArrayEx<CMySnapPtr, CMySnapPtr&> CMySnapList;

#endif
