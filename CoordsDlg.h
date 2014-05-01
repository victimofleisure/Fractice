// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version

        viewer coordinates dialog
 
*/

#if !defined(AFX_COORDSDLG_H__AA6D635A_86DE_4B66_9555_E89B14D1A198__INCLUDED_)
#define AFX_COORDSDLG_H__AA6D635A_86DE_4B66_9555_E89B14D1A198__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CoordsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCoordsDlg dialog

#include "PersistDlg.h"
#include "CtrlResize.h"

class CCoordsDlg : public CPersistDlg
{
	DECLARE_DYNAMIC(CCoordsDlg);
// Construction
public:
	CCoordsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCoordsDlg)
	enum { IDD = IDD_COORDS };
	CString	m_x;
	CString	m_y;
	CString	m_z;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCoordsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Generated message map functions
	//{{AFX_MSG(CCoordsDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Constants
	static const CCtrlResize::CTRL_LIST	m_CtrlList[];
	
// Data members
	CCtrlResize	m_Resize;		// control resizer
	CRect	m_InitRect;			// initial rectangle in screen coords
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COORDSDLG_H__AA6D635A_86DE_4B66_9555_E89B14D1A198__INCLUDED_)
