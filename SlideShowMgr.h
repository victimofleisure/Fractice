// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14mar09	initial version
		01		06jan10	W64: in GetItemCount, cast item array size to 32-bit

        slide show manager
 
*/

#ifndef CSLIDESHOWMGR_INCLUDED
#define CSLIDESHOWMGR_INCLUDED

#include "WndTimer.h"
#include "RandList.h"

class CHistoryView;

class CSlideShowMgr : public WObject
{
public:
// Construction
	CSlideShowMgr();
	~CSlideShowMgr();

// Constants
	enum {	// transport states
		TS_STOP,
		TS_PAUSE,
		TS_PLAY,
	};
	enum {	// order types
		OT_SEQUENTIAL,
		OT_RANDOM,
		ORDER_TYPES
	};

// Attributes
	void	SetView(CHistoryView *Hist, int TimerID);
	int		GetTransport() const;
	bool	SetTransport(int State);
	bool	IsStopped() const;
	bool	IsPaused() const;
	bool	IsPlaying() const;
	bool	IsRunning() const;
	int		GetItemCount() const;
	int		GetCurPos() const;
	double	GetInterval() const;
	void	SetInterval(double Interval);
	int		GetOrder() const;
	void	SetOrder(int Order);
	bool	GetLoop() const;
	void	SetLoop(bool Enable);

// Operations
	bool	Stop();
	bool	Pause();
	bool	Play();
	bool	ShowNextSlide();

protected:
// Constants
	enum {
		DEFAULT_INTERVAL = 5,	// default interval in seconds
	};

// Member data
	CHistoryView	*m_Hist;	// pointer to parent view
	CWndTimer	m_Timer;		// timer for triggering slides
	int		m_TimerID;			// timer identifier
	double	m_Interval;			// interval between slides, in seconds
	int		m_Order;			// slide show order; see enum above
	bool	m_Loop;				// true if slide show should loop
	bool	m_Running;			// true if slide show is in progress
	int		m_Transport;		// transport state; see enum above
	CDWordArray	m_SelIdx;		// indices of selected items
	int		m_CurPos;			// current position in slide list
	CRandList	m_RandList;		// randomizes list without duplicates

// Helpers
	void	OnSelectionChange();
	void	OnIntervalChange();
};

inline void CSlideShowMgr::SetView(CHistoryView *Hist, int TimerID)
{
	m_Hist = Hist;
	m_TimerID = TimerID;
}

inline int CSlideShowMgr::GetTransport() const
{
	return(m_Transport);
}

inline bool CSlideShowMgr::IsStopped() const
{
	return(m_Transport == TS_STOP);
}

inline bool CSlideShowMgr::IsPaused() const
{
	return(m_Transport == TS_PAUSE);
}

inline bool CSlideShowMgr::IsPlaying() const
{
	return(m_Transport == TS_PLAY);
}

inline bool CSlideShowMgr::Stop()
{
	return(SetTransport(TS_STOP));
}

inline bool CSlideShowMgr::Pause()
{
	return(SetTransport(TS_PAUSE));
}

inline bool CSlideShowMgr::Play()
{
	return(SetTransport(TS_PLAY));
}

inline bool CSlideShowMgr::IsRunning() const
{
	return(m_Running);
}

inline int CSlideShowMgr::GetItemCount() const
{
	return(INT64TO32(m_SelIdx.GetSize()));
}

inline int CSlideShowMgr::GetCurPos() const
{
	return(m_CurPos);
}

inline double CSlideShowMgr::GetInterval() const
{
	return(m_Interval);
}

inline int CSlideShowMgr::GetOrder() const
{
	return(m_Order);
}

inline void CSlideShowMgr::SetOrder(int Order)
{
	m_Order = Order;
}

inline bool CSlideShowMgr::GetLoop() const
{
	return(m_Loop);
}

inline void CSlideShowMgr::SetLoop(bool Enable)
{
	m_Loop = Enable;
}

#endif
