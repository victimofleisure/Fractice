// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version

        image size dialog
 
*/

#if !defined(AFX_IMAGESIZEDLG_H__61B7C353_162B_4964_BB9B_30384E60D9CB__INCLUDED_)
#define AFX_IMAGESIZEDLG_H__61B7C353_162B_4964_BB9B_30384E60D9CB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ImageSizeDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CImageSizeDlg dialog

#include "NumEdit.h"

class CImageSizeDlg : public CDialog
{
	DECLARE_DYNAMIC(CImageSizeDlg);
// Construction
public:
	CImageSizeDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CImageSizeDlg)
	enum { IDD = IDD_IMAGE_SIZE };
	//}}AFX_DATA
	int		m_Width;
	int		m_Height;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImageSizeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

// Generated message map functions
	//{{AFX_MSG(CImageSizeDlg)
	afx_msg void OnSelchangeFrameSize();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Member data
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMAGESIZEDLG_H__61B7C353_162B_4964_BB9B_30384E60D9CB__INCLUDED_)
