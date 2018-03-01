// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
        
		static control that displays a picture
 
*/

#if !defined(AFX_PICTURECTRL_H__DF828822_03A9_4A83_8AAD_0DB4A0FE00F6__INCLUDED_)
#define AFX_PICTURECTRL_H__DF828822_03A9_4A83_8AAD_0DB4A0FE00F6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PictureCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPictureCtrl window

class CPictureCtrl : public CStatic
{
	DECLARE_DYNAMIC(CPictureCtrl);
// Construction
public:
	CPictureCtrl();

// Attributes
public:
	void	SetBitmap(HBITMAP Bitmap, const CRect *Detail);
	void	SetAspectRatio(double Aspect);

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPictureCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPictureCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CPictureCtrl)
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Member data
	HBITMAP	m_Bitmap;
	CRect	m_Detail;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PICTURECTRL_H__DF828822_03A9_4A83_8AAD_0DB4A0FE00F6__INCLUDED_)
