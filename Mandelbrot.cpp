// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
        01      07jan10	separate from engine

        Mandelbrot rendering
 
*/

#include "stdafx.h"
#include "EngineImpl.h"

void CEngine::CRenderer::MandelbrotFPU()
{
	PROLOGUE(double, m_Engine->m_Job, m_Engine->m_Bounds)
	double	a, zx, zy, zx2, zy2;
	double	bx1 = m_Engine->m_Bounds.x1;
	for (UINT y = y1; y < y2; y += dy) {
		m_CurRow = y;
		if (IsCanceled())
			break;
		a = bx1;
		for (UINT x = 0; x < Width; x++) {
			zx = a;
			zy = b;
			ESCTIME	i;
			for (i = 0; i < Quality; i++) {
				zx2 = zx * zx;
				zy2 = zy * zy;
				if (zx2 + zy2 >= 4.0)
					break;
				zy = 2 * zx * zy + b;
				zx = zx2 - zy2 + a;
			}
			*pFrame++ = i;
			a += da;
		}
		pFrame += RowDelta;
		b += db;
	}
}

void CEngine::CRenderer::MandelbrotGMP()
{
	PROLOGUE(BigNum, m_Engine->m_Job, m_Engine->m_Bounds)
	UINT	prec = b.GetPrec();
	BigNum	a(prec, 0), zx(prec, 0), zy(prec, 0);
	BigNum	zx2(prec, 0), zy2(prec, 0), tst(prec, 0);
	for (UINT y = y1; y < y2; y += dy) {
		m_CurRow = y;
		mpf_set(a, m_Engine->m_Bounds.x1);
		for (UINT x = 0; x < Width; x++) {
			if (IsCanceled())
				return;
			mpf_set(zx, a);
			mpf_set(zy, b);
			ESCTIME	i;
			for (i = 0; i < Quality; i++) {
				mpf_mul(zx2, zx, zx);
				mpf_mul(zy2, zy, zy);
				mpf_add(tst, zx2, zy2);
				if (mpf_cmp_d(tst, 4.0) >= 0)
					break;
				mpf_mul(zy, zx, zy);
				mpf_mul_ui(zy, zy, 2);
				mpf_add(zy, zy, b);
				mpf_sub(zx, zx2, zy2);
				mpf_add(zx, zx, a);
			}
			*pFrame++ = i;
			mpf_add(a, a, da);
		}
		pFrame += RowDelta;
		mpf_add(b, b, db);
	}
}

extern "C" {
	void	Mandelbrot64(
		int Width, int Height, int y1, int y2, int dy, int RowDelta, UINT Quality,
		ESCTIME *pEscTime, volatile bool *pCanceled, volatile UINT *pCurRow,
		double da, double db, double b, double bx1);
}

#if _MFC_VER > 0x0600
#ifdef _WIN64
void CEngine::CRenderer::MandelbrotSSE2()
{
	PROLOGUE(double, m_Engine->m_Job, m_Engine->m_Bounds)
	Mandelbrot64(Width, Height, y1, y2, dy, RowDelta, Quality, 
		pFrame, &m_Canceled, &m_CurRow, da, db, b, m_Engine->m_Bounds.x1);
}
#else
void CEngine::CRenderer::MandelbrotSSE2()
{
	PROLOGUE(double, m_Engine->m_Job, m_Engine->m_Bounds)
	RowDelta <<= 2;
	double	a = m_Engine->m_Bounds.x1;
	double	radius = 4.0;
	// SSE vars require 16-byte alignment, but the stack isn't necessarily
	// 16-byte aligned; the compiler handles this by using the bx register
	// as a 2nd frame pointer, so we can't use bx, else we get error C4731
	__m128	a2;
	__m128	da2;
	__m128	db2;
	// derived from "Generating Fractals with SSE/SSE2" by Peter Kankowski
	__asm {
		mov		edi, pFrame
		XORPD	xmm2, xmm2		; xmm2 = 0
		MOVSD	xmm4, a
		SHUFPD	xmm4, xmm4, 0	; xmm4 = a
		MOVHPD	xmm2, da		; xmm2.H = da, xmm2.L = 0
		ADDPD	xmm4, xmm2		; xmm4.H = a + da, xmm4.L = a
		MOVAPD	a2, xmm4		; a2 = xmm4
		SHUFPD	xmm2, xmm2, 3	; xmm2 = da
		ADDPD	xmm2, xmm2		; xmm2 = da * 2
		MOVAPD	da2, xmm2		; da2 = xmm2
		MOVSD	xmm5, b
		SHUFPD	xmm5, xmm5, 0	; xmm5 = b
		MOVSD	xmm3, db
		SHUFPD	xmm3, xmm3, 0	; xmm3 = db
		MOVAPD	db2, xmm3		; db2 = xmm3
		MOVSD	xmm7, radius
		SHUFPD	xmm7, xmm7, 0	; xmm7 = radius
		mov		edx, y1			; edx = y = initial row
		cmp		edx, y2			; at least one row?
		jae		ydone			; no, done
yloop:
		mov		eax, dword ptr this
		mov		[eax].m_CurRow, edx	; m_CurRow = y
		mov		al, [eax].m_Canceled
		test	al, al			; m_Canceled?
		jne		ydone			; yes, abort
		MOVAPD	xmm4, a2		; xmm4.H = a + da, xmm4.L = a
		mov		esi, Width		; esi = x = width in pixels
xloop:
		MOVAPD	xmm0, xmm4		; xmm0 = zx = a
		XORPD	xmm6, xmm6		; xmm6 = 0
		MOVAPD	xmm1, xmm5		; xmm1 = zy = b
		mov		ecx, Quality	; ecx = iteration count
iloop:
		MOVAPD	xmm2, xmm0		; xmm2 = zx
		MULPD	xmm0, xmm0		; xmm0 = zx ^ 2
		MOVAPD	xmm3, xmm1		; xmm3 = zy
		ADDPD	xmm1, xmm1		; xmm1 = 2 * zy
		MULPD	xmm1, xmm2		; xmm1 = 2 * zx * zy
		MOVAPD	xmm2, xmm0		; xmm2 = zx ^ 2
		MULPD	xmm3, xmm3		; xmm3 = zy ^ 2
		ADDPD	xmm1, xmm5		; xmm1 = 2 * zx * zy + b
		SUBPD	xmm0, xmm3		; xmm0 = zx ^ 2 - zy ^ 2
		ADDPD	xmm2, xmm3		; xmm2 = zx ^ 2 + zy ^ 2
		CMPLTPD	xmm2, xmm7		; xmm2 = xmm2 < 4.0 ? 0xffffffff : 0
		ADDPD	xmm0, xmm4		; xmm0 = zx ^ 2 - zy ^ 2 + a
		MOVMSKPD eax, xmm2		; eax = xmm2 sign mask
		test	eax, eax		; both pixels escaped?
		jz		idone			; yes, early out
		ANDPD	xmm2, xmm7		; xmm2 = not escaped yet ? 4.0 : 0.0
		ADDPD	xmm6, xmm2		; xmm6 += xmm2
		sub		ecx, 1			; one iteration done, count > 0?
		jnz		iloop			; yes, next iteration
idone:
		CVTTSD2SI	eax, xmm6	; truncate radius sum to integer
		shr		eax, 2			; radius sum / 4 = iteration count
		mov		[edi], eax		; pFrame[0] = iteration count
		SHUFPD	xmm6, xmm6, 1	; exchange high and low radius sums
		CVTTSD2SI	eax, xmm6	; truncate radius sum to integer
		shr		eax, 2			; radius sum / 4 = iteration count
		mov		[edi + 4], eax	; pFrame[1] = iteration count
		add		edi, 8			; pFrame += 2
		ADDPD	xmm4, da2		; a += da
		sub		esi, 2			; two pixels done, x > 0?
		ja		xloop			; yes, next pair of pixels
		ADDPD	xmm5, db2		; b += db
		add		edi, RowDelta	; pFrame += RowDelta
		add		edx, dy			; y += dy
		cmp		edx, y2			; one row done, y < y2?
		jb		yloop			; yes, next row
ydone:
	}
}
#endif	// _WIN64
#endif	// _MFC_VER

#define MAND_EXP_HDR								\
	double	a, zx, zy, zx2, zy2;					\
	double	bx1 = m_Engine->m_Bounds.x1;			\
	for (UINT y = y1; y < y2; y += dy) {			\
		m_CurRow = y;								\
		if (IsCanceled())							\
			break;									\
		a = bx1;									\
		for (UINT x = 0; x < Width; x++) {			\
			zx = a;									\
			zy = b;									\
			ESCTIME	i;								\
			for (i = 0; i < Quality; i++) {			\
				zx2 = zx * zx;						\
				zy2 = zy * zy;						\
				if (zx2 + zy2 >= 4.0)				\
					break;

#define MAND_EXP_FTR								\
			}										\
			*pFrame++ = i;							\
			a += da;								\
		}											\
		pFrame += RowDelta;							\
		b += db;									\
	}

#define SQRC(a, b) zx##a = zx##b * zx; zy##a = zy##b * zy;

void CEngine::CRenderer::MandelbrotE3FPU()
{
	PROLOGUE(double, m_Engine->m_Job, m_Engine->m_Bounds)
	MAND_EXP_HDR
	zx = zx2 * zx - 3 * zx * zy2 + a;
	zy = 3 * zx2 * zy - zy2 * zy + b;
	MAND_EXP_FTR
}

void CEngine::CRenderer::MandelbrotE4FPU()
{
	PROLOGUE(double, m_Engine->m_Job, m_Engine->m_Bounds)
	double	zx3, zy3, pzx;
	MAND_EXP_HDR
	SQRC(3, 2)
	pzx = zx;
	zx = zx3 * zx - 6 * zx2 * zy2 + zy3 * zy + a;
	zy = 4 * zx3 * zy - 4 * pzx * zy3 + b;
	MAND_EXP_FTR
}

void CEngine::CRenderer::MandelbrotE5FPU()
{
	PROLOGUE(double, m_Engine->m_Job, m_Engine->m_Bounds)
	double	zx3, zy3, zx4, zy4;
	MAND_EXP_HDR
	SQRC(3, 2) SQRC(4, 3)
	zx = zx4 * zx - 10 * zx3 * zy2 + 5 * zx * zy4 + a;
	zy = 5 * zx4 * zy - 10 * zx2 * zy3 + zy4 * zy + b;
	MAND_EXP_FTR
}

void CEngine::CRenderer::MandelbrotE6FPU()
{
	PROLOGUE(double, m_Engine->m_Job, m_Engine->m_Bounds)
	double	zx3, zy3, zx4, zy4, zx5, zy5, pzx;
	MAND_EXP_HDR
	SQRC(3, 2) SQRC(4, 3) SQRC(5, 4)
	pzx = zx;
	zx = zx5 * zx - 15 * zx4 * zy2 + 15 * zx2 * zy4 - zy5 * zy + a;
	zy = 6 * zx5 * zy - 20 * zx3 * zy3 + 6 * pzx * zy5 + b;
	MAND_EXP_FTR
}

void CEngine::CRenderer::MandelbrotE7FPU()
{
	PROLOGUE(double, m_Engine->m_Job, m_Engine->m_Bounds)
	double	zx3, zy3, zx4, zy4, zx5, zy5, zx6, zy6;
	MAND_EXP_HDR
	SQRC(3, 2) SQRC(4, 3) SQRC(5, 4) SQRC(6, 5)
	zx = zx6 * zx - 21 * zx5 * zy2 + 35 * zx3 * zy4 - 7 * zx * zy6 + a;
	zy = 7 * zx6 * zy - 35 * zx4 * zy3 + 21 * zx2 * zy5 - zy6 * zy + b;
	MAND_EXP_FTR
}

void CEngine::CRenderer::MandelbrotE8FPU()
{
	PROLOGUE(double, m_Engine->m_Job, m_Engine->m_Bounds)
	double	zx3, zy3, zx4, zy4, zx5, zy5, zx6, zy6, zx7, zy7, pzx;
	MAND_EXP_HDR
	SQRC(3, 2) SQRC(4, 3) SQRC(5, 4) SQRC(6, 5) SQRC(7, 6)
	pzx = zx;
	zx = zx7 * zx - 28 * zx6 * zy2 + 70 * zx4 * zy4 - 28 * zx2 * zy6 + zy7 * zy + a;
	zy = 8 * zx7 * zy - 56 * zx5 * zy3 + 56 * zx3 * zy5 - 8 * pzx * zy7 + b;
	MAND_EXP_FTR
}

void CEngine::CRenderer::MandelbrotE9FPU()
{
	PROLOGUE(double, m_Engine->m_Job, m_Engine->m_Bounds)
	double	zx3, zy3, zx4, zy4, zx5, zy5, zx6, zy6, zx7, zy7, zx8, zy8;
	MAND_EXP_HDR
	SQRC(3, 2) SQRC(4, 3) SQRC(5, 4) SQRC(6, 5) SQRC(7, 6) SQRC(8, 7)
	zx = zx8 * zx - 36 * zx7 * zy2 + 126 * zx5 * zy4 - 84 * zx3 * zy6 + 9 * zx * zy8 + a;
	zy = 9 * zx8 * zy - 84 * zx6 * zy3 + 126 * zx4 * zy5 - 36 * zx2 * zy7 + zy8 * zy + b;
	MAND_EXP_FTR
}

void CEngine::CRenderer::MandelbrotE10FPU()
{
	PROLOGUE(double, m_Engine->m_Job, m_Engine->m_Bounds)
	double	zx3, zy3, zx4, zy4, zx5, zy5, zx6, zy6, zx7, zy7, zx8, zy8, zx9, zy9, pzx;
	MAND_EXP_HDR
	SQRC(3, 2) SQRC(4, 3) SQRC(5, 4) SQRC(6, 5) SQRC(7, 6) SQRC(8, 7) SQRC(9, 8)
	pzx = zx;
	zx = zx9 * zx - 45 * zx8 * zy2 + 210 * zx6 * zy4 - 210 * zx4 * zy6 + 45 * zx2 * zy8 - zy9 * zy + a;
	zy = 10 * zx9 * zy - 120 * zx7 * zy3 + 252 * zx5 * zy5 - 120 * zx3 * zy7 + 10 * pzx * zy9 + b;
	MAND_EXP_FTR
}

void CEngine::CRenderer::MandelbrotE11FPU()
{
	PROLOGUE(double, m_Engine->m_Job, m_Engine->m_Bounds)
	double	zx3, zy3, zx4, zy4, zx5, zy5, zx6, zy6, zx7, zy7, zx8, zy8, zx9, zy9, zx10, zy10;
	MAND_EXP_HDR
	SQRC(3, 2) SQRC(4, 3) SQRC(5, 4) SQRC(6, 5) SQRC(7, 6) SQRC(8, 7) SQRC(9, 8) SQRC(10, 9)
	zx = zx10 * zx - 55 * zx9 * zy2 + 330 * zx7 * zy4 - 462 * zx5 * zy6 + 165 * zx3 * zy8 - 11 * zx * zy10 + a;
	zy = 11 * zx10 * zy - 165 * zx8 * zy3 + 462 * zx6 * zy5 - 330 * zx4 * zy7 + 55 * zx2 * zy9 - zy10 * zy + b;
	MAND_EXP_FTR
}

void CEngine::CRenderer::MandelbrotE12FPU()
{
	PROLOGUE(double, m_Engine->m_Job, m_Engine->m_Bounds)
	double	zx3, zy3, zx4, zy4, zx5, zy5, zx6, zy6, zx7, zy7, zx8, zy8, zx9, zy9, zx10, zy10, zx11, zy11, pzx;
	MAND_EXP_HDR
	SQRC(3, 2)	SQRC(4, 3)	SQRC(5, 4)	SQRC(6, 5)	SQRC(7, 6) SQRC(8, 7) SQRC(9, 8) SQRC(10, 9) SQRC(11, 10)
	pzx = zx;
	zx = zx11 * zx - 66 * zx10 * zy2 + 495 * zx8 * zy4 - 924 * zx6 * zy6 + 495 * zx4 * zy8 - 66 * zx2 * zy10 + zy11 * zy + a;
	zy = 12 * zx11 * zy - 220 * zx9 * zy3 + 792 * zx7 * zy5 - 792 * zx5 * zy7 + 220 * zx3 * zy9 - 12 * pzx * zy11 + b;
	MAND_EXP_FTR
}

void CEngine::CRenderer::MandelbrotE13FPU()
{
	PROLOGUE(double, m_Engine->m_Job, m_Engine->m_Bounds)
	double	zx3, zy3, zx4, zy4, zx5, zy5, zx6, zy6, zx7, zy7, zx8, zy8, zx9, zy9, zx10, zy10, zx11, zy11, zx12, zy12;
	MAND_EXP_HDR
	SQRC(3, 2) SQRC(4, 3) SQRC(5, 4) SQRC(6, 5) SQRC(7, 6) SQRC(8, 7) SQRC(9, 8) SQRC(10, 9) SQRC(11, 10) SQRC(12, 11)
	zx = zx12 * zx - 78 * zx11 * zy2 + 715 * zx9 * zy4 - 1716 * zx7 * zy6 + 1287 * zx5 * zy8 - 286 * zx3 * zy10 + 13 * zx * zy12 + a;
	zy = 13 * zx12 * zy - 286 * zx10 * zy3 + 1287 * zx8 * zy5 - 1716 * zx6 * zy7 + 715 * zx4 * zy9 - 78 * zx2 * zy11 + zy12 * zy + b;
	MAND_EXP_FTR
}
