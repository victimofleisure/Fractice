// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      11jan09	initial version
		01		16jan09	add pause/resume

        poster dialog
 
*/

#if !defined(AFX_POSTERDLG_H__1B16FC8C_CDB8_439A_B889_F3EC7F39DAED__INCLUDED_)
#define AFX_POSTERDLG_H__1B16FC8C_CDB8_439A_B889_F3EC7F39DAED__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PosterDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPosterDlg dialog

#include "ImageSizeResDlg.h"
#include "PictureCtrl.h"
#include "Snapshot.h"

class CPosterDlg : public CImageSizeResDlg
{
	DECLARE_DYNAMIC(CPosterDlg);
// Construction
public:
	CPosterDlg(CWnd* pParent = NULL);
	~CPosterDlg();

// Attributes
	void	SetSnapshot(CSnapshot& Snap);

// Operations

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPosterDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CPosterDlg)
	enum { IDD = IDD_POSTER };
	CPictureCtrl	m_SnapPic;
	BOOL	m_QueueJob;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CPosterDlg)
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnResume();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Constants

// Data members
	UINT	m_Antialias;		// antialiasing factor
	UINT	m_PrevAntialias;	// previous antialiasing factor
	CSnapshot	m_Snap;			// snapshot to render
	WORD	m_BitCount;			// bitmap color depth, in bits per pixel

// Helpers
	bool	RenderPoster(LPCTSTR Path);
	void	QueuePoster(LPCTSTR Path);
};

inline void CPosterDlg::SetSnapshot(CSnapshot& Snap)
{
	m_Snap = Snap;
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_POSTERDLG_H__1B16FC8C_CDB8_439A_B889_F3EC7F39DAED__INCLUDED_)
