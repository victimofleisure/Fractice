// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		01feb09	in SetPalette, add color offset
		02		21feb09	move mirroring to tweening engine

        fractal rendering engine with bitmap support
 
*/

#include "stdafx.h"
#include "DibEngine.h"
#include "Palette.h"

CDibEngine::CDibEngine()
{
	ZeroMemory(&m_DibInfo, sizeof(m_DibInfo));
	m_EscFrameSize = CSize(0, 0);
	m_Antialias = 0;
}

CDibEngine::~CDibEngine()
{
	DestroyFrame();
}

void CDibEngine::DestroyFrame()
{
	DestroyEscFrame();	// waits for engine to be idle
	m_Dib.Destroy();
	ZeroMemory(&m_DibInfo, sizeof(m_DibInfo));
}

bool CDibEngine::CreateFrame(CSize Size, WORD BitCount, UINT Antialias)
{
	if (GetSSE2() && (Size.cx & 1))	// if SSE2 and odd width
		Size.cx &= ~1;	// force even width
	if (Size != m_DibInfo.Size || BitCount != m_DibInfo.BitCount) {
		DestroyFrame();
		if (!m_Dib.Create(Size.cx, Size.cy, BitCount))
			return(FALSE);
		BITMAP	bmp;
		if (!m_Dib.GetBitmap(&bmp))
			return(FALSE);
		m_DibInfo.Size = Size;
		m_DibInfo.BitCount = BitCount;
		m_DibInfo.Stride = CDib::GetStride(bmp);
		m_DibInfo.Length = Size.cy * m_DibInfo.Stride;
	}
	m_Antialias = Antialias;
	return(CreateEscFrame());
}

bool CDibEngine::AttachDib(HBITMAP Bitmap, PVOID Bits)
{
	BITMAP	bmp;
	if (!GetObject(Bitmap, sizeof(BITMAP), &bmp))
		return(FALSE);
	CSize	Size = CSize(bmp.bmWidth, bmp.bmHeight);
	ASSERT(Size == m_DibInfo.Size);	// bitmap size shouldn't change
	m_DibInfo.Size = Size;
	m_DibInfo.BitCount = bmp.bmBitsPixel;
	m_DibInfo.Stride = CDib::GetStride(bmp);
	m_DibInfo.Length = bmp.bmHeight * m_DibInfo.Stride;
	m_Dib.Attach(Bitmap, Bits);
	return(TRUE);
}

void CDibEngine::AttachEscFrame(CEscTimeArray& Frame)
{
	ESCTIME	*pEscTime;
	int	Size;
	Frame.Detach(pEscTime, Size);	// detach data from caller's array
	ASSERT(Size == m_EscFrame.GetSize());	// frame size shouldn't change
	m_EscFrame.Attach(pEscTime, Size);	// attach caller's data to our array
}

void CDibEngine::Swap(CDib& Dib, CEscTimeArray *Frame)
{
	m_Dib.Swap(Dib);
	if (Frame != NULL)
		m_EscFrame.Swap(*Frame);
}

void CDibEngine::DestroyEscFrame()
{
	CancelRender(TRUE);	// wait for engine to be idle
	m_EscFrame.RemoveAll();
	m_EscFrameSize = CSize(0, 0);
}

bool CDibEngine::CreateEscFrame()
{
	DestroyEscFrame();
	m_EscFrameSize = CSize(m_DibInfo.Size.cx * m_Antialias, 
		m_DibInfo.Size.cy * m_Antialias);
	int	elems = m_EscFrameSize.cx * m_EscFrameSize.cy;
	m_EscFrame.SetSize(elems);
	ZeroMemory(m_EscFrame.GetData(), elems * sizeof(ESCTIME));
	return(TRUE);
}

void CDibEngine::SetAntialias(UINT Antialias)
{
	if (Antialias == m_Antialias)
		return;	// nothing to do
	m_Antialias = Antialias;
	CreateEscFrame();
}

void CDibEngine::SetPalette(const DPalette& Palette, UINT Quality, UINT CycleLen, double Offset)
{
	CancelRender(TRUE);	// wait for engine to be idle
	SetQuality(Quality);	// set palette size
	Palette.GetRGBBk(m_Palette.GetData(), Quality, CycleLen, Offset);
}

void CDibEngine::OnRenderDone(CRenderer& Render)
{
	// virtual function override: same as base class behavior
	LONG	Busy = InterlockedDecrement(&m_BusyCount);
	if (!Busy && !Render.IsCanceled()) {
		MapColor();	// except we also map escape times to DIB
		PostMessage(m_hWnd, UWM_RENDERDONE, m_CurUID, 0);
	}
}

void CDibEngine::MapColor()
{
	ASSERT(m_DibInfo.BitCount == 24);	// 24 bits per pixel ONLY
	const ESCTIME	*pEscFrame = m_EscFrame.GetData();
	PBYTE	pDib = (PBYTE)m_Dib.GetBits();
	if (m_Antialias > 1) {
		int	Factor = m_Antialias;
		int	FactorSquared = Factor * Factor;
		int	Stride = m_EscFrameSize.cx;
		int	Delta1 = Stride - Factor;
		int	Delta2 = Stride * Factor - Factor;
		int	Delta3 = Stride * (Factor - 1);
		for (int y = 0; y < m_DibInfo.Size.cy; y++) {
			PBYTE	p = pDib; 
			for (int x = 0; x < m_DibInfo.Size.cx; x++) {
				int	r = 0;
				int	g = 0;
				int	b = 0;
				for (int yi = 0; yi < Factor; yi++) {
					for (int xi = 0; xi < Factor; xi++) {
						COLORREF	color = m_Palette[*pEscFrame++];
						b += GetBValue(color);
						g += GetGValue(color);
						r += GetRValue(color);
					}
					pEscFrame += Delta1;
				}
				pEscFrame -= Delta2;
				*p++ = BYTE(b / FactorSquared);
				*p++ = BYTE(g / FactorSquared);
				*p++ = BYTE(r / FactorSquared);
			}
			pDib += m_DibInfo.Stride;
			pEscFrame += Delta3;
		}
	} else {	// no antialiasing, so just map escape times to colors
		for (int y = 0; y < m_DibInfo.Size.cy; y++) {
			PBYTE	p = pDib; 
			for (int x = 0; x < m_DibInfo.Size.cx; x++) {
				COLORREF	color = m_Palette[*pEscFrame++];
				*p++ = GetBValue(color);
				*p++ = GetGValue(color);
				*p++ = GetRValue(color);
			}
			pDib += m_DibInfo.Stride;
		}
	}
}
