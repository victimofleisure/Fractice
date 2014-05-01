// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      28feb09	initial version
        01      09mar09	add channel array

        motion mixer page
 
*/

#if !defined(AFX_MIXMOTION_H__51F7FA3A_F298_4A31_9EF4_DF86C4FDDE88__INCLUDED_)
#define AFX_MIXMOTION_H__51F7FA3A_F298_4A31_9EF4_DF86C4FDDE88__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MixMotionDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMixMotionDlg dialog

#include "MixerInfo.h"
#include "PersistDlg.h"
#include "NumEdit.h"
#include "ClickSliderCtrl.h"
#include "CtrlResize.h"

class CFracticeView;

class CMixMotionDlg : public CPropertyPage
{
// Construction
public:
	CMixMotionDlg(CMixerInfo& Info);
	~CMixMotionDlg();

// Attributes

// Operations

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMixMotionDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CMixMotionDlg)
	enum { IDD = IDD_MIX_MOTION };
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CMixMotionDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnMixMirrorChk();
	afx_msg void OnChanSelect();
	//}}AFX_MSG
	afx_msg void OnSelchangeOrgMotionCombo();
	afx_msg LRESULT OnQuerySiblings(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

// Constants
	static const int	m_OrgMotionPageID[];

// Data members
	CMixerInfo&	m_mi;			// mixer info reference
	CDialog	*m_OMPage;			// pointer to current origin motion page
	int		m_OMType;			// current origin motion type, or -1 if none

// Helpers
	void	UpdateCtrls(int CtrlID);
	void	CreateOrgMotionPage(int OMType);
	void	DestroyOrgMotionPage();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MIXMOTION_H__51F7FA3A_F298_4A31_9EF4_DF86C4FDDE88__INCLUDED_)
