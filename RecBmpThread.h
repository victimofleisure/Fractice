// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version

		worker thread to write bitmaps to AVI file 

*/

#ifndef CRECBMPTHREAD_INCLUDED
#define CRECBMPTHREAD_INCLUDED

#include "WorkerThread.h"

class CBmpToAvi;

class CRecBmpThread : public CWorkerThread {
public:
// Construction
	bool	Create(HWND hWnd, UINT StartFrame, LPCTSTR BitmapFolder, CBmpToAvi *RecAvi);

// Attributes
	UINT	GetNextFrame() const;
	LONG	GetWriteCount() const;

// Operations
	void	WriteFrames(LONG Count);

protected:
// Data members
	HWND	m_hWnd;			// handle of target window
	UINT	m_NextFrame;	// next frame to output
	LONG	m_WriteCount;	// number of frames to output
	LPCTSTR	m_BitmapFolder;	// path of folder for bitmaps
	CBmpToAvi	*m_RecAvi;	// pointer to AVI writer

// Helpers
	bool	MainLoop();
	void	Abort(WPARAM wParam, LPARAM lParam);
	static	UINT	ThreadFunc(LPVOID pParam);
};

inline UINT CRecBmpThread::GetNextFrame() const
{
	return(m_NextFrame);
}

inline LONG CRecBmpThread::GetWriteCount() const
{
	return(m_WriteCount);
}

inline void CRecBmpThread::WriteFrames(LONG Count)
{
	InterlockedExchangeAdd(&m_WriteCount, Count);
	StartWork();
}

#endif
