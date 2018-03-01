; Copyleft 2010 Chris Korda
; This program is free software; you can redistribute it and/or modify it
; under the terms of the GNU General Public License as published by the Free
; Software Foundation; either version 2 of the License, or any later version.
;
;       chris korda
;
;		revision history:
;		rev		date	comments
;       00      07jan10	initial version
;		01		27jan10	make x and iteration count regs 64-bit
;		02		01feb10	move arg and var macros to include file
;		03		16mar10	add quad version to process four pixels at once
;
;		Mandelbar 64-bit assembler
; 

%include "macro64.inc"

global	Mandelbar64

%define	Width		argdef(0)	; int	Width
%define	Height		argdef(1)	; int	Height
%define	y1			argdef(2)	; int	y1
%define	y2			argdef(3)	; int	y2
%define	dy			argdef(4)	; int	dy
%define	RowDelta	argdef(5)	; int	RowDelta
%define	Quality		argdef(6)	; UINT	Quality
%define	pFrame		argdef(7)	; ESCTIME	*pEscTime
%define	pCanceled	argdef(8)	; volatile bool	*pCanceled
%define	pCurRow		argdef(9)	; volatile UINT	*pCurRow
%define	fda			argdef(10)	; double da
%define	fdb			argdef(11)	; double db
%define	fb			argdef(12)	; double b
%define	fbx1		argdef(13)	; double bx1

%define	StackSpace	8 * 14
%define padding		vardef(0)
%define	SaveXMM6	vardef(1)	; must be 16-byte aligned
%define	SaveXMM7	vardef(3)
%define	SaveXMM8	vardef(5)
%define	SaveXMM9	vardef(7)
%define	SaveXMM10	vardef(9)
%define	SaveSI		vardef(11)
%define	SaveDI		vardef(12)
%define	radius		vardef(13)

proc_frame	Mandelbar64
		test	rcx, 3			; Width a multiple of four?
		jz		Mandelbar64Q	; yes, process four pixels at once
		rex_push_reg	rbp		; push frame pointer
		alloc_stack	StackSpace	; allocate local stack
		set_frame	rbp, FrameBias	; set frame register
		movdqa	SaveXMM6, xmm6
		movdqa	SaveXMM7, xmm7
		movdqa	SaveXMM8, xmm8
		movdqa	SaveXMM9, xmm9
		movdqa	SaveXMM10, xmm10
		mov		SaveSI, rsi
		mov		SaveDI, rdi
end_prologue

		mov		Width, rcx		; copy Width to shadow
		mov		r10d, RowDelta
		shl		r10, 2			; RowDelta <<= 2
		mov		r11d, Quality
		mov		rax, 0x4010000000000000
		mov		radius, rax		; radius = 4.0
		mov		rdi, pFrame
		XORPD	xmm2, xmm2		; xmm2 = 0
		MOVSD	xmm4, fbx1
		SHUFPD	xmm4, xmm4, 0	; xmm4 = a
		MOVHPD	xmm2, fda		; xmm2.H = da, xmm2.L = 0
		ADDPD	xmm4, xmm2		; xmm4.H = a + da, xmm4.L = a
		MOVAPD	xmm10, xmm4		; a2 = xmm4
		SHUFPD	xmm2, xmm2, 3	; xmm2 = da
		ADDPD	xmm2, xmm2		; xmm2 = da * 2
		MOVAPD	xmm8, xmm2		; da2 = xmm2
		MOVSD	xmm5, fb
		SHUFPD	xmm5, xmm5, 0	; xmm5 = b
		MOVSD	xmm3, fdb
		SHUFPD	xmm3, xmm3, 0	; xmm3 = db
		MOVAPD	xmm9, xmm3		; db2 = xmm3
		MOVSD	xmm7, radius
		SHUFPD	xmm7, xmm7, 0	; xmm7 = radius
		mov		edx, r8d		; edx = y = initial row
		cmp		edx, r9d		; at least one row?
		jae		ydone			; no, done
yloop:
		mov		eax, pCurRow
		mov		[eax], edx		; m_CurRow = y
		mov		eax, pCanceled
		mov		al, [eax]
		test	al, al			; m_Canceled?
		jne		ydone			; yes, abort
		MOVAPD	xmm4, xmm10		; xmm4.H = a + da, xmm4.L = a
		mov		esi, Width		; rsi = x = width in pixels
xloop:
		MOVAPD	xmm0, xmm4		; xmm0 = zx = a
		XORPD	xmm6, xmm6		; xmm6 = 0
		MOVAPD	xmm1, xmm5		; xmm1 = zy = b
		mov		rcx, r11		; rcx = iteration count
iloop:
		MOVAPD	xmm2, xmm0		; xmm2 = zx
		MULPD	xmm0, xmm0		; xmm0 = zx ^ 2
		MOVAPD	xmm3, xmm1		; xmm3 = zy
		ADDPD	xmm1, xmm1		; xmm1 = 2 * zy
		MULPD	xmm1, xmm2		; xmm1 = 2 * zx * zy
		MOVAPD	xmm2, xmm0		; xmm2 = zx ^ 2
		MULPD	xmm3, xmm3		; xmm3 = zy ^ 2
		SUBPD	xmm1, xmm5		; xmm1 = 2 * zx * zy - b
		ADDPD	xmm2, xmm3		; xmm2 = zx ^ 2 + zy ^ 2
		SUBPD	xmm3, xmm0		; xmm3 = zy ^ 2 - zx ^ 2
		CMPLTPD	xmm2, xmm7		; xmm2 = xmm2 < 4.0 ? 0xffffffff : 0
		SUBPD	xmm3, xmm4		; xmm3 = zy ^ 2 - zx ^ 2 - a
		MOVAPD	xmm0, xmm3		; xmm0 = next zx
		MOVMSKPD eax, xmm2		; eax = xmm2 sign mask
		test	eax, eax		; both pixels escaped?
		jz		idone			; yes, early out
		ANDPD	xmm2, xmm7		; xmm2 = not escaped yet ? 4.0 : 0.0
		ADDPD	xmm6, xmm2		; xmm6 += xmm2
		sub		rcx, 1			; one iteration done, count > 0?
		jnz		iloop			; yes, next iteration
idone:
		CVTPD2DQ	xmm0, xmm6	; truncate radius sum to integer
		PSRLW	xmm0, 2			; radius sum / 4 = iteration count
		movq	[rdi], xmm0		; pFrame[0] = iteration count
		add		rdi, 8			; pFrame += 2
		ADDPD	xmm4, xmm8		; a += da
		sub		rsi, 2			; two pixels done, x > 0?
		ja		xloop			; yes, next pair of pixels
		ADDPD	xmm5, xmm9		; b += db
		add		rdi, r10		; pFrame += RowDelta
		add		edx, dy			; y += dy
		cmp		edx, r9d		; one row done, y < y2?
		jb		yloop			; yes, next row
ydone:
		movdqa	xmm6, SaveXMM6
		movdqa	xmm7, SaveXMM7
		movdqa	xmm8, SaveXMM8
		movdqa	xmm9, SaveXMM9
		movdqa	xmm10, SaveXMM10
		mov		rsi, SaveSI
		mov		rdi, SaveDI
		add		rsp, StackSpace
		pop		rbp
		ret

endproc_frame

%define	Width		argdef(0)	; int	Width
%define	Height		argdef(1)	; int	Height
%define	y1			argdef(2)	; int	y1
%define	y2			argdef(3)	; int	y2
%define	dy			argdef(4)	; int	dy
%define	RowDelta	argdef(5)	; int	RowDelta
%define	Quality		argdef(6)	; UINT	Quality
%define	pFrame		argdef(7)	; ESCTIME	*pEscTime
%define	pCanceled	argdef(8)	; volatile bool	*pCanceled
%define	pCurRow		argdef(9)	; volatile UINT	*pCurRow
%define	fda			argdef(10)	; double da
%define	fdb			argdef(11)	; double db
%define	fb			argdef(12)	; double b
%define	fbx1		argdef(13)	; double bx1

%define	StackSpace	8 * 28
%define SaveR12		vardef(0)
%define	SaveXMM6	vardef(1)	; must be 16-byte aligned
%define	SaveXMM7	vardef(3)
%define	SaveXMM8	vardef(5)
%define	SaveXMM9	vardef(7)
%define	SaveXMM10	vardef(9)
%define	SaveXMM11	vardef(11)
%define	SaveXMM12	vardef(13)
%define	SaveXMM13	vardef(15)
%define	SaveXMM14	vardef(17)
%define	SaveXMM15	vardef(19)
%define	a01			vardef(21)
%define a23			vardef(23)
%define	SaveSI		vardef(25)
%define	SaveDI		vardef(26)
%define	radius		vardef(27)

proc_frame	Mandelbar64Q
		rex_push_reg	rbp		; push frame pointer
		alloc_stack	StackSpace	; allocate local stack
		set_frame	rbp, FrameBias	; set frame register
		movdqa	SaveXMM6, xmm6
		movdqa	SaveXMM7, xmm7
		movdqa	SaveXMM8, xmm8
		movdqa	SaveXMM9, xmm9
		movdqa	SaveXMM10, xmm10
		movdqa	SaveXMM11, xmm11
		movdqa	SaveXMM12, xmm12
		movdqa	SaveXMM13, xmm13
		movdqa	SaveXMM14, xmm14
		movdqa	SaveXMM15, xmm15
		mov		SaveSI, rsi
		mov		SaveDI, rdi
		mov		SaveR12, r12
end_prologue

		mov		Width, rcx		; copy Width to shadow
		mov		r10d, RowDelta
		shl		r10, 2			; RowDelta <<= 2
		mov		r11d, Quality
		mov		rax, 0x4010000000000000
		mov		radius, rax		; radius = 4.0
		mov		rdi, pFrame
		XORPD	xmm14, xmm14	; xmm14 = 0
		MOVSD	xmm4, fbx1
		SHUFPD	xmm4, xmm4, 0	; xmm4 = a
		MOVHPD	xmm14, fda		; xmm14.H = da, xmm14.L = 0
		ADDPD	xmm4, xmm14		; xmm4.H = a + da, xmm4.L = a
		MOVAPD	a01, xmm4		; a01 = xmm4
		SHUFPD	xmm14, xmm14, 3	; xmm2 = da
		ADDPD	xmm14, xmm14	; xmm2 = da * 2
		ADDPD	xmm4, xmm14		; xmm4.H = a + da * 3, xmm4.L = a + da * 2
		MOVAPD	a23, xmm4		; a23 = xmm4
		ADDPD	xmm14, xmm14	; xmm14 = da * 4
		MOVSD	xmm6, fb
		SHUFPD	xmm6, xmm6, 0	; xmm6 = b
		MOVSD	xmm15, fdb
		SHUFPD	xmm15, xmm15, 0	; xmm15 = db
		MOVSD	xmm7, radius
		SHUFPD	xmm7, xmm7, 0	; xmm7 = radius
		mov		edx, r8d		; edx = y = initial row
		cmp		edx, r9d		; at least one row?
		jae		ydoneq			; no, done
yloopq:
		mov		eax, pCurRow
		mov		[eax], edx		; m_CurRow = y
		mov		eax, pCanceled
		mov		al, [eax]
		test	al, al			; m_Canceled?
		jne		ydoneq			; yes, abort
		MOVAPD	xmm4, a01		; xmm4.H = a + da, xmm4.L = a
		MOVAPD	xmm12, a23		; xmm12.H = a + da * 3, xmm12.L = a + da * 2
		mov		esi, Width		; rsi = x = width in pixels
xloopq:
		mov		rcx, r11		; rcx = iteration count
		
		MOVAPD	xmm0, xmm4		; xmm0 = zx = a
		MOVAPD	xmm8, xmm12		; xmm8 = zx = a
		
		XORPD	xmm5, xmm5		; xmm5 = 0
		XORPD	xmm13, xmm13	; xmm13 = 0
		
		MOVAPD	xmm1, xmm6		; xmm1 = zy = b
		MOVAPD	xmm9, xmm6		; xmm1 = zy = b
iloopq:
		MOVAPD	xmm2, xmm0		; xmm2 = zx
		MOVAPD	xmm10, xmm8		; xmm10 = zx
		
		MULPD	xmm0, xmm0		; xmm0 = zx ^ 2
		MULPD	xmm8, xmm8		; xmm8 = zx ^ 2
		
		MOVAPD	xmm3, xmm1		; xmm3 = zy
		MOVAPD	xmm11, xmm9		; xmm11 = zy
		
		ADDPD	xmm1, xmm1		; xmm1 = 2 * zy
		ADDPD	xmm9, xmm9		; xmm9 = 2 * zy
		
		MULPD	xmm1, xmm2		; xmm1 = 2 * zx * zy
		MULPD	xmm9, xmm10		; xmm9 = 2 * zx * zy
		
		MOVAPD	xmm2, xmm0		; xmm2 = zx ^ 2
		MOVAPD	xmm10, xmm8		; xmm10 = zx ^ 2

		MULPD	xmm3, xmm3		; xmm3 = zy ^ 2
		MULPD	xmm11, xmm11	; xmm11 = zy ^ 2

		SUBPD	xmm1, xmm6		; xmm1 = 2 * zx * zy - b
		SUBPD	xmm9, xmm6		; xmm9 = 2 * zx * zy - b

		ADDPD	xmm2, xmm3		; xmm2 = zx ^ 2 + zy ^ 2
		ADDPD	xmm10, xmm11	; xmm10 = zx ^ 2 + zy ^ 2

		SUBPD	xmm3, xmm0		; xmm3 = zy ^ 2 - zx ^ 2
		SUBPD	xmm11, xmm8		; xmm11 = zy ^ 2 - zx ^ 2

		CMPLTPD	xmm2, xmm7		; xmm2 = xmm2 < 4.0 ? 0xffffffff : 0
		CMPLTPD	xmm10, xmm7		; xmm10 = xmm10 < 4.0 ? 0xffffffff : 0

		SUBPD	xmm3, xmm4		; xmm3 = zy ^ 2 - zx ^ 2 - a
		SUBPD	xmm11, xmm12	; xmm11 = zy ^ 2 - zx ^ 2 - a
		
		MOVAPD	xmm0, xmm3		; xmm0 = next zx
		MOVAPD	xmm8, xmm11		; xmm11 = next zx

		MOVMSKPD rax, xmm2		; rax = xmm2 sign mask
		MOVMSKPD r12, xmm10		; rax = xmm10 sign mask

		or		rax, r12		; combine sign masks
		jz		idoneq			; yes, early out
		
		ANDPD	xmm2, xmm7		; xmm2 = not escaped yet ? 4.0 : 0.0
		ANDPD	xmm10, xmm7		; xmm10 = not escaped yet ? 4.0 : 0.0
		
		ADDPD	xmm5, xmm2		; xmm5 += xmm2
		ADDPD	xmm13, xmm10	; xmm13 += xmm10
		
		sub		rcx, 1			; one iteration done, count > 0?
		jnz		iloopq			; yes, next iteration
idoneq:
		CVTPD2DQ	xmm0, xmm5	; truncate radius sum to integer
		CVTPD2DQ	xmm8, xmm13	; truncate radius sum to integer
		
		PSRLW	xmm0, 2			; radius sum / 4 = iteration count
		PSRLW	xmm8, 2			; radius sum / 4 = iteration count
		
		movq	[rdi], xmm0		; pFrame[0] = iteration count
		movq	[rdi + 8], xmm8	; pFrame[2] = iteration count
		
		ADDPD	xmm4, xmm14		; a += da
		ADDPD	xmm12, xmm14	; a += da
		
		add		rdi, 16			; pFrame += 4
		sub		rsi, 4			; four pixels done, x > 0?
		ja		xloopq			; yes, next set of four pixels
		ADDPD	xmm6, xmm15		; b += db
		add		rdi, r10		; pFrame += RowDelta
		add		edx, dy			; y += dy
		cmp		edx, r9d		; one row done, y < y2?
		jb		yloopq			; yes, next row
ydoneq:
		movdqa	xmm6, SaveXMM6
		movdqa	xmm7, SaveXMM7
		movdqa	xmm8, SaveXMM8
		movdqa	xmm9, SaveXMM9
		movdqa	xmm10, SaveXMM10
		movdqa	xmm11, SaveXMM11
		movdqa	xmm12, SaveXMM12
		movdqa	xmm13, SaveXMM13
		movdqa	xmm14, SaveXMM14
		movdqa	xmm15, SaveXMM15
		mov		rsi, SaveSI
		mov		rdi, SaveDI
		mov		r12, SaveR12
		add		rsp, StackSpace
		pop		rbp
		ret
	
endproc_frame
