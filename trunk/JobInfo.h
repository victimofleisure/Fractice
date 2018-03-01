// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*	
		chris korda

		revision history:
		rev		date	comments
		00		02aug07	initial version
        01      23oct08	customize
		02		06jan10	W64: in SerializeElements, nCount is 64-bit

		container for job information

*/

#ifndef CJOBINFO_INCLUDED
#define CJOBINFO_INCLUDED

#include "RecordInfo.h"
#include "VideoComprState.h"

class CJobInfo : public CObject {
public:
	DECLARE_SERIAL(CJobInfo);

// Construction
	CJobInfo();
	CJobInfo(const CJobInfo& Info);
	CJobInfo& operator=(const CJobInfo& Info);

// Types

// Constants
	enum {
		ARCHIVE_VERSION = 1			// archive version number
	};

// Public data
	// REMEMBER to add new members to ctor, Copy, and Serialize
	CString	m_RecordPath;	// path to destination file
	CString	m_Name;			// job title
	CString	m_Source;		// source file name
	CString	m_Dest;			// destination file name
	CTime	m_Start;		// when job began
	CTime	m_End;			// when job ended
	int		m_Status;		// see status enum above
	int		m_ID;			// unique identifier
	CString	m_ErrorMsg;		// error message if any
	CRecordInfo	m_RecInfo;	// record information
	CVideoComprState	m_ComprState;	// compressor state

// Operations
	void	Serialize(CArchive& ar);

protected:
// Member data
	UINT	m_Version;		// archive version number

// Helpers
	void	Copy(const CJobInfo& Info);
};

inline CJobInfo::CJobInfo(const CJobInfo& Info)
{
	Copy(Info);
}

inline CJobInfo& CJobInfo::operator=(const CJobInfo& Info)
{
	Copy(Info);
	return(*this);
}

template<> inline void AFXAPI
SerializeElements<CJobInfo>(CArchive& ar, CJobInfo* pObj, W64INT nCount)
{
    for (int i = 0; i < nCount; i++, pObj++)
        pObj->Serialize(ar);
}

#endif
