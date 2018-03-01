// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda

		revision history:
		rev		date	comments
        00      21feb09	initial version
		01		01mar09	cycle old palette during tween
		02		08mar09	add MixDibs
		03		13mar09	fix needless reallocation in SetPalette
		04		06jan10	W64: disable MMX for 64-bit platforms
		05		06jan10	W64: in TweenPalette, cast palette array size to 32-bit
		06		26jan10	in MapColorEx, use SSE multiply instead of divides
		07		27jan10	add MapColorEx64 for 64-bit MMX
		08		01feb10	add MixDibs64 and TweenPalette64

        fractal rendering engine with palette tweening

*/

#include "stdafx.h"
#include "Fractice.h"
#include "TweenEngine.h"
#include "MainFrm.h"

#ifdef	_WIN64
#define TWEEN_MMX		TRUE
#define ANTIALIAS_MMX	TRUE
#define MIXDIBS_MMX		TRUE
#else
#define TWEEN_MMX		TRUE
#define ANTIALIAS_MMX	TRUE
#define MIXDIBS_MMX		TRUE
#endif

#if defined(_WIN64)
extern "C" {
	void	MapColorEx64(int Width, int Height, const DWORD *pPalette, const ESCTIME *pEscFrame, 
		PBYTE pDibRow, int Factor, int Stride, int DibStride, int MirrorDelta);
	void	MixDibs64(const BYTE *pSrc, BYTE *pDst, int DWords, WORD MixPos);
	void	TweenPalette64(DWORD *pDest, DWORD *pNewPal, DWORD *pOldPal, int Quality, 
		UINT m_OldCycleLen, UINT OldPalIdx, WORD MixPos);
};
#endif

CTweenEngine::CTweenEngine()
{
	m_Quality = 0;
	m_Tweening = FALSE;
	m_TweenPos = 0;
	m_TweenDelta = 0;
	m_CycleLen = 0;
	m_OldCycleLen = 0;
	m_ColorOffset = 0;
	m_InitColorOfs = 0;
}

void CTweenEngine::SetTweening(bool Enable)
{
	m_Tweening = Enable;
	if (!Enable)
		m_TweenPal.Copy(m_Palette);	// copy base class palette to tween output
}

void CTweenEngine::SetPalette(const DPalette& Palette, UINT Quality, UINT CycleLen, double Offset)
{
//
// In exclusive mode, color mapping and rendering overlap; the GUI thread is
// reading the escape times while the engine is simultaneously updating them.
// In this situation we must avoid shrinking the RGB palette, otherwise the
// previous render's escape times will very likely exceed the RGB palette's
// index range, causing an access violation in MapColorEx.
//
// The subtle issue is that we're allowing our frame to contain escape times
// from multiple renders.  The renders may have different ranges of escape
// times, due to having different Quality values.  Since we use the escape
// times as indices into our color table, our color table must contain enough
// entries to handle any of the escape times that could possibly exist in our
// frame.  We guarantee this by growing the table, but never shrinking it.
//
	CMainFrame	*pMain = theApp.GetMain();
	if (pMain->IsExclusive() && m_Palette.GetSize()) {
		m_OldPal.Copy(m_TweenPal);	// start from current tween palette
		// 3/13/09: compare Quality to GetQuality(), not m_Quality; we're
		// comparing the new palette's size to the base class palette size
		if (Quality > GetQuality())	{	// if base class palette must grow
			// before reallocating palette, make sure threads aren't accessing it
			CancelRender(TRUE);	// wait for engine to be idle
			SetQuality(Quality);	// expand base class palette
			Palette.GetRGBBk(m_Palette.GetData(), Quality, CycleLen, Offset);
			int	PrevSize = m_Quality + 1;	// one extra for background color
			int	NewSize = Quality + 1;
			m_TweenPal.SetSize(NewSize);	// must expand tween palette too
			// The excess portion of the new palette can't be tweened. There's
			// nothing to tween it with, so just copy it to the tween palette.
			memcpy(m_TweenPal.GetData() + PrevSize, m_Palette.GetData() + PrevSize,
				(NewSize - PrevSize) * sizeof(ESCTIME));
		} else	// new palette is same size or smaller; no reallocation needed
			Palette.GetRGBBk(m_Palette.GetData(), Quality, CycleLen, Offset);
		m_NewPal.Copy(m_Palette);	// set target palette for tween
		double	TweenTime = pMain->GetMixer().GetPalTweenTime();
		if (TweenTime > 0) {	// if valid tween time
			m_TweenDelta = 1.0 / TweenTime / pMain->GetOptionsDlg().GetFrameRate();
			m_TweenPos = 0;
		} else {	// can't tween
			m_TweenDelta = 0;
			m_TweenPos = 1;	// jump directly to new palette
		}
		m_Tweening = TRUE;	// start tweening on next frame
	} else {	// reset palette
		// If quality changed, we're resizing and potentially reallocating the
		// base class palette, so we must wait for the engine to become idle;
		// otherwise an access violation could occur, because MapColor could be
		// running in a different thread, and it reads the base class palette.
		if (Quality != m_Quality)	// if quality changed
			CancelRender(TRUE);	// wait for engine to be idle
		SetQuality(Quality);	// set palette size
		Palette.GetRGBBk(m_Palette.GetData(), Quality, CycleLen, Offset);
		m_Tweening = FALSE;	// stop tweening
		m_TweenPal.Copy(m_Palette);	// copy base class palette to tween output
	}
	m_Quality = Quality;	// update our quality shadow
	m_OldCycleLen = m_CycleLen;	// save old palette's cycle length
	m_CycleLen = CycleLen;
	m_InitColorOfs = Offset;	// color delta is zero at start of tween
	m_ColorOffset = Offset;
}

void CTweenEngine::UpdatePalette(const DPalette& Palette, UINT CycleLen, double Offset)
{
	// This method is similar to CDibEngine::SetPalette except we use the current
	// quality and don't wait for the engine to be idle; we assume this method is
	// never called in exclusive mode, so we only update the base class palette.
	Palette.GetRGBBk(m_Palette.GetData(), m_Quality, CycleLen, Offset);
}

void CTweenEngine::UpdateTweenPalette(const DPalette& Palette, UINT CycleLen, double Offset)
{
	DWORD	*pPalette = m_Tweening ? m_NewPal.GetData() : m_TweenPal.GetData();
	Palette.GetRGBBk(pPalette, m_Quality, CycleLen, Offset);
	m_CycleLen = CycleLen;
	m_ColorOffset = Offset;
}

void CTweenEngine::TweenPalette()
{
	if (m_TweenPos >= 1) {	// if tween position reached end
		m_Tweening = FALSE;	// tween is done
		m_TweenPal.Copy(m_NewPal);	// set output palette to new palette
		return;
	}
	double	ColorDelta = m_ColorOffset - m_InitColorOfs;
	if (ColorDelta < 0)
		ColorDelta += 1;
	UINT	OldPalIdx = round(m_OldCycleLen * ColorDelta);
	// old and new palettes can be different sizes, so take minimum size
	int	Quality = min(int(m_Quality), INT64TO32(m_OldPal.GetSize()) - 1);
#if !TWEEN_MMX	// if using C
	for (int i = 0; i <= Quality; i++) {
		if (i < Quality) {	// if foreground color
			if (OldPalIdx >= m_OldCycleLen)	// if past end of old palette
				OldPalIdx = 0;	// wrap index
		} else	// background color
			OldPalIdx = i;	// no color cycling
		COLORREF	c0 = m_OldPal[OldPalIdx];	// get old color
		COLORREF	c1 = m_NewPal[i];	// get new color
		int	r0 = GetRValue(c0);
		int	tr = round(r0 + (GetRValue(c1) - r0) * m_TweenPos);	// tween red
		int	g0 = GetGValue(c0);
		int	tg = round(g0 + (GetGValue(c1) - g0) * m_TweenPos);	// tween green
		int	b0 = GetBValue(c0);
		int	tb = round(b0 + (GetBValue(c1) - b0) * m_TweenPos);	// tween blue
		m_TweenPal[i] = RGB(tr, tg, tb);	// write tweened color
		OldPalIdx++;
	}
#else	// using MMX
	DWORD	*pOldPal = m_OldPal.GetData();
	DWORD	*pNewPal = m_NewPal.GetData();
	DWORD	*pDest = m_TweenPal.GetData();
	WORD	pos = WORD(round(m_TweenPos * 512));
#if _WIN64
	TweenPalette64(pDest, pNewPal, pOldPal, Quality, m_OldCycleLen, OldPalIdx, pos);
#else
	__asm {
		mov		ecx, Quality
		mov		esi, pOldPal
		mov		edi, pNewPal
		mov		ebx, pDest
		pxor	mm3, mm3		// mm3 = 0
		mov		dx, pos
		mov		ax, dx
		shl		eax, 16
		mov		ax, dx			// eax = pos in both words
		movd	mm4, eax
		movd	mm5, eax
		punpcklwd	mm4, mm5	// mm4 = pos in all four words
		mov		eax, this
		mov		edx, [eax].m_OldCycleLen
		shl		edx, 2
		add		edx, esi		// edx = &m_OldPal[m_OldCycleLen]
		mov		eax, OldPalIdx
		shl		eax, 2
		add		esi, eax		// esi = &m_OldPal[OldPalIdx]
$tw1:
		cmp		esi, edx		// within old palette?
		jl		$tw2			// y, skip
		mov		esi, pOldPal	// n, wrap pointer
$tw2:
		movd	mm1, [esi]
		movd	mm2, [edi]
		punpcklbw	mm1, mm3	// mm1 = old
		punpcklbw	mm2, mm3	// mm2 = new
		psubsw		mm2, mm1	// mm2 = new - old
		psllw	mm2, 7			// scale is pos * 512
		pmulhw	mm2, mm4		// mm2 = (new - old) * pos
		paddsw	mm1, mm2		// mm1 = old + (new - old) * pos
		packuswb	mm1, mm3
		movd	[ebx], mm1		// store tweened color
		add		esi, 4
		add		edi, 4
		add		ebx, 4
		dec		ecx
		jnz		$tw1			// exclude background color; handled below

		mov		esi, pOldPal	// tween background color
		mov		eax, Quality
		movd	mm1, [esi + eax * 4]	// esi = &m_OldPal[Quality]
		movd	mm2, [edi]
		punpcklbw	mm1, mm3	// mm1 = old
		punpcklbw	mm2, mm3	// mm2 = new
		psubsw		mm2, mm1	// mm2 = new - old
		psllw	mm2, 7			// scale is pos * 512
		pmulhw	mm2, mm4		// mm2 = (new - old) * pos
		paddsw	mm1, mm2		// mm1 = old + (new - old) * pos
		packuswb	mm1, mm3
		movd	[ebx], mm1		// store tweened color

		emms	// crucial: empty MMX state!
	}
#endif
#endif
	m_TweenPos += m_TweenDelta;	// update tween position
}

void CTweenEngine::MapColorEx(CDib& Dib, const DIB_INFO& Info, bool Mirror, CPoint Origin)
{
	ASSERT(Info.BitCount == 24);	// 24 bits per pixel ONLY
	DWORD	*pPalette;
	if (m_Tweening)	// if tween in progress
		TweenPalette();	// do tween
	pPalette = m_TweenPal.GetData();	// use tween output palette
	int	Width = Info.Size.cx;
	int	Height = Info.Size.cy;
	const ESCTIME	*pEscFrame;
	int	MirrorDelta;
	if (Mirror) {
		ASSERT(Width == (m_DibInfo.Size.cx >> 1));
		ASSERT(Height == (m_DibInfo.Size.cy >> 1));
		pEscFrame = m_EscFrame.GetData()
			+ Origin.y * m_EscFrameSize.cx * m_Antialias
			+ (Width - Origin.x) * m_Antialias;
		MirrorDelta = m_EscFrameSize.cx >> 1;
	} else {
		ASSERT(Width == m_DibInfo.Size.cx);
		ASSERT(Height == m_DibInfo.Size.cy);
		pEscFrame = m_EscFrame.GetData();
		MirrorDelta = 0;
	}
	ASSERT(Origin.x >= 0 && Origin.x < Width);	// validate origin
	ASSERT(Origin.y >= 0 && Origin.y < Height);
	PBYTE	pDibRow = (PBYTE)Dib.GetBits();	// output to caller's bitmap
	if (m_Antialias > 1) {
#if !(defined(_WIN64) && ANTIALIAS_MMX)	// if not calling 64-bit MMX
		int	Factor = m_Antialias;
		int	FactorSquared = Factor * Factor;
		int	Stride = m_EscFrameSize.cx;
		int	Delta1 = Stride - Factor;
		int	Delta2 = Stride * Factor - Factor;
		int	Delta3 = MirrorDelta + Stride * (Factor - 1);
#endif
#if !ANTIALIAS_MMX	// if using C
		for (int y = 0; y < Height; y++) {
			PBYTE	pDib = pDibRow;
			for (int x = 0; x < Width; x++) {
				int	r = 0;
				int	g = 0;
				int	b = 0;
				for (int yi = 0; yi < Factor; yi++) {
					for (int xi = 0; xi < Factor; xi++) {
						COLORREF	color = pPalette[*pEscFrame++];
						b += GetBValue(color);
						g += GetGValue(color);
						r += GetRValue(color);
					}
					pEscFrame += Delta1;
				}
				pEscFrame -= Delta2;
				*pDib++ = BYTE(b / FactorSquared);
				*pDib++ = BYTE(g / FactorSquared);
				*pDib++ = BYTE(r / FactorSquared);
			}
			pDibRow += Info.Stride;
			pEscFrame += Delta3;
		}
#else	// using MMX
#ifdef	_WIN64
		MapColorEx64(Width, Height, pPalette, pEscFrame, pDibRow, 
			m_Antialias, m_EscFrameSize.cx, Info.Stride, MirrorDelta);
#else
		Delta1 <<= 2;	// convert to dword offset
		Delta2 <<= 2;
		Delta3 <<= 2;
		int	DibStride = Info.Stride;
		__asm {
#if _MFC_VER > 0x0600
			mov		eax, 010000h
			xor		edx, edx
			mov		ecx, FactorSquared
			div		ecx			// eax = 65536 / FactorSquared
			shr		ecx, 1
			cmp		edx, ecx	// remainder < divisor / 2 ?
			jb		$aa0		// y, skip
			inc		eax			// n, round quotient up
$aa0:
			movd	mm5, eax 
			movq	mm6, mm5
			punpcklwd	mm5, mm6	// mm5 = quotient in two low words
			movq	mm6, mm5
			punpcklwd	mm5, mm6	// mm5 = quotient in all four words
#endif
			mov		ecx, Height	// y = Height
			mov		esi, pEscFrame
			mov		ebx, pDibRow
			pxor	mm3, mm3	// zero for unpacking RGB values
			movd	mm4, Factor	// avoids memory accesses in inner loops
$aa1:
			// y loop
			push	ecx			// save y
			push	ebx			// save pDibRow; pDib = pDibRow
			mov		ecx, Width	// x = Width
$aa2:
			// x loop
			push	ecx			// save x
			push	ebx			// save pDib
			movd	edi, mm4	// yi = Factor
			mov		ebx, pPalette	// ebx = palette data
			mov		edx, Delta1	// edx = Delta1
			pxor	mm1, mm1	// zero RGB sum
$aa3:
			// yi loop
			movd	ecx, mm4	// xi = Factor
$aa4:
			// xi loop
			mov		eax, [esi]	// load pEscFrame
			movd	mm2, [ebx + eax * 4]	// mm2 = pPalette[*pEscFrame]
			punpcklbw	mm2, mm3	// unpack RGB value into four words
			paddw	mm1, mm2	// add RGB value to RGB sum
			add		esi, 4		// pEscFrame++
			dec		ecx			// xi--
			jnz		$aa4		// next xi

			add		esi, edx	// pEscFrame += Delta1
			dec		edi			// yi--
			jnz		$aa3		// next yi

#if _MFC_VER > 0x0600
			pmulhuw	mm1, mm5	// divide RGB sums by FactorSquared
			packuswb	mm1, mm3	// pack RGB averages into dword
			pop		ebx			// restore pDib
			sub		esi, Delta2	// pEscFrame -= Delta2
			movd	eax, mm1	// eax = average RGB value
			mov		edx, eax
			shr		edx, 16
			mov		[ebx], dl	// *pDib = b
			inc		ebx			// pDib++
			mov		[ebx], ah	// *pDib = g
			inc		ebx			// pDib++
			mov		[ebx], al	// *pDib = r
			inc		ebx			// pDib++
			pop		ecx			// restore x
			dec		ecx			// x--
			jnz		$aa2		// next x
#else
			//  no SSE: substitute divides for pmulhuw
			movq	mm2, mm1	// mm1 = mm2 = RGB sum
			punpckhdq	mm2, mm3	// mm2 = high dword of RGB sum
			pop		ebx			// restore pDib
			sub		esi, Delta2	// pEscFrame -= Delta2
			mov		ecx, FactorSquared
			xor		edx, edx	// zero high dword of dividend
			movd	eax, mm2	// eax = b
			div		ecx			// b /= FactorSquared
			mov		[ebx], al	// *pDib = b
			inc		ebx			// pDib++
			xor		edx, edx	// zero high dword of dividend
			movd	eax, mm1
			shr		eax, 16		// eax = g
			div		ecx			// g /= FactorSquared
			mov		[ebx], al	// *pDib = g
			inc		ebx			// pDib++
			xor		edx, edx	// zero high dword of dividend
			movd	eax, mm1
			and		eax, 0ffffh	// eax = r
			div		ecx			// r /= FactorSquared
			mov		[ebx], al	// *pDib = r
			inc		ebx			// pDib++
			pop		ecx			// restore x
			dec		ecx			// x--
			jnz		$aa2		// next x
#endif

			add		esi, Delta3	// pEscFrame += Delta3
			pop		ebx			// restore pDibRow
			add		ebx, DibStride	// pDibRow += DibStride
			pop		ecx			// restore y
			dec		ecx			// y--
			jnz		$aa1		// next y

			emms	// crucial: empty MMX state!
		}
#endif
#endif
	} else {	// no antialiasing, so just map escape times to colors
		for (int y = 0; y < Height; y++) {
			PBYTE	pDib = pDibRow;
			for (int x = 0; x < Width; x++) {
				COLORREF	color = pPalette[*pEscFrame++];
				*pDib++ = GetBValue(color);
				*pDib++ = GetGValue(color);
				*pDib++ = GetRValue(color);
			}
			pEscFrame += MirrorDelta;
			pDibRow += Info.Stride;
		}
	}
}

void CTweenEngine::MixDibs(CDib& Dst, CDib& Src, const DIB_INFO& Info, double MixPos)
{
	BYTE	*pSrc = (BYTE *)Src.GetBits();
	BYTE	*pDst = (BYTE *)Dst.GetBits();
#if !MIXDIBS_MMX	// if using C
	int	RowBytes = Info.Size.cx * 3;
	double	RevMixPos = 1 - MixPos;
	for (int y = 0; y < Info.Size.cy; y++) {
		for (int x = 0; x < RowBytes; x++) {
			pDst[x] = BYTE(round(pDst[x] * RevMixPos + pSrc[x] * MixPos));
		}
		pSrc += Info.Stride;
		pDst += Info.Stride;
	}
#else	// using MMX
	WORD	pos = WORD(round(MixPos * 512));
	int	DWords = Info.Size.cy * (Info.Stride >> 2);
#ifdef	_WIN64
	MixDibs64(pDst, pSrc, DWords, pos);
#else
	__asm {
		mov		ecx, DWords
		mov		esi, pSrc
		mov		edi, pDst
		pxor	mm3, mm3		// mm3 = 0
		mov		dx, pos
		mov		ax, dx
		shl		eax, 16
		mov		ax, dx			// eax = MixPos in both words
		movd	mm4, eax
		movd	mm5, eax
		punpcklwd	mm4, mm5	// mm4 = MixPos in all four words
		mov		edx, 02000200h	// fixed point 512 in both words
		sub		edx, eax		// (1 - MixPos) in both words
		movd	mm5, edx
		movd	mm6, edx
		punpcklwd	mm5, mm6	// mm5 = 1 - MixPos in all four words
$md1:
		movd	mm1, [esi]
		movd	mm2, [edi]
		punpcklbw	mm1, mm3	// mm1 = src pixel
		punpcklbw	mm2, mm3	// mm2 = dst pixel
		psllw	mm1, 7			// fixed point is 512
		psllw	mm2, 7			// fixed point is 512
		pmulhw	mm1, mm4		// src *= MixPos
		pmulhw	mm2, mm5		// dst *= (1 - MixPos)
		paddsw	mm1, mm2		// mm1 = src * MixPos + dst * (1 - MixPos)
		packuswb	mm1, mm3
		movd	[edi], mm1		// store mixed pixel in dst
		add		esi, 4
		add		edi, 4
		dec		ecx
		jnz		$md1

		emms	// crucial: empty MMX state!
	}
#endif
#endif
}
