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

#if !defined(AFX_STDAFX_H__62BD213C_F89A_42EF_9608_B6B12B1F0D9E__INCLUDED_)
#define AFX_STDAFX_H__62BD213C_F89A_42EF_9608_B6B12B1F0D9E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afx.h>
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include "Globals.h"		// global definitions and inlines

// for console app, replace PostMessage with PostThreadMessage; hWnd is thread ID
#undef PostMessage
#define PostMessage(hWnd, Msg, wParam, lParam) PostThreadMessage((DWORD)hWnd, Msg, wParam, lParam)

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__62BD213C_F89A_42EF_9608_B6B12B1F0D9E__INCLUDED_)
