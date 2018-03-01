// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		
		parameters view
 
*/

#if !defined(AFX_PARAMSVIEW_H__C29609CB_CEC1_49C4_82A2_88BCCACB7439__INCLUDED_)
#define AFX_PARAMSVIEW_H__C29609CB_CEC1_49C4_82A2_88BCCACB7439__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ParamsView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CParamsView window

#include "ParamsCtrl.h"
#include "Snapshot.h"

class CFracticeView;

class CParamsView : public CParamsCtrl
{
	DECLARE_DYNAMIC(CParamsView);
// Construction
public:
	CParamsView();

// Constants

// Attributes
public:
	void	SetView(CFracticeView *View);

// Operations
public:
	void	Update();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CParamsView)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CParamsView();

// Generated message map functions
protected:
	//{{AFX_MSG(CParamsView)
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Types

// Constants

// Data members
	CFracticeView	*m_View;	// pointer to view
	HACCEL	m_MainAccel;	// saved copy of main accelerator table

// Overrides
	void	OnBeginEdit();
	void	OnEndEdit(bool Modified);

// Helpers
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PARAMSVIEW_H__C29609CB_CEC1_49C4_82A2_88BCCACB7439__INCLUDED_)
