; Copyleft 2010 Chris Korda
; This program is free software; you can redistribute it and/or modify it
; under the terms of the GNU General Public License as published by the Free
; Software Foundation; either version 2 of the License, or any later version.
;
;       chris korda
;
;		revision history:
;		rev		date	comments
;       00      27jan10	initial version
;		01		01feb10	add MixDibs64 and TweenPalette64
;
;		tween engine 64-bit MMX
;

%include "macro64.inc"

global	MapColorEx64

%define	Width		argdef(0)	; int		Width			width of output bitmap, in pixels
%define	Height		argdef(1)	; int		Height			height of output bitmap, in pixels
%define	pPalette	argdef(2)	; const DWORD *pPalette		pointer to array of RGB values
%define	pEscFrame	argdef(3)	; const ESCTIME *pEscFrame	pointer to array of escape times
%define	pDibRow		argdef(4)	; PBYTE	pDibRow				pointer to output bitmap data
%define	Factor		argdef(5)	; int		Factor			antialiasing factor, from 1 to 16
%define	Stride		argdef(6)	; int		Stride			size of escape time array row, in bytes
%define	DibStride	argdef(7)	; int		DibStride		size of output bitmap row, in bytes
%define	MirrorDelta	argdef(8)	; int		MirrorDelta		mirror quadrant delta, in bytes

%define	StackSpace	8 * 6
%define	SaveSI		vardef(0)
%define	SaveDI		vardef(1)
%define	SaveBX		vardef(2)
%define	Delta1		vardef(3)
%define	Delta2		vardef(4)
%define	Delta3		vardef(5)

proc_frame	MapColorEx64
		rex_push_reg	rbp		; push frame pointer
		alloc_stack	StackSpace	; allocate local stack
		set_frame	rbp, FrameBias	; set frame register
		mov		SaveSI, rsi
		mov		SaveDI, rdi
		mov		SaveBX, rbx
end_prologue

		mov		Width, rcx		; copy register args to shadow memory	
		mov		Height, rdx
		mov		pPalette, r8
		mov		pEscFrame, r9
		mov		eax, DibStride
		mov		DibStride, rax	; zero high dword
		mov		r8d, Stride
		mov		r9d, Factor
		mov		rax, r8
		sub		rax, r9		; rax = Stride - Factor
		shl		rax, 2		; convert to dword offset
		mov		Delta1, rax	; store Delta1
		mov		rax, r8
		mul		r9
		sub		rax, r9		; rax = Stride * Factor - Factor
		shl		rax, 2		; convert to dword offset
		mov		Delta2, rax	; store Delta2
		mov		rax, r8
		mov		rcx, r9
		dec		rcx
		mul		rcx
		mov		ecx, MirrorDelta
		add		rax, rcx	; rax = Stride * (Factor - 1) + MirrorDelta
		shl		rax, 2		; convert to dword offset
		mov		Delta3, rax	; store Delta3
		mov		rax, r9
		mul		rax
		mov		rcx, rax	; rcx = FactorSquared
		mov		rax, 010000h
		xor		rdx, rdx
		div		rcx			; rax = 65536 / FactorSquared
		shr		rcx, 1
		cmp		rdx, rcx	; remainder < divisor / 2 ?
		jb		$aa0		; y, skip
		inc		rax			; n, round quotient up
$aa0:
		movd	mm5, eax 
		movq	mm6, mm5
		punpcklwd	mm5, mm6	; mm5 = quotient in two low words
		movq	mm6, mm5
		punpcklwd	mm5, mm6	; mm5 = quotient in all four words
		mov		r8d, Height	; y = Height
		mov		rsi, pEscFrame
		mov		rdi, pDibRow
		pxor	mm3, mm3	; zero for unpacking RGB values
		mov		r11d, Factor	; avoids memory accesses in inner loops
$aa1:
		; y loop
		push	rdi			; save pDibRow; pDib = pDibRow
		mov		r9d, Width	; x = Width
$aa2:
		; x loop
		mov		r10, r11	; yi = Factor
		mov		rbx, pPalette	; rbx = palette data
		mov		rdx, Delta1	; rdx = Delta1
		pxor	mm1, mm1	; zero RGB sum
		xor		rax, rax	; clear high dword of escape time
$aa3:
		; yi loop
		mov		rcx, r11	; xi = Factor
$aa4:
		; xi loop
		mov		eax, [rsi]	; rax = escape time at *pEscFrame
		movd	mm2, [rbx + rax * 4]	; mm2 = pPalette[*pEscFrame]
		punpcklbw	mm2, mm3	; unpack RGB value into four words
		paddw	mm1, mm2	; add RGB value to RGB sum
		add		rsi, 4		; pEscFrame++
		dec		rcx			; xi--
		jnz		$aa4		; next xi

		add		rsi, rdx	; pEscFrame += Delta1
		dec		r10			; yi--
		jnz		$aa3		; next yi

		pmulhuw	mm1, mm5	; divide RGB sums by FactorSquared
		packuswb	mm1, mm3	; pack RGB averages into dword
		sub		rsi, Delta2	; pEscFrame -= Delta2
		movd	eax, mm1	; eax = average RGB value
		mov		edx, eax
		shr		edx, 16
		mov		[rdi], dl	; *pDib = b
		inc		rdi			; pDib++
		mov		[rdi], ah	; *pDib = g
		inc		rdi			; pDib++
		mov		[rdi], al	; *pDib = r
		inc		rdi			; pDib++
		dec		r9			; x--
		jnz		$aa2		; next x

		add		rsi, Delta3	; pEscFrame += Delta3
		pop		rdi			; restore pDibRow
		add		rdi, DibStride	; pDibRow += DibStride
		dec		r8			; y--
		jnz		$aa1		; next y

		emms	; crucial: empty MMX state!
		
		mov		rsi, SaveSI
		mov		rdi, SaveDI
		mov		rbx, SaveBX
		add		rsp, StackSpace
		pop		rbp
		ret
	
endproc_frame

global	MixDibs64

%define	pDst		argdef(0)	; BYTE	*pDst
%define	pSrc		argdef(1)	; const BYTE	*pSrc
%define	DWords		argdef(2)	; int	DWords
%define	MixPos		argdef(3)	; WORD	MixPos (16-bit fixed point)

%define	StackSpace	8 * 2
%define	SaveSI		vardef(0)
%define	SaveDI		vardef(1)

proc_frame	MixDibs64
		rex_push_reg	rbp		; push frame pointer
		alloc_stack	StackSpace	; allocate local stack
		set_frame	rbp, FrameBias	; set frame register
		mov		SaveSI, rsi
		mov		SaveDI, rdi
end_prologue

		mov		rdi, rcx		; rdi = pDst
		mov		rsi, rdx		; rsi = pSrc
		mov		rcx, r8			; rcx = DWords
		pxor	mm3, mm3		; mm3 = 0
		mov		rdx, r9			; rdx = MixPos
		mov		eax, edx
		shl		eax, 16
		mov		ax, dx			; eax = MixPos in both words
		movd	mm4, eax
		movd	mm5, eax
		punpcklwd	mm4, mm5	; mm4 = MixPos in all four words
		mov		edx, 02000200h	; fixed point 512 in both words
		sub		edx, eax		; (1 - MixPos) in both words
		movd	mm5, edx
		movd	mm6, edx
		punpcklwd	mm5, mm6	; mm5 = 1 - MixPos in all four words
		
$md1:
		movd	mm1, [rsi]
		movd	mm2, [rdi]
		punpcklbw	mm1, mm3	; mm1 = src pixel
		punpcklbw	mm2, mm3	; mm2 = dst pixel
		psllw	mm1, 7			; fixed point is 512
		psllw	mm2, 7			; fixed point is 512
		pmulhw	mm1, mm4		; src *= MixPos
		pmulhw	mm2, mm5		; dst *= (1 - MixPos)
		paddsw	mm1, mm2		; mm1 = src * MixPos + dst * (1 - MixPos)
		packuswb	mm1, mm3
		movd	[rdi], mm1		; store mixed pixel in dst
		add		rsi, 4
		add		rdi, 4
		dec		rcx
		jnz		$md1

		emms	; crucial: empty MMX state!
		
		mov		rsi, SaveSI
		mov		rdi, SaveDI
		add		rsp, StackSpace
		pop		rbp
		ret
	
endproc_frame

global	TweenPalette64

%define	pDest		argdef(0)	; DWORD	*pDest
%define	pNewPal		argdef(1)	; DWORD	*pNewPal
%define	pOldPal		argdef(2)	; DWORD	*pOldPal
%define	Quality		argdef(3)	; UINT	Quality
%define	OldCycleLen	argdef(4)	; UINT	OldCycleLen
%define	OldPalIdx	argdef(5)	; UINT	OldPalIdx
%define	MixPos		argdef(6)	; WORD	pos (16-bit fixed point)

%define	StackSpace	8 * 3
%define	SaveSI		vardef(0)
%define	SaveDI		vardef(1)
%define	SaveBX		vardef(2)

proc_frame	TweenPalette64
		rex_push_reg	rbp		; push frame pointer
		alloc_stack	StackSpace	; allocate local stack
		set_frame	rbp, FrameBias	; set frame register
		mov		SaveSI, rsi
		mov		SaveDI, rdi
		mov		SaveBX, rbx
end_prologue

		mov		rbx, rcx		; rbx = pDest
		mov		rdi, rdx		; rdi = pNewPal
		mov		rsi, r8			; rsi = pOldPal
		mov		ecx, r9d		; rcx = Quality
		pxor	mm3, mm3		; mm3 = 0
		mov		eax, MixPos
		movd	mm4, eax
		movq	mm5, mm4
		punpcklwd	mm4, mm5	; mm4 = pos in two low words
		movq	mm5, mm4
		punpcklwd	mm4, mm5	; mm4 = pos in all four words
		mov		edx, OldCycleLen
		shl		rdx, 2
		add		rdx, rsi		; edx = &m_OldPal[m_OldCycleLen]
		mov		eax, OldPalIdx
		shl		rax, 2
		add		rsi, rax		; rsi = &m_OldPal[OldPalIdx]
$tw1:
		cmp		rsi, rdx		; within old palette?
		jl		$tw2			; y, skip
		mov		rsi, r8			; n, wrap pointer; rsi = pOldPal
$tw2:
		movd	mm1, [rsi]
		movd	mm2, [rdi]
		punpcklbw	mm1, mm3	; mm1 = old
		punpcklbw	mm2, mm3	; mm2 = new
		psubsw		mm2, mm1	; mm2 = new - old
		psllw	mm2, 7			; scale is pos * 512
		pmulhw	mm2, mm4		; mm2 = (new - old) * pos
		paddsw	mm1, mm2		; mm1 = old + (new - old) * pos
		packuswb	mm1, mm3
		movd	[rbx], mm1		; store tweened color
		add		rsi, 4
		add		rdi, 4
		add		rbx, 4
		dec		rcx
		jnz		$tw1			; exclude background color; handled below

		mov		rsi, r8			; tween background color; rsi = pOldPal
		mov		eax, r9d		; rax = Quality
		movd	mm1, [rsi + rax * 4]	; rsi = &m_OldPal[Quality]
		movd	mm2, [rdi]
		punpcklbw	mm1, mm3	; mm1 = old
		punpcklbw	mm2, mm3	; mm2 = new
		psubsw		mm2, mm1	; mm2 = new - old
		psllw	mm2, 7			; scale is pos * 512
		pmulhw	mm2, mm4		; mm2 = (new - old) * pos
		paddsw	mm1, mm2		; mm1 = old + (new - old) * pos
		packuswb	mm1, mm3
		movd	[rbx], mm1		; store tweened color

		emms	; crucial: empty MMX state!

		mov		rsi, SaveSI
		mov		rdi, SaveDI
		mov		rbx, SaveBX
		add		rsp, StackSpace
		pop		rbp
		ret
	
endproc_frame
