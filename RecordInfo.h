// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		11jan09	add IsPoster
		02		12jan09	add bit count
		03		06feb09	add color cycling
		04		11jul09	allow unlimited record points

        record info container
 
*/

#ifndef CRECORDINFO_INCLUDED
#define CRECORDINFO_INCLUDED

#include "Snapshot.h"

class CRecordInfo : public WObject {
public:
// Construction
	CRecordInfo();
	CRecordInfo(const CRecordInfo& Info);
	CRecordInfo& operator=(const CRecordInfo& Info);

// Constants
	enum {
		DEF_BIT_COUNT = 24,		// default color depth, in bits
	};
	static const double DEF_COLOR_CYCLE_RATE;	// in cycles per second

// Data members
	// Note that this object can also store poster info, in which case m_Snap
	// contains a single snapshot, m_BitCount contains poster's color depth,
	// m_FrameRate contains the poster's DPI, and all other members are zero.
	int		m_Duration;			// recording length, in seconds
	UINT	m_FrameCount;		// recording length, in frames
	UINT	m_StartFrame;		// index of first frame
	UINT	m_EndFrame;			// index of last frame
	float	m_FrameRate;		// frame rate, in frames per second, or poster DPI
	WORD	m_BitCount;			// color depth, in bits
	bool	m_ToBitmaps;		// if true, record to bitmaps
	bool	m_CycleColors;		// true if we're cycling colors
	double	m_ColorCycleRate;	// color cycling rate, in cycles per second
	CSnapshotArray	m_Snap;		// snapshots of start and end points

// Attributes
	bool	IsPoster() const;

// Operations
	void	Serialize(CArchive &ar);
	void	Reset();

protected:
// Helpers
	void	Copy(const CRecordInfo& Info);
};

inline CRecordInfo::CRecordInfo(const CRecordInfo& Info)
{
	Copy(Info);
}

inline CRecordInfo& CRecordInfo::operator=(const CRecordInfo& Info)
{
	Copy(Info);
	return(*this);
}

inline bool CRecordInfo::IsPoster() const
{
	return(m_Snap.GetSize() == 1);
}

#endif
