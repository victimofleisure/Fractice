// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      17dec09	initial version

        select images dialog
 
*/

#if !defined(AFX_SELECTIMAGESDLG_H__8E708961_BBE9_460C_9FF8_47B33CF7F32C__INCLUDED_)
#define AFX_SELECTIMAGESDLG_H__8E708961_BBE9_460C_9FF8_47B33CF7F32C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SelectImagesDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSelectImagesDlg dialog

#include "PersistDlg.h"
#include "CtrlResize.h"

class CSelectImagesDlg : public CPersistDlg
{
	DECLARE_DYNAMIC(CSelectImagesDlg);
// Construction
public:
	CSelectImagesDlg(CImageList& ImgList, LPCTSTR Caption = NULL, LPCTSTR RegKey = NULL, CWnd* pParent = NULL);

// Attributes
	void	GetSelection(CDWordArray& Selection) const;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectImagesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CSelectImagesDlg)
	enum { IDD = IDD_SELECT_IMAGES };
	CListCtrl	m_List;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CSelectImagesDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnGetdispinfoList(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Constants
	static const CCtrlResize::CTRL_LIST	m_CtrlList[];

// Member data
	CImageList&	m_ImgList;		// reference to caller's image list
	LPCTSTR	m_Caption;			// custom dialog caption if any
	CRect	m_InitRect;			// initial rectangle in screen coords
	CCtrlResize	m_Resize;		// control resizer
	CDWordArray	m_Selection;	// array of selection indices
};

inline void CSelectImagesDlg::GetSelection(CDWordArray& Selection) const
{
	Selection.Copy(m_Selection);
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTIMAGESDLG_H__8E708961_BBE9_460C_9FF8_47B33CF7F32C__INCLUDED_)
