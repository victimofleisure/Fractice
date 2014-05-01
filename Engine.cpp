// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
        01      06jan09	reformat squaring macro usage
        02      10jan09	set bignum precision explicitly
		03		23jul09	add Mandelbar
		04		13dec09	for bignum renders, move cancel check into x loop
		05		07jan10	move render code to separate files
		06		27jan10	SSE was accidentally disabled, fix conditionals

        multi-threaded fractal rendering engine
 
*/

#include "stdafx.h"
#include "EngineImpl.h"
#include "Benchmark.h"

#ifndef	_CONSOLE
#include "Fractice.h"	// for CancelRender wait cursor
#endif

CEngine::CEngine()
{
	m_RenderCount = 0;
	m_BusyCount = 0;
	m_hWnd = NULL;
	m_CurUID = 0;
	ZeroMemory(&m_Job, sizeof(m_Job));
	m_HaveSSE2 = QuerySSE2();
	m_UseSSE2 = m_HaveSSE2;
}

CEngine::~CEngine()
{
	KillThreads();
}

bool CEngine::LaunchThreads(HWND hWnd, int ThreadCount)
{
	enum {
		BOSS_PRIORITY = THREAD_PRIORITY_BELOW_NORMAL,
		RENDER_PRIORITY = THREAD_PRIORITY_LOWEST
	};
	ASSERT(ThreadCount > 0);
	KillThreads();
	m_hWnd = hWnd;
	m_RenderCount = ThreadCount;
	m_Render.RemoveAll();
	m_Render.SetSize(m_RenderCount);
	for (int i = 0; i < m_RenderCount; i++) {
		CRenderer&	Render = m_Render[i];
		Render.m_Engine = this;
		Render.m_ThrIdx = i;
		if (Render.Launch(RenderFunc, &Render, RENDER_PRIORITY) == NULL)
			return(FALSE);
	}
	return(m_Boss.Launch(BossFunc, this, BOSS_PRIORITY) != NULL);
}

void CEngine::KillThreads()
{
	m_Boss.Kill();
	for (int i = 0; i < m_RenderCount; i++)
		m_Render[i].Kill();
}

void CEngine::SetMathPrec(UINT Bits)
{
	// set precision of BigNum-derived member vars here
	m_Bounds.SetPrec(Bits);
}

void CEngine::RenderFrame(const RENDER_INFO& Info, const BigRect& Bounds)
{
	m_Job = Info;
	m_Bounds = Bounds;
	m_Boss.StartWork();
}

void CEngine::CancelRender(bool WaitForIdle)
{
#ifndef	_CONSOLE	// if GUI app
	if (WaitForIdle)
		theApp.BeginWaitCursor();	// show wait cursor
#endif
	// if WaitForIdle is true, and a render is in progress, the caller is
	// blocked until each render thread finishes its current row; the wait
	// can be significant if we're using bignums or rendering a huge frame
	m_Boss.Cancel(WaitForIdle);
	for (int i = 0; i < m_RenderCount; i++)
		m_Render[i].Cancel(WaitForIdle);
#ifndef	_CONSOLE	// if GUI app
	if (WaitForIdle)
		theApp.EndWaitCursor();		// restore previous cursor
#endif
}

bool CEngine::QuerySSE2()
{
#if _MFC_VER > 0x0600
	int	CPUInfo[4];
	__cpuid(CPUInfo, 1);
	return((CPUInfo[3] & CPUID_FEATURE_SSE2) != 0);
#else	// MFC 6
	return(FALSE);	// can't compile SSE2
#endif
}

void CEngine::RenderLoop(CRenderer& Render)
{
	while (Render.WaitForWork()) {
		ASSERT(Render.m_y1 >= 0 && Render.m_y1 <= Render.m_y2 
			&& Render.m_y2 <= UINT(m_Job.FrameSize.cy));
		switch (m_Job.FractalType) {
		case FT_MANDELBROT:
			switch (m_Job.Exponent) {
			case 2:
				if (m_Job.DeepZoom)
					Render.MandelbrotGMP();	// use GMP multi-precision math
				else {
#if _MFC_VER > 0x0600
					if (m_UseSSE2)
						Render.MandelbrotSSE2();	// use SSE2 floating-point
					else
						Render.MandelbrotFPU();	// use FPU floating-point
#else	// MFC 6
					Render.MandelbrotFPU();	// use FPU floating-point
#endif
				}
				break;
			case 3:		Render.MandelbrotE3FPU();	break;
			case 4:		Render.MandelbrotE4FPU();	break;
			case 5:		Render.MandelbrotE5FPU();	break;
			case 6:		Render.MandelbrotE6FPU();	break;
			case 7:		Render.MandelbrotE7FPU();	break;
			case 8:		Render.MandelbrotE8FPU();	break;
			case 9:		Render.MandelbrotE9FPU();	break;
			case 10:	Render.MandelbrotE10FPU();	break;
			case 11:	Render.MandelbrotE11FPU();	break;
			case 12:	Render.MandelbrotE12FPU();	break;
			case 13:	Render.MandelbrotE13FPU();	break;
			default:
				ASSERT(0);	// invalid exponent
			}
			break;
		case FT_MANDELBAR:
			switch (m_Job.Exponent) {
			case 2:
				if (m_Job.DeepZoom)
					Render.MandelbarGMP();	// use GMP multi-precision math
				else {
#if _MFC_VER > 0x0600
					if (m_UseSSE2)
						Render.MandelbarSSE2();	// use SSE2 floating-point
					else
						Render.MandelbarFPU();	// use FPU floating-point
#else	// MFC 6
					Render.MandelbarFPU();	// use FPU floating-point
#endif
				}
				break;
			case 3:		Render.MandelbarE3FPU();	break;
			case 4:		Render.MandelbarE4FPU();	break;
			case 5:		Render.MandelbarE5FPU();	break;
			case 6:		Render.MandelbarE6FPU();	break;
			case 7:		Render.MandelbarE7FPU();	break;
			case 8:		Render.MandelbarE8FPU();	break;
			case 9:		Render.MandelbarE9FPU();	break;
			case 10:	Render.MandelbarE10FPU();	break;
			case 11:	Render.MandelbarE11FPU();	break;
			case 12:	Render.MandelbarE12FPU();	break;
			case 13:	Render.MandelbarE13FPU();	break;
			default:
				ASSERT(0);	// invalid exponent
			}
			break;
		case FT_NEWTON:
			Render.NewtonFPU();
			break;
		case FT_NO_RENDER:
			break;
		default:
			ASSERT(0);	// invalid fractal type
		}
		OnRenderDone(Render);	// virtual function
	}
}

void CEngine::OnRenderDone(CRenderer& Render)
{
	LONG	Busy = InterlockedDecrement(&m_BusyCount);
	if (!Busy && !Render.IsCanceled())
		PostMessage(m_hWnd, UWM_RENDERDONE, m_CurUID, 0);
}

void CEngine::BossLoop()
{
	while (m_Boss.WaitForWork()) {
		int	i;
		for (i = 0; i < m_RenderCount; i++)
			m_Render[i].Cancel(TRUE);
		m_CurUID = m_Job.FrameUID;
		m_BusyCount = m_RenderCount;
		for (i = 0; i < m_RenderCount; i++) {
			CRenderer&	rend = m_Render[i];
			rend.m_y1 = i;
			rend.m_y2 = m_Job.FrameSize.cy;
			rend.m_DeltaY = m_RenderCount;
			rend.m_FrameBuf = m_Job.FrameBuf + i * m_Job.FrameSize.cx;
			rend.StartWork();
		}
	}
}

UINT CEngine::RenderFunc(LPVOID pParam)
{
	CRenderer	*pRender = (CRenderer *)pParam;
	CEngine	*pEngine = pRender->m_Engine;
	pEngine->RenderLoop(*pRender);
	return(0);
}

UINT CEngine::BossFunc(LPVOID pParam)
{
	CEngine	*pEngine = (CEngine *)pParam;
	pEngine->BossLoop();
	return(0);
}
