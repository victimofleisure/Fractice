// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		12jan09	add bit count
		02		06feb09	add color cycling
		03		11jul09	allow unlimited record points

        record info container
 
*/

#include "stdafx.h"
#include "RecordInfo.h"

const double CRecordInfo::DEF_COLOR_CYCLE_RATE = .031;

CRecordInfo::CRecordInfo()
{
	Reset();
}

void CRecordInfo::Copy(const CRecordInfo& Info)
{
	m_Duration			= Info.m_Duration;
	m_FrameCount		= Info.m_FrameCount;
	m_StartFrame		= Info.m_StartFrame;
	m_EndFrame			= Info.m_EndFrame;
	m_FrameRate			= Info.m_FrameRate;
	m_BitCount			= Info.m_BitCount;
	m_ToBitmaps			= Info.m_ToBitmaps;
	m_CycleColors		= Info.m_CycleColors;
	m_ColorCycleRate	= Info.m_ColorCycleRate;
	m_Snap.Copy(Info.m_Snap);
}

void CRecordInfo::Reset()
{
	m_Duration = 0;
	m_FrameCount = 0;
	m_StartFrame = 0;
	m_EndFrame = 0;
	m_FrameRate = 0;
	m_BitCount = DEF_BIT_COUNT;
	m_ToBitmaps = FALSE;
	m_CycleColors = FALSE;
	m_ColorCycleRate = DEF_COLOR_CYCLE_RATE;
	m_Snap.RemoveAll();
}

void CRecordInfo::Serialize(CArchive &ar)
{
	if (ar.IsStoring()) {
		ar << m_Duration;
		ar << m_FrameCount;
		ar << m_StartFrame;
		ar << m_EndFrame;
		ar << m_FrameRate;
		ar << m_BitCount;
		StoreBool(ar, m_ToBitmaps);
		StoreBool(ar, m_CycleColors);
		ar << m_ColorCycleRate;
		// m_Snap must have at least two elements for backwards compatibility
		int	snaps = m_Snap.GetSize();
		m_Snap.SetSize(max(snaps, 2));	// pad with extra elements if needed
		m_Snap.Serialize(ar);
		m_Snap.SetSize(snaps);	// restore original size
	} else {	// loading
		ar >> m_Duration;
		ar >> m_FrameCount;
		ar >> m_StartFrame;
		ar >> m_EndFrame;
		ar >> m_FrameRate;
		ar >> m_BitCount;
		LoadBool(ar, m_ToBitmaps);
		LoadBool(ar, m_CycleColors);
		ar >> m_ColorCycleRate;
		m_Snap.Serialize(ar);
		// remove any empty elements from m_Snap
		int	i = 0;
		while (i < m_Snap.GetSize()) {
			if (m_Snap[i].IsEmpty())	// if snap is empty
				m_Snap.RemoveAt(i);	// remove it
			else	// not empty
				i++;	// skip it
		}
	}
}
