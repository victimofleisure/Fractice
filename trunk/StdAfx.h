// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version

        pre-compiled headers
 
*/

// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__E2E88B04_84A3_4197_A811_AC831CF06CE7__INCLUDED_)
#define AFX_STDAFX_H__E2E88B04_84A3_4197_A811_AC831CF06CE7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#define _WIN32_WINNT 0x0500	// for GDI functions SetDCPenColor, SetDCBrushColor
#define WINVER	0x0500		// for monitor API; may cause NT 5.0 beta warning

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include "sizecbar.h"		// resizeable control bar
#include "scbarg.h"			// resizeable control bar with gripper

#include "Globals.h"		// global definitions and inlines

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__E2E88B04_84A3_4197_A811_AC831CF06CE7__INCLUDED_)
