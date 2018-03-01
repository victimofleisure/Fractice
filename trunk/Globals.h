// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00		14feb08	initial version
		01		04mar09	add trunc and MIDI setup
		02		06jan10	W64: in round/trunc, make inline assembler 32-bit only
		03		06jan10	W64: add macros for casting to 32-bit

		global definitions and inlines

*/

#pragma once

#pragma warning(disable : 4100)	// unreferenced formal parameter

// minimal base for non-CObject classes
#include "WObject.h"

// file extensions
#define PROJECT_EXT		_T(".frp")
#define SNAPSHOT_EXT	_T(".frs")
#define PALETTE_EXT		_T(".frc")
#define IMPORT_PAL_EXT	_T(".pal")
#define	ROW_DATA_EXT	_T(".frw")
#define ROW_INDEX_EXT	_T(".frx")
#define	BITMAP_EXT		_T(".bmp")
#define	MIDI_SETUP_EXT	_T(".txt")
#define	MIXER_EXT		_T(".frm")

// registry strings
#define REG_SETTINGS		_T("Settings")
#define REG_MIXER			_T("Settings\\Mixer")

// key status bits for GetAsyncKeyState
#define GKS_TOGGLED			0x0001
#define GKS_DOWN			0x8000

// trig macros
#define PI 3.141592653589793
#define DTR(x) (x * PI / 180)	// degrees to radians
#define RTD(x) (x * 180 / PI)	// radians to degrees

// clamp a value to a range
#define CLAMP(x, lo, hi) (min(max((x), (lo)), (hi)))

// load string from resource via temporary object
#define LDS(x) CString((LPCTSTR)x)

// optimized FPU rounding
inline int round(double x)
{
#ifdef _WIN64
	return(int(x > 0 ? x + 0.5 : x - 0.5));
#else
	int		temp;
	__asm {
		fld		x		// load real
		fistp	temp	// store integer and pop stack
	}
	return(temp);
#endif
}

// optimized FPU truncation
inline int trunc(double x)
{
#ifdef _WIN64
	return(int(x));
#else
	int		temp;
	short	cw, chop;
	__asm {
		fstcw	cw		// save control word
		mov		ax, cw
		or		ax, 0c00h	// set rounding mode to chop
		mov		chop, ax
		fldcw	chop	// load chop control word
		fld		x		// load real
		fistp	temp	// store integer and pop stack
		fldcw	cw		// restore control word
	}
	return(temp);
#endif
}

enum {	// user windows messages
	UWM_FIRST = WM_APP,
	UWM_RENDERDONE,		// wParam: frame UID, lParam: none
	UWM_RCVPACKET,		// wParam: packet pointer, lParam: cookie
	UWM_TCPCONNECT,		// wParam: connected flag, lParam: cookie
	UWM_TCPACCEPT,		// wParam: WTcp pointer, lParam: client address
	UWM_ABORTRECORD,	// wParam: error string resource ID, lParam: none
	UWM_MULTIFILESEL,	// wParam: CMultiFileDlg pointer, lParam: none
	UWM_SHUTDOWN,		// wParam: warning flag, lParam: none
	UWM_STRIPDONE,		// wParam: frame UID, lParam: thread index
	UWM_ENABLECTRLS,	// wParam: non-zero to enable, lParam: none
	UWM_ABORTRENDER,	// wParam: frame UID, lParam: error string
	UWM_DLGBARUPDATE,	// wParam: none, lParam: none
	UWM_TOOLCHANGE,		// wParam: tool index, lParam: none
	UWM_MIDIROWEDIT,	// wParam: row index, lParam: control ID
	UWM_MIDIROWSEL,		// wParam: row index, lParam: control ID
	UWM_MIDIIN,			// wParam: incoming MIDI message, lParam: none
	UWM_HANDLEDLGKEY,	// wParam: MSG pointer, lParam: none
};

// atof's generic-text wrapper is missing in MFC 6
#ifndef _tstof
#ifdef UNICODE
#define _tstof(x) _tcstod(x, NULL)
#else
#define _tstof(x) atof(x)
#endif
#endif

#if _MFC_VER < 0x0800
#define genericException generic	// generic was deprecated in .NET 2005
#endif

inline void StoreBool(CArchive& ar, bool flag)
{
#if _MFC_VER >= 0x0700
	ar << flag;
#else	// MFC 6 CArchive doesn't support bool
	BYTE	byte = flag;
	ar << byte;
#endif
}

inline void LoadBool(CArchive& ar, bool& flag)
{
#if _MFC_VER >= 0x0700
	ar >> flag;
#else	// MFC 6 CArchive doesn't support bool
	BYTE	byte;
	ar >> byte;
	flag = byte != 0;
#endif
}

#ifdef _WIN64
typedef INT_PTR W64INT;
typedef UINT_PTR W64UINT;
#define INT64TO32(x) static_cast<int>(x)
#define UINT64TO32(x) static_cast<UINT>(x)
#define GCL_HBRBACKGROUND GCLP_HBRBACKGROUND
#else
typedef int W64INT;
typedef UINT W64UINT;
#define INT64TO32(x) x
#define UINT64TO32(x) x
#endif
