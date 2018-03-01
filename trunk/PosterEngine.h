// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      11jan09	initial version
		01		16jan09	add pause/resume

        fractal rendering engine for posters
 
*/

#ifndef CPOSTERENGINE_INCLUDED
#define CPOSTERENGINE_INCLUDED

#include "NetEngine.h"

class CPosterEngine : public CNetEngine {
public:
// Construction
	CPosterEngine();

// Attributes
	CString	GetRowFilePath() const;
	bool	IsRowFileOpen() const;
	void	GetRowIndex(CDWordArray& RowIndex) const;

// Operations
	// note that these functions can throw CFileException
	void	BeginPosterJob(const RENDER_INFO& Info, const BigRect& Bounds, const CSnapshot& Snap, LPCTSTR RowFilePath);
	bool	WritePoster(LPCTSTR BitmapPath, WORD BitCount, float Resolution);
	void	EndPosterJob();
	bool	ResumePosterJob(const CDWordArray& RowIndex);

protected:
// Constants

// Member data
	CString	m_RowFilePath;		// path of file to receive rows
	UINT	m_PosterAntialias;	// poster's antialiasing factor
	bool	m_ResumingJob;		// true if we're resuming a paused job

// Helpers
	static	void	WriteDibHeader(CFile& File, const BITMAP& Bmp, float Resolution);
	static	void	FileSeek64(CFile& File, __int64 llOff, UINT nFrom);
};

inline CString CPosterEngine::GetRowFilePath() const
{
	return(m_RowFilePath);
}

inline bool CPosterEngine::IsRowFileOpen() const
{
	return(m_RowFile.m_hFile != CFile::hFileNull);
}

inline void CPosterEngine::GetRowIndex(CDWordArray& RowIndex) const
{
	RowIndex.Copy(m_RowIndex);
}

#endif
