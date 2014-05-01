// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*	
		chris korda

		revision history:
		rev		date	comments
        00      28feb09	initial version
        01      09mar09	add channel array
		02		30mar09	add Read/Write and doc paths
		03		06jan10	W64: in SerializeElements, nCount is 64-bit

		container for mixer information

*/

#ifndef CMIXERINFO_INCLUDED
#define CMIXERINFO_INCLUDED

enum {	// mixer channels
	MIXER_CHAN_A,
	MIXER_CHAN_B,
	MIXER_CHANS,
};

// APPEND ONLY, else archive will no longer be backwardly compatible
typedef struct tagMIXER_CHAN_INFO {
	int		m_OrgMotion;	// type of origin motion; see enum in BackBufWnd.h
	int		m_PongSpeed;	// pong speed
	double	m_Damping;		// damping factor
	double	m_Tempo;		// tempo, in beats per minute
	double	m_LissSpeed;	// Lissajous speed
	double	m_LissXFreq;	// Lissajous a
	double	m_LissYFreq;	// Lissajous b
	double	m_LissPhase;	// Lissajous phase delta, in radians
} MIXER_CHAN_INFO;

// APPEND ONLY, else archive will no longer be backwardly compatible
typedef struct tagMIXER_MISC_INFO {
	double	m_FaderPos;		// fader position; normalized
	BOOL	m_AutofadePlay;	// true if auto-fading
	BOOL	m_AutofadeLoop;	// true if auto-fade should loop
	double	m_AutofadeTime;	// autofade time, in seconds
	double	m_PalTweenTime;	// palette tween time, in seconds
	BOOL	m_Mirror;		// true if exclusive display is mirrored
	int		m_CurChan;		// index of current channel
} MIXER_MISC_INFO;

typedef struct tagMIXER_INFO : public MIXER_MISC_INFO {
	MIXER_CHAN_INFO	m_Chan[MIXER_CHANS];	// array of channel info
} MIXER_INFO;

class CMixerInfo : public MIXER_INFO {
public:
// Construction
	CMixerInfo();
	CMixerInfo(const MIXER_INFO& Info);
	CMixerInfo(const MIXER_MISC_INFO& Misc, const MIXER_CHAN_INFO& Chan);
	CMixerInfo(const CMixerInfo& Info);
	CMixerInfo& operator=(const CMixerInfo& Info);

// Constants
	enum {
		ARCHIVE_SIG = 0x4d637246,	// archive signature (FrcM)
		ARCHIVE_VERSION = 1,		// archive version number
	};

// Attributes
	void	GetBaseInfo(MIXER_INFO& Info) const;
	void	SetBaseInfo(const MIXER_INFO& Info);
	void	GetMiscInfo(MIXER_MISC_INFO& Info) const;
	void	SetMiscInfo(const MIXER_MISC_INFO& Info);
	MIXER_CHAN_INFO&	GetChan(int ChanIdx);
	const MIXER_CHAN_INFO&	GetChan(int ChanIdx) const;
	void	SetChan(int ChanIdx, const MIXER_CHAN_INFO& Info);
	MIXER_CHAN_INFO&	GetCurChan();
	const MIXER_CHAN_INFO&	GetCurChan() const;
	void	SetCurChan(const MIXER_CHAN_INFO& Info);

// Operations
	void	Serialize(CArchive& ar);
	bool	DoIO(LPCTSTR Path, DWORD FileMode, DWORD ArchiveMode);
	bool	Write(LPCTSTR Path);
	bool	Read(LPCTSTR Path);

// Public data
	// REMEMBER to add new members to ctor, Copy, and Serialize
	CStringArray	m_DocPath;	// document paths for each channel

protected:
// Helpers
	void	Copy(const CMixerInfo& Info);
};

inline CMixerInfo::CMixerInfo()
{
}

inline CMixerInfo::CMixerInfo(const MIXER_INFO& Info)
{
	SetBaseInfo(Info);
}

inline CMixerInfo::CMixerInfo(const MIXER_MISC_INFO& Misc, const MIXER_CHAN_INFO& Chan)
{
	SetMiscInfo(Misc);
	m_Chan[MIXER_CHAN_A] = Chan;
	m_Chan[MIXER_CHAN_B] = Chan;
}

inline CMixerInfo::CMixerInfo(const CMixerInfo& Info)
{
	Copy(Info);
}

inline CMixerInfo& CMixerInfo::operator=(const CMixerInfo& Info)
{
	Copy(Info);
	return(*this);
}

inline MIXER_CHAN_INFO& CMixerInfo::GetChan(int ChanIdx)
{
	ASSERT(ChanIdx >= 0 && ChanIdx < MIXER_CHANS);
	return(m_Chan[ChanIdx]);
}

inline const MIXER_CHAN_INFO& CMixerInfo::GetChan(int ChanIdx) const
{
	ASSERT(ChanIdx >= 0 && ChanIdx < MIXER_CHANS);
	return(m_Chan[ChanIdx]);
}

inline void CMixerInfo::SetChan(int ChanIdx, const MIXER_CHAN_INFO& Info)
{
	ASSERT(ChanIdx >= 0 && ChanIdx < MIXER_CHANS);
	m_Chan[ChanIdx] = Info;
}

inline MIXER_CHAN_INFO& CMixerInfo::GetCurChan()
{
	return(GetChan(m_CurChan));
}

inline const MIXER_CHAN_INFO& CMixerInfo::GetCurChan() const
{
	return(GetChan(m_CurChan));
}

inline void CMixerInfo::SetCurChan(const MIXER_CHAN_INFO& Info)
{
	SetChan(m_CurChan, Info);
}

inline void CMixerInfo::GetBaseInfo(MIXER_INFO& Info) const
{
	Info = *this;
}

inline void CMixerInfo::SetBaseInfo(const MIXER_INFO& Info)
{
	MIXER_INFO&	mi = *this;	// downcast to base struct
	mi = Info;
}

inline void CMixerInfo::GetMiscInfo(MIXER_MISC_INFO& Info) const
{
	Info = *this;
}

inline void CMixerInfo::SetMiscInfo(const MIXER_MISC_INFO& Info)
{
	MIXER_MISC_INFO&	mmi = *this;	// downcast to base struct
	mmi = Info;
}

template<> inline void AFXAPI
SerializeElements<CMixerInfo>(CArchive& ar, CMixerInfo* pObj, W64INT nCount)
{
    for (int i = 0; i < nCount; i++, pObj++)
        pObj->Serialize(ar);
}

#endif
