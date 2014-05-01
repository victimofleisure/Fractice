// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      13aug05	initial version
        00      16aug05	add learn mode, shadow values
		01		21aug05	in zero controllers, set values to 64
		02		04sep05	add miscellaneous properties
		03		04oct05	add support for notes
		04		17oct05	in Assign, reset prev mapping's event only
		05		17feb06	add oscillator properties
		06		11oct06	remove OnClose remnant from message map
		07		10dec07	add global parameters
		08		21dec07	replace AfxGetMainWnd with GetThis 
		09		22jan08	add special caption for global rotation
		10		23jan08	replace MIDI range scaler with start/end
		11		28jan08	support Unicode
		12		29jan08	in MakeMidiMap, add static cast to fix warning
		13		30jan08	use main keyboard accelerators
		14		31mar08	redo default MIDI controller scheme
		15		03mar09	customize for Fractice
		16		23mar09	support channel properties
		17		24mar09	add dialog key handler
		18		06jan10	W64: cast row WPARAM to 32-bit

		MIDI setup dialog
 
*/

// MidiSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Fractice.h"
#include "MainFrm.h"
#include "MidiInfo.h"	// for open/save
#include "MidiSetupDlg.h"
#include "MidiSetupRow.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMidiSetupDlg dialog

IMPLEMENT_DYNAMIC(CMidiSetupDlg, CRowDialog);

const CRowDialog::COLINFO CMidiSetupDlg::m_ColInfo[COLS] = {
	{IDC_MS_TITLE,			0},
	{IDC_MS_RANGE_START,	IDS_MS_RANGE},
	{IDC_MS_RANGE_START,	IDS_MS_RANGE},
	{IDC_MS_EVENT,			IDS_MS_EVENT},
	{IDC_MS_CHAN,			IDS_MS_CHAN},
	{IDC_MS_CTRL,			IDS_MS_CTRL},
	{IDC_MS_VALUE,			IDS_MS_VALUE}
};

const CMidiSetupDlg::MIDIPROP CMidiSetupDlg::m_MidiProp[MIDI_PROPS] = {
#undef MIDI_PROP
#define MIDI_PROP(name, tag, start, end, ctrl) {_T(#name), IDS_MP_##tag, {start, end}, ctrl},
#include "MidiPropsGlobal.h"
#undef MIDI_PROP
#define MIDI_PROP(name, tag, start, end, ctrl) {_T(#name)_T("0"), IDS_MP_##tag, {start, end}, ctrl},
#include "MidiPropsChannel.h"
#undef MIDI_PROP
#define MIDI_PROP(name, tag, start, end, ctrl) {_T(#name)_T("1"), IDS_MP_##tag, {start, end}, ctrl},
#include "MidiPropsChannel.h"
};

CMidiSetupDlg::CMidiSetupDlg(CWnd* pParent /*=NULL*/)
	: CRowDialog(IDD, IDR_MAINFRAME, _T("MidiSetupDlg"), pParent)
{
	//{{AFX_DATA_INIT(CMidiSetupDlg)
	m_CurPage = 0;
	//}}AFX_DATA_INIT
	ZeroMemory(m_CtrlMap, sizeof(m_CtrlMap));
	ZeroMemory(m_NoteMap, sizeof(m_NoteMap));
	ZeroMemory(m_PitchMap, sizeof(m_PitchMap));
	memset(m_Value, CENTER_POS, sizeof(m_Value));	// center all control shadows
	ZeroMemory(&m_Info, sizeof(INFO));
	m_RowSel = -1;			// no row selection
	m_ShowRowSel = FALSE;
	m_Learn = FALSE;
}

int CMidiSetupDlg::RowToProp(int RowIdx)
{
	if (m_CurPage < PT_GLOBAL) {	// if channel page
		ASSERT(RowIdx >= 0 && RowIdx < CHAN_MIDI_PROPS);
		return(RowIdx + m_CurPage * CHAN_MIDI_PROPS + GLOBAL_MIDI_PROPS);
	}
	// global page
	ASSERT(RowIdx >= 0 && RowIdx < GLOBAL_MIDI_PROPS);
	return(RowIdx);
}

int CMidiSetupDlg::PropToRow(int PropIdx)
{
	ASSERT(PropIdx >= 0 && PropIdx < MIDI_PROPS);
	if (m_CurPage < PT_GLOBAL) {	// if channel page
		int	RowIdx = PropIdx - m_CurPage * CHAN_MIDI_PROPS - GLOBAL_MIDI_PROPS;
		if (RowIdx >= 0 && RowIdx < CHAN_MIDI_PROPS)
			return(RowIdx);
		return(-1);	// inaccessible property
	}
	// global page
	if (PropIdx >= 0 && PropIdx < GLOBAL_MIDI_PROPS)
		return(PropIdx);
	return(-1);	// inaccessible property
}

CWnd *CMidiSetupDlg::CreateRow(int Idx, int& Pos)
{
	CMidiSetupRow	*rp = new CMidiSetupRow;
	rp->Create(IDD_MIDI_SETUP_ROW);
	int	PropIdx = RowToProp(Pos);
	rp->SetInfo(m_Info.Row[PropIdx]);
	CString	s((LPCTSTR)m_MidiProp[PropIdx].TitleID);
	rp->SetCaption(s + ':');
	rp->SetValue(m_Value[PropIdx] - CENTER_POS);
	return(rp);	// return address of created row object to base class
}

int	CMidiSetupDlg::FindRow(LPCTSTR Name)
{
	for (int i = 0; i < MIDI_PROPS; i++) {
		if (!_tcscmp(Name, m_MidiProp[i].Name))
			return(i);
	}
	return(-1);
}

void CMidiSetupDlg::GetDefaults(INFO& Info) const
{
	ZeroMemory(&Info, sizeof(INFO));
	int	ctrl = CHAN_MIDI_PROPS * 2;
	for (int i = 0; i < MIDI_PROPS; i++) {
		ROWINFO&	ri = Info.Row[i];
		ri.Range.End = 1;
		ri.Event = ET_CTRL;
		ri.Ctrl = ++ctrl;
		if (ctrl >= MIDI_PROPS)
			ctrl = 0;
	}
}

void CMidiSetupDlg::GetInfo(INFO& Info) const
{
	Info = m_Info;
}

void CMidiSetupDlg::UpdateRows()
{
	int	rows = GetCount();
	int	PropIdx = RowToProp(0);	// assume page has contiguous properties
	for (int i = 0; i < rows; i++) {	// update all row dialogs
		CMidiSetupRow	*rp = GetRow(i);
		rp->SetInfo(m_Info.Row[PropIdx]);
		rp->SetValue(m_Value[PropIdx] - CENTER_POS);
		PropIdx++;
	}
}

void CMidiSetupDlg::SetInfo(const INFO& Info)
{
	m_Info = Info;
	SetLearn(FALSE);	// disable learn mode
	UpdateRows();
	MakeMidiMap();
}

void CMidiSetupDlg::RestoreDefaults()
{
	INFO	Info;
	GetDefaults(Info);
	SetInfo(Info);
}

void CMidiSetupDlg::SetLearn(bool Enable)
{
	m_Learn = Enable;
	m_LearnChk.SetCheck(Enable);
	ShowRowSel(Enable);
}

void CMidiSetupDlg::Assign(int PropIdx, int Event, int Chan, int Ctrl)
{
	ASSERT(PropIdx >= 0 && PropIdx < MIDI_PROPS);
	if (Event != ET_OFF) {
		int	PrevMap = GetMapping(Event, Chan, Ctrl);
		if (PrevMap >= 0 && PrevMap != PropIdx) {	// if already mapped to a different property
			ROWINFO&	pmri = m_Info.Row[PrevMap];	// previous mapping's row info
			pmri.Event = ET_OFF;	// remove previous mapping
			int	RowIdx = PropToRow(PrevMap);
			if (RowIdx >= 0)	// if property has a row dialog, update it
				GetRow(RowIdx)->Assign(pmri.Event, pmri.Chan, pmri.Ctrl);
		}
	}
	ROWINFO&	ri = m_Info.Row[PropIdx];
	ri.Event = Event;	// create new mapping
	ri.Chan = Chan;
	ri.Ctrl = Ctrl;
	int	RowIdx = PropToRow(PropIdx);
	if (RowIdx >= 0)	// if property has a row dialog, update it
		GetRow(RowIdx)->Assign(Event, Chan, Ctrl);
	MakeMidiMap();
}

void CMidiSetupDlg::MakeMidiMap()
{
	ZeroMemory(m_CtrlMap, sizeof(m_CtrlMap));
	ZeroMemory(m_NoteMap, sizeof(m_NoteMap));
	ZeroMemory(m_PitchMap, sizeof(m_PitchMap));
	for (int i = 0; i < MIDI_PROPS; i++) {
		ROWINFO&	ri = m_Info.Row[i];
		ASSERT(ri.Chan >= 0 && ri.Chan < MIDI_CHANS);
		ASSERT(ri.Ctrl >= 0 && ri.Ctrl < MIDI_PARMS);
		BYTE	targ = static_cast<BYTE>(i + 1);
		switch (ri.Event) {
		case ET_CTRL:
			m_CtrlMap[ri.Chan][ri.Ctrl] = targ;
			break;
		case ET_NOTE:
			m_NoteMap[ri.Chan][ri.Ctrl] = targ;
			break;
		case ET_PITCH:
			m_PitchMap[ri.Chan] = targ;
			break;
		}
	}
}

void CMidiSetupDlg::SelectRow(int RowIdx)
{
	if (RowIdx != m_RowSel) {
		if (m_ShowRowSel) {
			if (m_RowSel >= 0)
				GetRow(m_RowSel)->SetSelected(FALSE);	// remove previous selection
			if (RowIdx >= 0)
				GetRow(RowIdx)->SetSelected(TRUE);
		}
		m_RowSel = RowIdx;
	}
}

void CMidiSetupDlg::ShowRowSel(bool Enable)
{
	if (Enable != m_ShowRowSel) {
		if (Enable) {
			m_ShowRowSel = Enable;	// order matters
			int	row = m_RowSel;	// save selection
			m_RowSel = -1;	// force SelectRow to update
			SelectRow(row);	// show selection
		} else {
			SelectRow(-1);	// hide selection
			m_ShowRowSel = Enable;	// order matters
		}
	}
}

void CMidiSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CRowDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMidiSetupDlg)
	DDX_Control(pDX, IDC_MS_LEARN, m_LearnChk);
	DDX_Radio(pDX, IDC_MS_PAGE_SELECT, m_CurPage);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMidiSetupDlg, CRowDialog)
	//{{AFX_MSG_MAP(CMidiSetupDlg)
	ON_BN_CLICKED(IDC_MS_LEARN, OnLearn)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_MS_OPEN, OnOpen)
	ON_BN_CLICKED(IDC_MS_SAVE, OnSave)
	ON_BN_CLICKED(IDC_MS_PAGE_SELECT, OnPageSelect)
	ON_BN_CLICKED(IDC_MS_PAGE_SELECT2, OnPageSelect)
	ON_BN_CLICKED(IDC_MS_PAGE_SELECT3, OnPageSelect)
	ON_BN_CLICKED(IDC_MS_RESET, OnReset)
	//}}AFX_MSG_MAP
	ON_MESSAGE(UWM_MIDIROWEDIT, OnMidiRowEdit)
	ON_MESSAGE(UWM_MIDIROWSEL, OnMidiRowSel)
	ON_MESSAGE(UWM_HANDLEDLGKEY, OnHandleDlgKey)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMidiSetupDlg message handlers

BOOL CMidiSetupDlg::OnInitDialog() 
{
	SetIcon(theApp.LoadIcon(IDR_MAINFRAME), FALSE);
	CRowDialog::OnInitDialog();
	RestoreDefaults();

	return FALSE;	// CreateRows sets focus to a control
}

LRESULT	CMidiSetupDlg::OnHandleDlgKey(WPARAM wParam, LPARAM lParam)
{
	return(theApp.HandleDlgKeyMsg((MSG *)wParam));
}

LRESULT CMidiSetupDlg::OnMidiRowEdit(WPARAM wParam, LPARAM lParam)
{
	int	Row = INT64TO32(wParam);
	int	PropIdx = RowToProp(Row);
	switch (lParam) {
	case IDC_MS_EVENT:
	case IDC_MS_CHAN:
	case IDC_MS_CTRL:
		{
			ROWINFO	Info;
			GetRow(Row)->GetInfo(Info);
			Assign(PropIdx, Info.Event, Info.Chan, Info.Ctrl);
		}
		break;
	case IDC_MS_RANGE_START:
	case IDC_MS_RANGE_END:
		GetRow(Row)->GetRange(m_Info.Row[PropIdx].Range);
		break;
	}
	return(TRUE);
}

LRESULT CMidiSetupDlg::OnMidiRowSel(WPARAM wParam, LPARAM lParam)
{
	SelectRow(INT64TO32(wParam));
	return TRUE;
}

void CMidiSetupDlg::OnMidiIn(CMidiIO::MSG msg)
{
	static const int CmdToEvent[] = {
		0, 0, 0, 0, 0, 0, 0, 0,	// unused
		-1,			// note off
		ET_NOTE,	// note on
		-1,			// key aftertouch
		ET_CTRL,	// control change
		-1,			// program change
		-1,			// channel aftertouch
		ET_PITCH,	// pitch bend
		-1,			// system
	};
	int	event = CmdToEvent[(msg.s.cmd >> 4)];
	if (event >= 0) {
		int	chan = msg.s.cmd & 0x0f;
		int	ctrl = (event == ET_PITCH ? 0 : msg.s.p1);
		if (m_Learn) {	// if we're learning MIDI assignments
			if (m_RowSel >= 0) {
				int	PropIdx = RowToProp(m_RowSel);
				Assign(PropIdx, event, chan, ctrl);
				theApp.GetMain()->SendMessage(UWM_MIDIROWEDIT, m_RowSel, IDC_MS_CTRL);
			}
		}
		if (GetCount()) {	// if rows are created
			int	PropIdx = GetMapping(event, chan, ctrl);
			if (PropIdx >= 0) {	// if property is mapped
				int	RowIdx = PropToRow(PropIdx);
				if (RowIdx >= 0)	// if property has a row dialog, update it
					GetRow(RowIdx)->SetValue(msg.s.p2 - CENTER_POS);	// convert to signed
			}
		}
	}
}

void CMidiSetupDlg::OnLearn() 
{
	m_Learn ^= 1;
	ShowRowSel(m_Learn);
}

void CMidiSetupDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CRowDialog::OnShowWindow(bShow, nStatus);
	if (bShow) {
		if (!GetCount()) {	// if showing and rows not created yet
			CreateCols(COLS, m_ColInfo);
			CreateRows(CalcRowCount(), TOP_MARGIN);	// create rows
		}
	} else {	// hiding window
		SetLearn(FALSE);	// disable learn mode to avoid confusion
	}
}

void CMidiSetupDlg::OnPageSelect() 
{
	int	PrevPage = m_CurPage;
	UpdateData();
	if (m_CurPage == PrevPage)
		return;	// nothing to do
	// if switching between channel pages
	if (m_CurPage < PT_GLOBAL && PrevPage < PT_GLOBAL)
		UpdateRows();	// no need to recreate rows, just update them
	else {	// switching from channel page to global page, or vice versa
		m_RowSel = -1;
		ReplaceRows(CalcRowCount());
	}
}

void CMidiSetupDlg::OnOpen() 
{
	CFileDialog	fd(TRUE, MIDI_SETUP_EXT, NULL, OFN_HIDEREADONLY,
		LDS(IDS_MIDI_SETUP_FILTER));
	if (fd.DoModal() == IDOK) {
		CMidiInfo	mi;
		if (mi.Read(fd.GetPathName()))
			SetInfo(mi);
	}
}

void CMidiSetupDlg::OnSave() 
{
	CFileDialog	fd(FALSE, MIDI_SETUP_EXT, NULL, OFN_OVERWRITEPROMPT,
		LDS(IDS_MIDI_SETUP_FILTER));
	if (fd.DoModal() == IDOK) {
		CMidiInfo	mi(m_Info);
		mi.Write(fd.GetPathName());
	}
}

void CMidiSetupDlg::OnReset() 
{
	if (AfxMessageBox(IDS_MS_RESET_WARN, MB_YESNO) == IDYES)
		RestoreDefaults();
}
