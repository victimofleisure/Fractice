// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version

        levels control
 
*/

#if !defined(AFX_LEVELSCTRL_H__25E0B638_6817_47F1_9218_66D9AE1D28E3__INCLUDED_)
#define AFX_LEVELSCTRL_H__25E0B638_6817_47F1_9218_66D9AE1D28E3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LevelsCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLevelsCtrl window

class CFracticeView;

class CLevelsCtrl : public CStatic
{
	DECLARE_DYNAMIC(CLevelsCtrl);
// Construction
public:
	CLevelsCtrl();

// Constants
	enum {	// scaling types
		ST_LINEAR,
		ST_LOG10
	};

// Attributes
public:
	void	SetView(CFracticeView *View);
	int		GetScalingType() const;
	void	SetScalingType(int Type);
	bool	IsAxisShowing() const;
	void	ShowAxis(bool Enable);

// Operations
public:
	void	CalcLevels();
	void	SetEmpty();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLevelsCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CLevelsCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CLevelsCtrl)
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Member data
	CFracticeView	*m_View;	// pointer to view
	CDWordArray	m_Sample;	// array of samples
	UINT	m_MaxSamp;		// maximum sample
	int		m_ScalingType;	// scaling type; see enum above
	bool	m_ShowAxis;		// if true, show axis

// Helpers
	static	UINT	CalcAxisStep(int Samples, int AxisLen, int MinSpacing);
};

inline int CLevelsCtrl::GetScalingType() const
{
	return(m_ScalingType);
}

inline bool CLevelsCtrl::IsAxisShowing() const
{
	return(m_ShowAxis);
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LEVELSCTRL_H__25E0B638_6817_47F1_9218_66D9AE1D28E3__INCLUDED_)
