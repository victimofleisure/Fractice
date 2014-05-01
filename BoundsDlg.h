// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version

        bounding rectangle dialog
 
*/

#if !defined(AFX_BOUNDSDLG_H__224429D8_C0B4_4FE2_9EFE_AB09A3724135__INCLUDED_)
#define AFX_BOUNDSDLG_H__224429D8_C0B4_4FE2_9EFE_AB09A3724135__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BoundsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBoundsDlg dialog

#include "PersistDlg.h"
#include "CtrlResize.h"

class CBoundsDlg : public CPersistDlg
{
	DECLARE_DYNAMIC(CBoundsDlg);
// Construction
public:
	CBoundsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CBoundsDlg)
	enum { IDD = IDD_BOUNDS };
	CString	m_x1;
	CString	m_x2;
	CString	m_y1;
	CString	m_y2;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBoundsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Generated message map functions
	//{{AFX_MSG(CBoundsDlg)
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

#endif // !defined(AFX_BOUNDSDLG_H__224429D8_C0B4_4FE2_9EFE_AB09A3724135__INCLUDED_)
