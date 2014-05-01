// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		14mar09	add get/set options
		02		01jul09	add image list member

		parameters control
 
*/

#if !defined(AFX_PARAMSCTRL_H__C29609CB_CEC1_49C4_82A2_88BCCACB7439__INCLUDED_)
#define AFX_PARAMSCTRL_H__C29609CB_CEC1_49C4_82A2_88BCCACB7439__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ParamsCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CParamsCtrl window

#include "GridCtrl.h"
#include "Snapshot.h"

class CFracticeView;

class CParamsCtrl : public CGridCtrl
{
	DECLARE_DYNAMIC(CParamsCtrl);
// Construction
public:
	CParamsCtrl();

// Constants
	enum {	// columns
		COL_PARAM,
		COL_VALUE,
		COLUMNS
	};
	enum {	// option flags
		OP_ALLOW_EMPTY	= 0x01
	};

// Attributes
public:
	void	GetSnapshot(CSnapshot& Snap) const;
	void	SetSnapshot(const CSnapshot& Snap);
	UINT	GetOptions() const;
	void	SetOptions(UINT Flags);
	bool	IsEmpty() const;
	void	SetEmpty();

// Operations
public:
	void	Init();
	void	Update();
	static	int		FindComboString(int Row, LPCTSTR Text);
	static	CString	GetComboString(int Row, int SelIdx);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CParamsCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CParamsCtrl();

// Generated message map functions
protected:
	//{{AFX_MSG(CParamsCtrl)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Types
	typedef struct tagCOL_INFO {	// list column info
		int		Title;		// string resource ID of title
		int		Align;		// alignment
		int		Width;		// initial width
	} COL_INFO;
	typedef struct tagCOMBO_INFO {	// combo box info
		const int	*ItemID;	// pointer to list of item string IDs
		UINT	Style;		// style bitmask, ORed with default style
	} COMBO_INFO;
	typedef struct tagPARM_INFO {	// parameter info
		int		NameID;		// string ID of parameter name
		int		Type;		// data type; see FormatIO enum
		int		Offset;		// offset of value within snapshot
		int		MinVal;		// minimum value
		int		MaxVal;		// maximum value
		WORD	UndoCode;	// view undo code for SaveState
		const	COMBO_INFO	*ComboInfo;	// pointer to combo box info
	} PARM_INFO;

// Constants
	enum {
		ROW_HEIGHT = 16
	};
	static const COL_INFO	m_ColInfo[COLUMNS];
	static const PARM_INFO	m_ParmInfo[NUM_PARMS];
	static const int		m_DeepZoomComboStr[];
	static const COMBO_INFO	m_DeepZoomComboInfo;
	static const int		m_FractalTypeComboStr[];
	static const COMBO_INFO	m_FractalTypeComboInfo;

// Data members
	CSnapshot	m_Snap;			// current snapshot
	WORD	m_UndoCode;			// undo code of most recent edit
	int		m_ModCount;			// number of modifications made
	int		m_LastModIdx;		// index of last item modified
	UINT	m_Options;			// option flags; see enum above
	CImageList	m_ImgList;		// image list for setting row height

// Overrides
	void	OnBeginEdit();
	bool	OnItemChange(int Row, int Col, LPCTSTR Text);
	bool	GetComboStrings(int Row, CStringArray& ComboStr, UINT& Style);

// Helpers
};

inline void CParamsCtrl::GetSnapshot(CSnapshot& Snap) const
{
	Snap = m_Snap;
}

inline UINT CParamsCtrl::GetOptions() const
{
	return(m_Options);
}

inline void CParamsCtrl::SetOptions(UINT Options)
{
	m_Options = Options;
}

inline bool CParamsCtrl::IsEmpty() const
{
	return(m_Snap.IsEmpty());
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PARAMSCTRL_H__C29609CB_CEC1_49C4_82A2_88BCCACB7439__INCLUDED_)
