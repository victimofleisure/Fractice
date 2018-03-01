// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*	
		chris korda

		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		05jan09	add cache images flag
		02		01mar09	add scroll delta
		03		06jan10	W64: in SerializeElements, nCount is 64-bit

		container for options information

*/

#ifndef COPTIONSINFO_INCLUDED
#define COPTIONSINFO_INCLUDED

typedef struct tagBASE_OPTIONS_INFO {
	SIZE	m_ThumbSize;	// thumbnail size
	double	m_ZoomStep;		// zoom in/out scaling factor
	double	m_ContZoomStep;	// continuous zoom scaling factor
	int		m_ScrollDelta;	// scrolling delta, in logical coords
	BOOL	m_ThreadsAuto;	// true if thread count should equal CPU count
	UINT	m_ThreadCount;	// number of rendering threads
	int		m_DefDocFmt;	// default document format; see enum in CFracticeDoc
	BOOL	m_UndoUnlimited;// true if undo is unlimited
	int		m_UndoLevels;	// number of undo levels
	BOOL	m_SaveChgsWarn;	// true if save changes warning is enabled
	BOOL	m_UseSSE2;		// true if using SSE2 instructions
	UINT	m_NetworkPort;	// network port number for TCP/UDP
	BOOL	m_NetDefPort;	// true if using default network port
	COLORREF	m_GridColor;	// color for drawing coordinate grid
	BOOL	m_ListenServers;	// true if listening for incoming server connections
	BOOL	m_CacheImages;	// true if history caches full-size images
	BOOL	m_SaveEscTimes;	// true if saving escape times
	BOOL	m_PalFirstBkgnd;	// true if palette's first entry is background color
	BOOL	m_PalSmoothWrap;	// true if palette wraps around smoothly
	UINT	m_PalExportSize;	// number of palette entries to export
	int		m_HueRotation;	// hue rotation amount, in degrees
	UINT	m_MaxMRUParams;	// maximum size of recently used parameter list
	UINT	m_MaxMRUPalettes;	// maximum size of recently used palette list
	UINT	m_FrameRate;	// display frame rate, in cycles per second
} BASE_OPTIONS_INFO;

class COptionsInfo : public BASE_OPTIONS_INFO {
public:
// Construction
	COptionsInfo();
	COptionsInfo(const BASE_OPTIONS_INFO& Info);
	COptionsInfo(const COptionsInfo& Info);
	COptionsInfo& operator=(const COptionsInfo& Info);

// Constants
	enum {
		DEF_THUMB_WIDTH = 100,
		MIN_THUMB_WIDTH = 2,
		MAX_THUMB_WIDTH = INT_MAX,
		DEF_THUMB_HEIGHT = 75,
		MIN_THUMB_HEIGHT = 2,
		MAX_THUMB_HEIGHT = INT_MAX,
		MIN_THREAD_COUNT = 1,
		MAX_THREAD_COUNT = 256,
		ARCHIVE_VERSION = 1,
	};

// Attributes
	void	GetBaseInfo(BASE_OPTIONS_INFO& Info) const;
	void	SetBaseInfo(const BASE_OPTIONS_INFO& Info);

// Public data
	// REMEMBER to add new members to ctor, Copy, and Serialize
	CString	m_DefSnapshot;	// path of default snapshot

// Operations
	void	Serialize(CArchive& ar);

protected:
// Helpers
	void	Copy(const COptionsInfo& Info);
};

inline COptionsInfo::COptionsInfo()
{
}

inline COptionsInfo::COptionsInfo(const BASE_OPTIONS_INFO& Info)
{
	SetBaseInfo(Info);
}

inline COptionsInfo::COptionsInfo(const COptionsInfo& Info)
{
	Copy(Info);
}

inline COptionsInfo& COptionsInfo::operator=(const COptionsInfo& Info)
{
	Copy(Info);
	return(*this);
}

inline void COptionsInfo::GetBaseInfo(BASE_OPTIONS_INFO& Info) const
{
	Info = *this;
}

inline void COptionsInfo::SetBaseInfo(const BASE_OPTIONS_INFO& Info)
{
	BASE_OPTIONS_INFO&	boi = *this;	// downcast to base struct
	boi = Info;
}

template<> inline void AFXAPI
SerializeElements<COptionsInfo>(CArchive& ar, COptionsInfo* pObj, W64INT nCount)
{
    for (int i = 0; i < nCount; i++, pObj++)
        pObj->Serialize(ar);
}

#endif
