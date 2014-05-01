// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		06feb09	add color cycling
		02		07feb09	add escape time array
		03		14feb09	avoid redundant storage of shared escape time arrays
		04		13mar09	replace image/escape times with snapshot

        project container
 
*/

#ifndef CPROJECT_INCLUDED
#define CPROJECT_INCLUDED

#include "HistoryItem.h"
#include "RecordInfo.h"
#include "Dib.h"

class CProject : public WObject
{
public:
// Construction
	CProject();
	CProject(const CProject& Proj);
	CProject& operator=(const CProject& Proj);

// Constants
	enum {
		ARCHIVE_SIG = 0x74637246,	// archive signature (Frct)
		ARCHIVE_VERSION = 1			// archive format version
	};

// Types

// Data members
	UINT	m_Version;			// format version number
	CHistory	m_History;		// history of previous view states
	int		m_HistPos;			// current history position, or -1 if none
	CSize	m_ThumbSize;		// history thumbnail size
	CRecordInfo	m_RecInfo;		// recording info
	CSnapshotPtr	m_Snapshot;	// view's current state; may be empty
	bool	m_CycleColors;		// true if we're cycling colors
	double	m_ColorCycleRate;	// color cycling rate, in cycles per second

// Attributes
	bool	HasSnapshot() const;

// Operations
	void	Serialize(CArchive& ar);
	void	Reset();

protected:
// Constants
	enum {
		ETAO_NONE = -1,			// escape time array default ownership value
	};

// Helpers
	void	Copy(const CProject& Proj);
	void	LoadHistory(CArchive& ar);
	void	StoreHistory(CArchive& ar);
};

inline CProject::CProject(const CProject& Proj)
{
	Copy(Proj);
}

inline CProject& CProject::operator=(const CProject& Proj)
{
	Copy(Proj);
	return(*this);
}

inline bool CProject::HasSnapshot() const
{
	return(!m_Snapshot.IsEmpty());
}

#endif
