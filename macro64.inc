; Copyleft 2010 Chris Korda
; This program is free software; you can redistribute it and/or modify it
; under the terms of the GNU General Public License as published by the Free
; Software Foundation; either version 2 of the License, or any later version.
;
;       chris korda
;
;		revision history:
;		rev		date	comments
;       00      01feb10	initial version
;
;		64-bit assembler macros
;

; argdef(0) = first argument's shadow space
; argdef(1) = second argument's shadow space
; argdef(2) = third argument's shadow space
; argdef(3) = fourth argument's shadow space
; argdef(4) = fifth argument if any
; argdef(5) = sixth argument if any
; etc.
;
%define	argdef(x) [rbp + (StackSpace - FrameBias) + 16 + (8 * x)]

; vardef(0) = first saved non-volatile register or local variable
; vardef(1) = second saved non-volatile register or local variable
; etc.
;
%define	vardef(x) [rbp + (StackSpace - FrameBias) - 8 - (8 * x)]

%define	FrameBias	0x20	; typically optimal, but functions may redefine it
