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
		03		06jan10	W64: in GetQuality, cast palette array size to 32-bit

        fractal rendering engine with bitmap support
 
*/

#ifndef CDIBENGINE_INCLUDED
#define CDIBENGINE_INCLUDED

#include "Engine.h"
#include "Dib.h"

class DPalette;

class CDibEngine : public CEngine {
public:
// Construction
	CDibEngine();
	~CDibEngine();

// Types
	typedef struct tagDIB_INFO {	// bitmap information
		SIZE	Size;		// bitmap dimensions, in pixels
		WORD	BitCount;	// bitmap color depth, in bits per pixel
		UINT	Stride;		// storage required for a scan line, in bytes
		UINT	Length;		// storage required for entire bitmap, in bytes
	} DIB_INFO;
	typedef	CArrayEx<ESCTIME, ESCTIME> CEscTimeArray;

// Attributes
	const	DIB_INFO&	GetDibInfo() const;
	CDib&	GetDib();
	const CDib&	GetDib() const;
	const	PVOID	GetDibBits() const;
	void	SetDib(CDib& Dib);
	ESCTIME	*GetEscFrame();
	void	GetEscFrame(CEscTimeArray& Frame) const;
	void	SetEscFrame(const CEscTimeArray& Frame);
	CSize	GetEscFrameSize() const;
	UINT	GetAntialias() const;
	void	SetAntialias(UINT Antialias);
	UINT	GetQuality() const;
	void	SetQuality(UINT Quality);
	void	SetPalette(const DPalette& Palette, UINT Quality, UINT CycleLen, double Offset);

// Operations
	bool	CreateFrame(CSize Size, WORD BitCount, UINT Antialias);
	void	DestroyFrame();
	bool	AttachDib(HBITMAP Bitmap, PVOID Bits);
	void	AttachEscFrame(CEscTimeArray& Frame);
	void	Swap(CDib& Dib, CEscTimeArray *Frame);
	void	MapColor();

protected:
// Member data
	CDib	m_Dib;			// device-independent bitmap
	DIB_INFO	m_DibInfo;	// bitmap description
	CEscTimeArray	m_EscFrame;	// escape time frame buffer
	CSize	m_EscFrameSize;	// escape time frame dimensions, in pixels
	UINT	m_Antialias;	// oversampling factor; 1 = no antialiasing
	CDWordArray	m_Palette;	// palette RGB values

// Overrides
	void	OnRenderDone(CRenderer& Render);

// Helpers
	bool	CreateEscFrame();
	void	DestroyEscFrame();
};

inline const CDibEngine::DIB_INFO& CDibEngine::GetDibInfo() const
{
	return(m_DibInfo);
}

inline CDib& CDibEngine::GetDib()
{
	return(m_Dib);
}

inline const CDib& CDibEngine::GetDib() const
{
	return(m_Dib);
}

inline const PVOID CDibEngine::GetDibBits() const
{
	return(m_Dib.GetBits());
}

inline void CDibEngine::SetDib(CDib& Dib)
{
#ifdef _DEBUG
	BITMAP	bmp;
	Dib.GetBitmap(&bmp);
	UINT	len = bmp.bmHeight * bmp.bmWidthBytes;
	ASSERT(len == m_DibInfo.Length);	// frame size can't change
#endif
	memcpy(m_Dib.GetBits(), Dib.GetBits(), m_DibInfo.Length);
}

inline ESCTIME *CDibEngine::GetEscFrame()
{
	return(m_EscFrame.GetData());
}

inline void CDibEngine::GetEscFrame(CEscTimeArray& Frame) const
{
	Frame.Copy(m_EscFrame);
}

inline void CDibEngine::SetEscFrame(const CEscTimeArray& Frame)
{
	ASSERT(Frame.GetSize() == m_EscFrameSize.cx * m_EscFrameSize.cy);	// frame size can't change
	m_EscFrame.Copy(Frame);
}

inline CSize CDibEngine::GetEscFrameSize() const
{
	return(m_EscFrameSize);
}

inline UINT CDibEngine::GetAntialias() const
{
	return(m_Antialias);
}

inline UINT CDibEngine::GetQuality() const
{
	return(INT64TO32(m_Palette.GetSize()) - 1);	// quality excludes background color
}

inline void CDibEngine::SetQuality(UINT Quality)
{
	m_Palette.SetSize(Quality + 1);	// one extra for background color
}

#endif
