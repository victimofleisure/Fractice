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

#include "stdafx.h"
#include "MySnap.h"

CMySnap::CMySnap()
{
	m_JobID = 0;
	m_FrameID = 0;
}

void CMySnap::Copy(const CMySnap& Snap)
{
	CSnapshot::Copy(Snap);
	m_JobID = Snap.m_JobID;
	m_FrameID = Snap.m_FrameID;
}



