// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      13aug05	initial version
		01		17feb06	add oscillator properties
		02		10dec07	add global parameters
		03		22jan08	main now calls OnMidiIn directly
		04		23jan08	replace MIDI range scaler with start/end
		05		28jan08	support Unicode
		06		29jan08	in SetValue, add static cast to fix warning
		07		03mar09	customize for Fractice
		08		23mar09	support channel properties
		09		24mar09	add dialog key handler
		10		06jan10	W64: in GetCount, cast row array size to 32-bit

		MIDI setup dialog
 
*/

#if !defined(AFX_MIDISETUPDLG_H__88433D48_4449_41E5_8534_6B2DFE3FAA5B__INCLUDED_)
#define AFX_MIDISETUPDLG_H__88433D48_4449_41E5_8534_6B2DFE3FAA5B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MidiSetupDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMidiSetupDlg dialog

#include "RowDialog.h"
#include "MidiSetupRow.h"
#include "MidiIO.h"

enum {	// global MIDI properties
	#define MIDI_PROP(name, tag, start, end, ctrl) MP_##tag,
	#include "MidiPropsGlobal.h"
	GLOBAL_MIDI_PROPS
};

enum {	// channel MIDI properties
	CHAN_MIDI_START = GLOBAL_MIDI_PROPS - 1,
	#undef MIDI_PROP
	#define MIDI_PROP(name, tag, start, end, ctrl) MP_##tag,
	#include "MidiPropsChannel.h"
	CHAN_MIDI_END,
	CHAN_MIDI_PROPS = CHAN_MIDI_END - CHAN_MIDI_START - 1,
	MIDI_PROPS = GLOBAL_MIDI_PROPS + CHAN_MIDI_PROPS * 2,
};

class CMidiSetupDlg : public CRowDialog
{
	DECLARE_DYNAMIC(CMidiSetupDlg);
// Construction
public:
	CMidiSetupDlg(CWnd* pParent = NULL);   // standard constructor

// Constants
	enum {	// columns
		COL_TITLE,
		COL_RANGE_START,
		COL_RANGE_END,
		COL_EVENT,
		COL_CHAN,
		COL_CTRL,
		COL_VALUE,
		COLS
	};
	enum {	// event types
		ET_OFF		= CMidiSetupRow::ET_OFF,
		ET_CTRL		= CMidiSetupRow::ET_CTRL,
		ET_NOTE		= CMidiSetupRow::ET_NOTE,
		ET_PITCH	= CMidiSetupRow::ET_PITCH,
	};
	enum {
		MIDI_CHANS	= 16,
		MIDI_PARMS	= 128,
		CENTER_POS	= 64,
	};

// Types
	typedef CMidiSetupRow::INFO ROWINFO;
	typedef CMidiSetupRow::DRANGE DRANGE;
	typedef struct tagINFO {
		ROWINFO	Row[MIDI_PROPS];
	} INFO;

// Attributes
	void	GetDefaults(INFO& Info) const;
	void	GetInfo(INFO& Info) const;
	void	SetInfo(const INFO& Info);
	int		GetCount() const;
	void	GetRange(int PropIdx, DRANGE& Range) const;
	int		GetMapping(int Event, int Chan, int Ctrl) const;
	int		GetCtrlMapping(int Chan, int Ctrl) const;
	int		GetNoteMapping(int Chan, int Note) const;
	int		GetPitchMapping(int Chan) const;
	void	SetValue(int PropIdx, int Value);
	int		GetValue(int PropIdx) const;
	static	LPCTSTR	GetRowName(int PropIdx);
	static	int	GetRowTitleID(int PropIdx);
	void	SetLearn(bool Enable);
	bool	IsLearning() const;

// Operations
	void	Assign(int PropIdx, int Event, int Chan, int Ctrl);
	void	RestoreDefaults();
	void	OnMidiIn(CMidiIO::MSG msg);
	static	int	FindRow(LPCTSTR Name);
	int		CalcRowCount() const;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMidiSetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CMidiSetupDlg)
	enum { IDD = IDD_MIDI_SETUP };
	CButton	m_LearnChk;
	int		m_CurPage;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CMidiSetupDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnLearn();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnOpen();
	afx_msg void OnSave();
	afx_msg void OnPageSelect();
	afx_msg void OnReset();
	//}}AFX_MSG
	afx_msg LRESULT	OnHandleDlgKey(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMidiRowEdit(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMidiRowSel(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

// Types
	typedef struct tagMIDIPROP {	// data about each MIDI property
		LPCTSTR	Name;		// name for tagged I/O
		int		TitleID;	// title string resource ID
		DRANGE	DefRange;	// default range
		int		DefCtrl;	// default controller number
	} MIDIPROP;

// Constants
	enum {
		TOP_MARGIN		= 40,
		DEFAULT_CHAN	= 0,
		DEFAULT_CTRL	= 7,
	};
	enum {	// page types
		PT_A,
		PT_B,
		PT_GLOBAL,
		PAGE_TYPES
	};
	static const COLINFO	m_ColInfo[COLS];
	static const MIDIPROP	m_MidiProp[MIDI_PROPS];

// Member data
	BYTE	m_CtrlMap[MIDI_CHANS][MIDI_PARMS];	// map of controller assignments
	BYTE	m_NoteMap[MIDI_CHANS][MIDI_PARMS];	// map of note assignments
	BYTE	m_PitchMap[MIDI_CHANS];		// map of pitch bend assignments
	BYTE	m_Value[MIDI_PROPS];		// shadow of MIDI value
	INFO	m_Info;						// information about each property
	int		m_RowSel;		// current row selection, or -1 if none
	bool	m_ShowRowSel;	// true if row selection is visible
	bool	m_Learn;		// true if we're in learn mode

// Overrides
	CWnd	*CreateRow(int Idx, int& Pos);

// Helpers
	CMidiSetupRow	*GetRow(int Idx) const;
	void	MakeMidiMap();
	void	SelectRow(int RowIdx);
	void	ShowRowSel(bool Enable);
	int		RowToProp(int RowIdx);
	int		PropToRow(int PropIdx);
	void	UpdateRows();
};

inline int CMidiSetupDlg::GetCount() const
{
	return(INT64TO32(m_Row.GetSize()));
}

inline CMidiSetupRow *CMidiSetupDlg::GetRow(int Idx) const
{
	return((CMidiSetupRow *)m_Row[Idx]);
}

inline void CMidiSetupDlg::GetRange(int PropIdx, DRANGE& Range) const
{
	Range = m_Info.Row[PropIdx].Range;
}

inline int CMidiSetupDlg::GetCtrlMapping(int Chan, int Ctrl) const
{
	ASSERT(Chan >= 0 && Chan < MIDI_CHANS);
	ASSERT(Ctrl >= 0 && Ctrl < MIDI_PARMS);
	return(int(m_CtrlMap[Chan][Ctrl]) - 1);
}

inline int CMidiSetupDlg::GetNoteMapping(int Chan, int Note) const
{
	ASSERT(Chan >= 0 && Chan < MIDI_CHANS);
	ASSERT(Note >= 0 && Note < MIDI_PARMS);
	return(int(m_NoteMap[Chan][Note]) - 1);
}

inline int CMidiSetupDlg::GetPitchMapping(int Chan) const
{
	ASSERT(Chan >= 0 && Chan < MIDI_CHANS);
	return(int(m_PitchMap[Chan]) - 1);
}

inline int CMidiSetupDlg::GetMapping(int Event, int Chan, int Ctrl) const
{
	switch (Event) {
	case ET_CTRL:
		return(GetCtrlMapping(Chan, Ctrl));
	case ET_NOTE:
		return(GetNoteMapping(Chan, Ctrl));
	case ET_PITCH:
		return(GetPitchMapping(Chan));
	default:
		return(-1);
	}
}

inline void CMidiSetupDlg::SetValue(int PropIdx, int Value)
{
	ASSERT(PropIdx >= 0 && PropIdx <= MIDI_PROPS);
	m_Value[PropIdx] = static_cast<BYTE>(Value);
}

inline int CMidiSetupDlg::GetValue(int PropIdx) const
{
	ASSERT(PropIdx >= 0 && PropIdx <= MIDI_PROPS);
	return(m_Value[PropIdx]);
}

inline LPCTSTR CMidiSetupDlg::GetRowName(int PropIdx)
{
	ASSERT(PropIdx >= 0 && PropIdx <= MIDI_PROPS);
	return(m_MidiProp[PropIdx].Name);
}

inline int CMidiSetupDlg::GetRowTitleID(int PropIdx)
{
	ASSERT(PropIdx >= 0 && PropIdx < MIDI_PROPS);
	return(m_MidiProp[PropIdx].TitleID);
}

inline bool CMidiSetupDlg::IsLearning() const
{
	return(m_Learn);
}

inline int CMidiSetupDlg::CalcRowCount() const
{
	return(m_CurPage < PT_GLOBAL ? CHAN_MIDI_PROPS : GLOBAL_MIDI_PROPS);
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MIDISETUPDLG_H__88433D48_4449_41E5_8534_6B2DFE3FAA5B__INCLUDED_)
