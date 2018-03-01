// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      15feb09	initial version
        01      09mar09	add crossfading
		02		10mar09	add autofade
		03		16mar09	add display mode info
		04		25mar09	animate mix slider during autofade
		05		01jun09	add Lissajous X/Y frequency accessors
		06		06jan10	W64: make OnTimer 64-bit compatible

        mix two views in DirectDraw Exclusive mode
 
*/

#if !defined(AFX_MIXER_H__8D2F217D_B274_4B13_8383_D25E8D5C2031__INCLUDED_)
#define AFX_MIXER_H__8D2F217D_B274_4B13_8383_D25E8D5C2031__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Mixer.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMixer window

#include "BackBufDD.h"
#include "DDEnumObj.h"
#include "WndTimer.h"
#include "Dib.h"
#include "MixerChan.h"
#include "DibEngine.h"	// for DIB_INFO

class CFracticeView;

class CMixer : public CWnd
{
// Construction
public:
	CMixer();

// Constants
	enum {	// mixer channels
		CH_A = MIXER_CHAN_A,
		CH_B = MIXER_CHAN_B,
		CHANS = MIXER_CHANS
	};
	enum {	// origin motions
		OM_NONE,
		OM_PING_PONG,
		OM_CURSOR_DRAG,
		OM_MIDI_DRAG,
		OM_RANDOM_JUMP,
		OM_LISSAJOUS,
		OM_TYPES
	};

// Types
	typedef struct tagDISPLAY_MODE_INFO {
		DWORD	Width;
		DWORD	Height;
		DWORD	BPP;
	} DISPLAY_MODE_INFO;

// Attributes
public:
	CFracticeView	*GetView(int ChanIdx) const;
	void	SetView(int ChanIdx, CFracticeView *View);
	int		GetCurChan() const;
	void	SetCurChan(int ChanIdx);
	CFracticeView	*GetCurView() const;
	CMixerChan&	GetChan(int ChanIdx);
	const CMixerChan&	GetChan(int ChanIdx) const;
	bool	IsExclusive() const;
	bool	IsDualMonitor() const;
	double	GetFaderPos() const;
	void	SetFaderPos(double Pos);
	bool	GetPlay() const;
	void	SetPlay(bool Enable);
	bool	GetLoop() const;
	void	SetLoop(bool Enable);
	double	GetAutofadeTime() const;
	void	SetAutofadeTime(double Seconds);
	double	GetPalTweenTime() const;
	void	SetPalTweenTime(double Seconds);
	bool	GetMirror() const;
	void	SetMirror(bool Enable);
	int		GetOriginMotion(int ChanIdx) const;
	void	SetOriginMotion(int ChanIdx, int OMType);
	double	GetTempo(int ChanIdx) const;
	void	SetTempo(int ChanIdx, double Tempo);
	double	GetDamping(int ChanIdx) const;
	void	SetDamping(int ChanIdx, double Damping);
	UINT	GetPongSpeed(int ChanIdx) const;
	void	SetPongSpeed(int ChanIdx, UINT Speed);
	double	GetLissajousSpeed(int ChanIdx) const;
	void	SetLissajousSpeed(int ChanIdx, double Speed);
	double	GetLissajousXFreq(int ChanIdx) const;
	void	SetLissajousXFreq(int ChanIdx, double XFreq);
	double	GetLissajousYFreq(int ChanIdx) const;
	void	SetLissajousYFreq(int ChanIdx, double YFreq);
	double	GetLissajousPhase(int ChanIdx) const;
	void	SetLissajousPhase(int ChanIdx, double Phase);
	void	GetTargetOrigin(int ChanIdx, DPoint& Origin) const;
	void	SetTargetOrigin(int ChanIdx, const DPoint& Origin);
	void	SetTargetOriginX(int ChanIdx, double Pos);
	void	SetTargetOriginY(int ChanIdx, double Pos);

// Operations
public:
	bool	Exclusive(bool Enable, const DISPLAY_MODE_INFO *ModeInfo = NULL);
	void	ApplySettings();
	static	bool	EnumDDObjects(CDDEnumObjArray& List);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMixer)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMixer();

// Generated message map functions
protected:
	//{{AFX_MSG(CMixer)
	afx_msg void OnClose();
	afx_msg void OnTimer(W64UINT nIDEvent);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Constants
	enum {
		FRAME_TIMER_ID = 1
	};

// Member data
	CBackBufDD	m_BackBuf;		// DirectDraw back buffer
	CFracticeView	*m_View[CHANS];	// view pointers; may be null
	CWndTimer	m_FrameTimer;	// timer to drive refresh
	CDib	m_Frame[CHANS];		// frame bitmaps, one per channel
	CDC		m_FrameDC;			// device context for frame bitmap
	HGDIOBJ	m_FrameDefBmp;		// frame DC's default bitmap
	CSize	m_FrameSize;		// current frame size in pixels
	CDibEngine::DIB_INFO	m_DibInfo;	// frame bitmap info
	bool	m_IsExclusive;		// true if we're in exclusive mode
	bool	m_IsDualMonitor;	// true if exclusive on a secondary monitor
	WINDOWPLACEMENT	m_MainPlace;	// main frame's previous placement
	CSize	m_DispSize;			// display size in pixels
	CRect	m_DispRect;			// display rectangle in screen coordinates
	int		m_FrameRate;		// frame rate, in frames per second
	CMixerChan	m_Chan[CHANS];	// array of mixer channels
	int		m_CurChan;			// index of current channel
	double	m_FaderPos;			// normalized fader position
	bool	m_FaderEnable;		// true if fader is enabled
	bool	m_Playing;			// true if auto-fading
	bool	m_Looping;			// true if auto-fade should loop
	int		m_AutofadeDir;		// autofade direction; 1 = forward, -1 = reverse
	double	m_AutofadeTime;		// autofade time, in seconds
	double	m_AutofadeDelta;	// autofade position delta per frame
	UINT	m_AutofadeAnimMod;	// autofade animation modulus
	UINT	m_AutofadeAnimCnt;	// autofade animation counter
	double	m_PalTweenTime;		// palette tween time, in seconds
	bool	m_Mirror;			// true if exclusive display is mirrored

// Helpers
	friend	class CMixerChan;
	bool	CreateBackBuf(CString& ErrMsg, const DISPLAY_MODE_INFO *ModeInfo);
	void	DestroyBackBuf();
	void	OnChannelChange();
	void	UpdateFrame(CDib& Frame, CFracticeView *View, int ChanIdx);
	void	FastSetFaderPos(double Pos);
	static	BOOL WINAPI DDEnumCallbackEx(GUID FAR *lpGUID, LPSTR lpDriverDescription, 
		LPSTR lpDriverName, LPVOID lpContext, HMONITOR hm);
};

inline CFracticeView *CMixer::GetView(int ChanIdx) const
{
	ASSERT(ChanIdx >= 0 && ChanIdx < CHANS);
	return(m_View[ChanIdx]);
}

inline int CMixer::GetCurChan() const
{
	return(m_CurChan);
}

inline CFracticeView *CMixer::GetCurView() const
{
	return(GetView(m_CurChan));
}

inline bool CMixer::IsExclusive() const
{
	return(m_IsExclusive);
}

inline bool CMixer::IsDualMonitor() const
{
	return(m_IsDualMonitor);
}

inline double CMixer::GetFaderPos() const
{
	return(m_FaderPos);
}

inline bool CMixer::GetPlay() const
{
	return(m_Playing);
}

inline bool CMixer::GetLoop() const
{
	return(m_Looping);
}

inline double CMixer::GetAutofadeTime() const
{
	return(m_AutofadeTime);
}

inline double CMixer::GetPalTweenTime() const
{
	return(m_PalTweenTime);
}

inline void CMixer::SetPalTweenTime(double Seconds)
{
	m_PalTweenTime = Seconds;
}

inline bool CMixer::GetMirror() const
{
	return(m_Mirror);
}

inline CMixerChan& CMixer::GetChan(int ChanIdx)
{
	ASSERT(ChanIdx >= 0 && ChanIdx < MIXER_CHANS);
	return(m_Chan[ChanIdx]);
}

inline const CMixerChan& CMixer::GetChan(int ChanIdx) const
{
	ASSERT(ChanIdx >= 0 && ChanIdx < MIXER_CHANS);
	return(m_Chan[ChanIdx]);
}

inline int CMixer::GetOriginMotion(int ChanIdx) const
{
	return(GetChan(ChanIdx).m_OrgMotion);
}

inline double CMixer::GetTempo(int ChanIdx) const
{
	return(GetChan(ChanIdx).m_Tempo);
}

inline double CMixer::GetDamping(int ChanIdx) const
{
	return(GetChan(ChanIdx).m_Damping);
}

inline void CMixer::SetDamping(int ChanIdx, double Damping)
{
	GetChan(ChanIdx).m_Damping = Damping;
}

inline UINT CMixer::GetPongSpeed(int ChanIdx) const
{
	return(GetChan(ChanIdx).m_PongSpeed);
}

inline void CMixer::SetPongSpeed(int ChanIdx, UINT Speed)
{
	GetChan(ChanIdx).m_PongSpeed = Speed;
}

inline double CMixer::GetLissajousXFreq(int ChanIdx) const
{
	return(GetChan(ChanIdx).m_LissXFreq);
}

inline void CMixer::SetLissajousXFreq(int ChanIdx, double XFreq)
{
	GetChan(ChanIdx).m_LissXFreq = XFreq;
}

inline double CMixer::GetLissajousYFreq(int ChanIdx) const
{
	return(GetChan(ChanIdx).m_LissYFreq);
}

inline void CMixer::SetLissajousYFreq(int ChanIdx, double YFreq)
{
	GetChan(ChanIdx).m_LissYFreq = YFreq;
}

inline double CMixer::GetLissajousSpeed(int ChanIdx) const
{
	return(GetChan(ChanIdx).m_LissSpeed);
}

inline void CMixer::SetLissajousSpeed(int ChanIdx, double Speed)
{
	GetChan(ChanIdx).m_LissSpeed = Speed;
}

inline double CMixer::GetLissajousPhase(int ChanIdx) const
{
	return(GetChan(ChanIdx).m_LissPhase);
}

inline void CMixer::SetLissajousPhase(int ChanIdx, double Phase)
{
	GetChan(ChanIdx).m_LissPhase = Phase;
}

inline void CMixer::GetTargetOrigin(int ChanIdx, DPoint& Origin) const
{
	Origin = GetChan(ChanIdx).m_TargetOrg;
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MIXER_H__8D2F217D_B274_4B13_8383_D25E8D5C2031__INCLUDED_)
