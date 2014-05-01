// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		23jul09	add Mandelbar
		02		16dec09	add GetBusyCount, IsIdle and GetJob

        multi-threaded fractal rendering engine
 
*/

#ifndef CENGINE_INCLUDED
#define CENGINE_INCLUDED

#include "EngineTypes.h"
#include "ArrayEx.h"
#include "WorkerThread.h"
#include "BigNum.h"
#include "Dump.h"

class CEngine : public WObject {
public:
// Construction
	CEngine();
	virtual	~CEngine();

// Attributes
	int		GetThreadCount() const;
	LONG	GetBusyCount() const;
	bool	IsIdle() const;
	void	SetMathPrec(UINT Bits);
	UINT	GetCurrentRow() const;
	UINT	GetCurrentRow(int ThreadIdx) const;
	UINT	GetY1(int ThreadIdx) const;
	UINT	GetY2(int ThreadIdx) const;
	const ESCTIME	*GetFrameBuf(int ThreadIdx) const;
	bool	HaveSSE2() const;
	bool	GetSSE2() const;
	void	SetSSE2(bool Enable);
	UINT	GetCurUID() const;
	HWND	GetNotifyWnd() const;
	void	SetNotifyWnd(HWND hWnd);
	const RENDER_INFO&	GetJob() const;

// Operations
	bool	LaunchThreads(HWND hWnd, int ThreadCount);
	void	KillThreads();
	void	RenderFrame(const RENDER_INFO& Info, const BigRect& Bounds);
	void	CancelRender(bool WaitForIdle = FALSE);
	static	bool	QuerySSE2();

protected:
// Types
	class CRenderer : public CWorkerThread {
	public:
// Data members
		CEngine	*m_Engine;		// pointer to engine
		volatile UINT	m_CurRow;	// current row
		UINT	m_ThrIdx;		// thread index
		UINT	m_y1;			// first row to render
		UINT	m_y2;			// last row to render, plus one
		UINT	m_DeltaY;		// number of rows to increment by
		ESCTIME	*m_FrameBuf;	// pointer to frame buffer

// Operations
		void	MandelbrotFPU();
		void	MandelbrotGMP();
		void	MandelbrotSSE2();
		void	MandelbrotE3FPU();
		void	MandelbrotE4FPU();
		void	MandelbrotE5FPU();
		void	MandelbrotE6FPU();
		void	MandelbrotE7FPU();
		void	MandelbrotE8FPU();
		void	MandelbrotE9FPU();
		void	MandelbrotE10FPU();
		void	MandelbrotE11FPU();
		void	MandelbrotE12FPU();
		void	MandelbrotE13FPU();
		void	MandelbarFPU();
		void	MandelbarGMP();
		void	MandelbarSSE2();
		void	MandelbarE3FPU();
		void	MandelbarE4FPU();
		void	MandelbarE5FPU();
		void	MandelbarE6FPU();
		void	MandelbarE7FPU();
		void	MandelbarE8FPU();
		void	MandelbarE9FPU();
		void	MandelbarE10FPU();
		void	MandelbarE11FPU();
		void	MandelbarE12FPU();
		void	MandelbarE13FPU();
		void	NewtonFPU();
	};
	typedef CArrayEx<CRenderer, CRenderer&> CRendererArray;
	
// Member data
	CWorkerThread	m_Boss;	// supervisor thread
	CRendererArray	m_Render;	// array of renderer threads
	int		m_RenderCount;	// number of renderers running
	LONG	m_BusyCount;	// number of renderers working
	HWND	m_hWnd;			// parent window
	RENDER_INFO	m_Job;		// current rendering information
	BigRect	m_Bounds;		// current frame bounding rectangle 
	volatile UINT	m_CurUID;	// current frame's unique indentifier
	bool	m_HaveSSE2;		// true if CPU supports SSE2
	bool	m_UseSSE2;		// true if we're using SSE2

// Overridables
	virtual	void	OnRenderDone(CRenderer& Render);

// Helpers
	void	RenderLoop(CRenderer& Render);
	void	BossLoop();
	static	UINT	RenderFunc(LPVOID pParam);
	static	UINT	BossFunc(LPVOID pParam);
	friend class CRenderer;
};

inline int CEngine::GetThreadCount() const
{
	return(m_RenderCount);
}

inline LONG CEngine::GetBusyCount() const
{
	return(m_BusyCount);
}

inline bool CEngine::IsIdle() const
{
	return(!m_BusyCount);
}

inline UINT CEngine::GetCurrentRow() const
{
	return(m_Render[0].m_CurRow);
}

inline UINT CEngine::GetCurrentRow(int ThreadIdx) const
{
	return(m_Render[ThreadIdx].m_CurRow);
}

inline UINT CEngine::GetY1(int ThreadIdx) const
{
	return(m_Render[ThreadIdx].m_y1);
}

inline UINT CEngine::GetY2(int ThreadIdx) const
{
	return(m_Render[ThreadIdx].m_y2);
}

inline const ESCTIME *CEngine::GetFrameBuf(int ThreadIdx) const
{
	return(m_Render[ThreadIdx].m_FrameBuf);
}

inline bool CEngine::HaveSSE2() const
{
	return(m_HaveSSE2);
}

inline bool CEngine::GetSSE2() const
{
	return(m_UseSSE2);
}

inline void CEngine::SetSSE2(bool Enable)
{
	m_UseSSE2 = Enable;
}

inline UINT CEngine::GetCurUID() const
{
	return(m_CurUID);
}

inline HWND CEngine::GetNotifyWnd() const
{
	return(m_hWnd);
}

inline void CEngine::SetNotifyWnd(HWND hWnd)
{
	m_hWnd = hWnd;
}

inline const RENDER_INFO& CEngine::GetJob() const
{
	return(m_Job);
}

#endif
