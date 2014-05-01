// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		24jan09	add OnDropFiles
		02		02feb09	add color mapping bar
		03		06feb09	add color cycling bar
		04		15feb09	add Exclusive mode
		05		23feb09	add non-blocking mode
		06		24feb09	add files bar
		07		03mar09	add MIDI
		08		11mar09	move OnImgCancel here
		09		14mar09	add slide show bar
		10		16mar09	add demo
		11		23mar09	support channel MIDI properties
		12		29mar09	add ParmToEnum for origin motion
		13		31mar09	add OnDisplayChange
		14		05jul09	allow safe accelerators in record

        main frame
 
*/

// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__05007120_C9FD_4AA0_962E_022C961FB642__INCLUDED_)
#define AFX_MAINFRM_H__05007120_C9FD_4AA0_962E_022C961FB642__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "HistoryBar.h"
#include "LevelsBar.h"
#include "ParamsBar.h"
#include "PaletteDlg.h"
#include "Recorder.h"
#include "RecordDlg.h"
#include "OptionsDlg.h"
#include "RecentItemList.h"
#include "SizingDialogBar.h"
#include "ColorMappingDlg.h"
#include "ColorCyclingDlg.h"
#include "Mixer.h"
#include "AccelTable.h"
#include "FilesBar.h"
#include "PropertyBar.h"
#include "MixerDlg.h"
#include "MidiSetupDlg.h"
#include "SlideShowDlg.h"

class CFracticeView;

class CMainFrame : public CMDIFrameWnd
{
	DECLARE_DYNCREATE(CMainFrame)
public:
	CMainFrame();

// Types
	typedef CMixer::DISPLAY_MODE_INFO DISPLAY_MODE_INFO;

// Attributes
public:
	CFracticeView	*GetView() const;
	void	SetView(CFracticeView *View);
	CStatusBar&		GetStatusBar();
	CHistoryBar&	GetHistoryBar();
	CLevelsBar&		GetLevelsBar();
	CParamsBar&		GetParamsBar();
	CFilesBar&		GetFilesBar();
	COptionsDlg&	GetOptionsDlg();
	CPaletteDlg&	GetPaletteDlg();
	CColorMappingDlg&	GetColorMappingDlg();
	CColorCyclingDlg&	GetColorCyclingDlg();
	CRecordDlg&	GetRecordDlg();
	CRecorder&	GetRecorder();
	CRecordStatusDlg&	GetRecordStatusDlg();
	CServersDlg&	GetServersDlg();
	CJobControlDlg&	GetJobControlDlg();
	CMixerDlg&	GetMixerDlg();
	CMidiSetupDlg&	GetMidiSetupDlg();
	CSlideShowDlg&	GetSlideShowDlg();
	LPCTSTR	GetStatusMsg() const;
	bool	IsRecording() const;
	HMENU	GetChildFrameMenu() const;
	UINT	GetCurTool() const;
	CRecentFileList&	GetMRUPalette();
	CRecentItemList&	GetMRUImgSize();
	CRecentItemList&	GetMRUQuality();
	CRecentItemList&	GetMRUAntialias();
	HACCEL	GetAccelTable() const;
	void	SetAccelTable(HACCEL Accel);
	bool	IsDistributed() const;
	CMixer&	GetMixer();
	void	SetFolder(int PaneIdx, LPCTSTR Path);

// Operations
public:
	void	OnParamChange();
	void	OnRecord(bool Recording);
	bool	Shutdown(bool Warning = TRUE);
	bool	IsFullScreen() const;
	void	FullScreen(bool Enable);
	bool	IsExclusive() const;
	bool	Exclusive(bool Enable, const DISPLAY_MODE_INFO *ModeInfo = NULL);
	bool	IsDualMonitor() const;
	bool	IsSingleMonitorExclusive() const;
	void	DoIdleLayout();
	void	UpdateBars();
	bool	OpenPalette(LPCTSTR Path, CPoint *DropPoint = NULL);
	bool	ShowDemo();
	void	UpdateMidiDevice();

public:
// Nested classes
	class CStatusMsg {	// temporarily overrides status message
	public:
		CStatusMsg(LPCTSTR Msg, bool WaitCursor = TRUE);
		CStatusMsg(UINT MsgID, bool WaitCursor = TRUE);
		~CStatusMsg();

	protected:
		void	Create(LPCTSTR Msg, bool WaitCursor);
		CString	m_PrevMsg;		// backup of status message
		bool	m_WaitCursor;	// if true, show wait cursor
	};

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL DestroyWindow();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnAppCancel();
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnEditOptions();
	afx_msg void OnFileRecord();
	afx_msg void OnFileRecordStop();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnImgCancel();
	afx_msg void OnImgSlideShow();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnUpdateFileRecord(CCmdUI* pCmdUI);
	afx_msg void OnUpdateImgCancel(CCmdUI* pCmdUI);
	afx_msg void OnUpdateImgSlideShow(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewColorCycling(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewColorMapping(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewFiles(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewHistoryBar(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewJobControl(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewLevels(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewMidiSetup(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewMixer(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewPalette(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewParams(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewRecord(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewRecordStatus(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewServers(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWndExclusive(CCmdUI* pCmdUI);
	afx_msg void OnViewColorCycling();
	afx_msg void OnViewColorMapping();
	afx_msg void OnViewFiles();
	afx_msg void OnViewHistoryBar();
	afx_msg void OnViewJobControl();
	afx_msg void OnViewLevels();
	afx_msg void OnViewMidiSetup();
	afx_msg void OnViewMixer();
	afx_msg void OnViewPalette();
	afx_msg void OnViewParams();
	afx_msg void OnViewRecord();
	afx_msg void OnViewRecordStatus();
	afx_msg void OnViewServers();
	afx_msg void OnWndExclusive();
	afx_msg void OnWndFullScreen();
	afx_msg void OnViewDemo();
	//}}AFX_MSG
#if _MSC_VER < 1300
	afx_msg void OnActivateApp(BOOL bActive, HTASK hTask);
#else
	afx_msg void OnActivateApp(BOOL bActive, DWORD hTask);
#endif
	afx_msg LRESULT OnSetMessageString(WPARAM wParam, LPARAM lParam);
	afx_msg void OnUpdateIndicatorFrameSize(CCmdUI *pCmdUI);
	afx_msg void OnUpdateIndicatorQuality(CCmdUI *pCmdUI);
	afx_msg void OnUpdateIndicatorAntialias(CCmdUI *pCmdUI);
	afx_msg LRESULT OnRcvPacket(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnTcpConnect(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnTcpAccept(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAbortRecord(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnShutdown(WPARAM wParam, LPARAM lParam);
	afx_msg void OnEditTool(UINT nID);
	afx_msg void OnUpdateEditTool(CCmdUI* pCmdUI);
	afx_msg LRESULT OnMidiIn(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnHandleDlgKey(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnDisplayChange(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

// Types
	typedef struct tagSIZING_BAR_INFO {
		int		BarIdx;		// control bar index; see enum below
		int		TitleID;	// string resource ID of bar's title
		int		DockStyle;	// docking style flags
		int		DockBarID;	// dock bar ID for initial docking
		BOOL	InitShow;	// true if bar should show on initial run
		POINT	InitFloat;	// initial floating position in client coords
	} SIZING_BAR_INFO;
	typedef struct tagACCEL_EDIT {	// accelerator table edit
		bool	Enable;		// if true, enable accelerator, else disable it
		int		StartID;	// target command ID, or start of command ID range
		int		EndID;		// if non-zero, end of command ID range
	} ACCEL_EDIT;

// Constants
	enum {	// tool dialogs
		TD_RECORD,
		TD_RECORD_STATUS,
		TD_SERVERS,
		TD_JOB_CONTROL,
		TD_MIDI_SETUP,
		TOOL_DLGS
	};
	enum {	// control bar indices
		// APPEND ONLY: this list generates unique IDs for the bars
		CBI_HISTORY,
		CBI_LEVELS,
		CBI_PARAMETERS,
		CBI_PALETTE,
		CBI_COLOR_MAPPING,
		CBI_COLOR_CYCLING,
		CBI_FILES,
		CBI_MIXER,
		CBI_SLIDE_SHOW,
		CONTROL_BARS
	};
	enum {	// sizing control bars; must match m_SizingBar and m_SizingBarInfo
		SCB_HISTORY,
		SCB_LEVELS,
		SCB_PARAMETERS,
		SCB_PALETTE,
		SCB_COLOR_MAPPING,
		SCB_COLOR_CYCLING,
		SCB_FILES,
		SCB_MIXER,
		SCB_SLIDE_SHOW,
		SIZING_BARS
	};
	static const SIZING_BAR_INFO	m_SizingBarInfo[SIZING_BARS];
	static const ACCEL_EDIT	m_RecordAccelEdit[];	// accelerator edits in record mode
	static const ACCEL_EDIT	m_SMExclAccelEdit[];	// edits in single-monitor exclusive

// Member data
	CFracticeView	*m_View;	// pointer to our view
	bool	m_WasShown;			// true if we were shown at least once
	bool	m_HaveWndPlace;		// true if initial window placement is valid
	WINDOWPLACEMENT	m_WndPlace;	// initial window placement
	CStatusBar	m_StatusBar;	// status bar
	CToolBar	m_ToolBar;		// tool bar
	CString	m_StatusMsg;		// text to display in pane 0 of status bar
	CToolDlg	*m_ToolDlg[TOOL_DLGS];	// list of tool modeless dialogs
	CMySizingControlBar	*m_SizingBar[SIZING_BARS];	// list of sizable control bars
	CHistoryBar	m_HistoryBar;	// sizable control bar for viewing history
	CLevelsBar	m_LevelsBar;	// sizable control bar for escape time histogram
	CParamsBar	m_ParamsBar;	// sizable control bar for parameters
	CFilesBar	m_FilesBar;		// sizable control bar for file browsing
	CSizingDialogBar	m_PaletteBar;		// host bar for palette dialog
	CPaletteDlg			m_PaletteDlg;		// palette dialog
	CSizingDialogBar	m_ColorMappingBar;	// host bar for color mapping dialog
	CColorMappingDlg	m_ColorMappingDlg;	// color mapping dialog
	CSizingDialogBar	m_ColorCyclingBar;	// host bar for color cycling dialog
	CColorCyclingDlg	m_ColorCyclingDlg;	// color cycling dialog
	CPropertyBar	m_MixerBar;	// host bar for mixer dialog
	CMixerDlg	m_MixerDlg;		// mixer dialog
	CRecorder	m_Recorder;		// recording subsystem
	CRecordDlg	m_RecordDlg;	// record dialog
	CMidiSetupDlg	m_MidiSetupDlg;	// MIDI setup dialog
	CSizingDialogBar	m_SlideShowBar;	// host bar for slide show dialog
	CSlideShowDlg	m_SlideShowDlg;	// slide show dialog
	CMenu	m_Menu;				// current menu while in full-screen mode
	CMenu	m_DocMenu;			// document menu
	CMenu	m_RecMenu;			// recording menu
	HMENU	m_ChildFrameMenu;	// menu to display when child frame exists
	CRect	m_PreFullScrnRect;	// window rect to restore when full-screen ends
	CRect	m_FullScreenRect;	// full-screen window rectangle
	CMixer	m_Mixer;			// back buffer window for exclusive mode
	CRect	m_ExclusiveRect;	// exclusive window rectangle
	WINDOWPLACEMENT	m_PreExclWndPlace;	// placement to restore when exclusive ends
	HHOOK	m_KeybdHook;		// if non-zero, handle to keyboard hook procedure
	BOOL	m_ShowToolBar;		// true if toolbar was visible before full-screen
	BOOL	m_ShowStatusBar;	// true if status bar was visible before full-screen
	BOOL	m_ShowSizingBar[SIZING_BARS];	// true if bar was visible before full-screen
	UINT	m_ShowToolDlg;		// tool dialog visibility bitmask
	bool	m_HideChrome;		// true if menus and toolbars are hidden
	bool	m_IsFullScreen;		// true if we're in full-screen mode
	UINT	m_CurTool;			// index of currently selected tool
	COptionsDlg	m_OptsDlg;		// options dialog
	HACCEL	m_PrevMainAccel;	// backup of main accelerators
	CAccelTable	m_RecordAccel;	// safe accelerators while recording a movie
	CAccelTable	m_SMExclAccel;	// safe accelerators in single-monitor exclusive
	bool	m_FullScrnDualMon;	// if true, go full-screen in dual-monitor exclusive
	CRecentFileList	m_MRUPalette;	// most recently used palette files
	CRecentItemList	m_MRUImgSize;	// most recently used image sizes
	CRecentItemList	m_MRUQuality;	// most recently used quality values
	CRecentItemList	m_MRUAntialias;	// most recently used antialiasing values
	CMidiIO	m_MidiIO;			// wrapper for MIDI API
	int		m_MidiDev;			// currently selected MIDI device
	CString	m_MidiSetupPath;	// path of MIDI setup file
	bool	m_NoteToggle[MIDI_PROPS];	// note flip-flops for MIDI properties
	bool	m_DemoMode;			// true if we're showing demo
	bool	m_PrevFitInWindow;	// pre-demo fit in window state

// Helpers
	BOOL	VerifyBarState(LPCTSTR lpszProfileName);
	static	BOOL	VerifyDockState(const CDockState& state, CFrameWnd *Frm);
	bool	CreateSizingBars();
	void	DockSizingBars();
	UINT	GetToolDlgState() const;
	void	SetToolDlgState(UINT State);
	static	bool	ShowWarning(int TemplateID, int TimeoutSecs);
	void	HideChrome(bool Enable);
	static	LRESULT CALLBACK KeybdProc(int nCode, WPARAM wParam, LPARAM lParam);
	bool	HookKeybd(bool Enable);
	bool	SetSingleMonitorExclusiveAccels(bool Enable);
	void	ToggleWindow(CWnd& Wnd);
	static	void CALLBACK MidiCallback(HMIDIIN handle, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
	int		ParmToEnum(double Val, int Enums);
	void	SetMidiProperty(int PropIdx, int Val, bool Toggle);
	bool	LoadMidiSetup();
	bool	StoreMidiSetup();
	void	CleanupDemo();
	static	void	EditAccels(CAccelArray& AccArr, const ACCEL_EDIT *EditList, bool Default);
};

inline CMainFrame::CStatusMsg::CStatusMsg(LPCTSTR Msg, bool WaitCursor)
{
	Create(Msg, WaitCursor);
}

inline CMainFrame::CStatusMsg::CStatusMsg(UINT MsgID, bool WaitCursor)
{
	Create(LDS(MsgID), WaitCursor);
}

inline CFracticeView *CMainFrame::GetView() const
{
	return(m_View);
}

inline CStatusBar& CMainFrame::GetStatusBar()
{
	return(m_StatusBar);
}

inline CHistoryBar& CMainFrame::GetHistoryBar()
{
	return(m_HistoryBar);
}

inline CLevelsBar& CMainFrame::GetLevelsBar()
{
	return(m_LevelsBar);
}

inline CParamsBar& CMainFrame::GetParamsBar()
{
	return(m_ParamsBar);
}

inline CFilesBar& CMainFrame::GetFilesBar()
{
	return(m_FilesBar);
}

inline COptionsDlg& CMainFrame::GetOptionsDlg()
{
	return(m_OptsDlg);
}

inline CPaletteDlg& CMainFrame::GetPaletteDlg()
{
	return(m_PaletteDlg);
}

inline CColorMappingDlg& CMainFrame::GetColorMappingDlg()
{
	return(m_ColorMappingDlg);
}

inline CColorCyclingDlg& CMainFrame::GetColorCyclingDlg()
{
	return(m_ColorCyclingDlg);
}

inline CRecordDlg& CMainFrame::GetRecordDlg()
{
	return(m_RecordDlg);
}

inline CRecorder& CMainFrame::GetRecorder()
{
	return(m_Recorder);
}

inline CRecordStatusDlg& CMainFrame::GetRecordStatusDlg()
{
	return(m_Recorder.GetRecordStatusDlg());
}

inline CServersDlg& CMainFrame::GetServersDlg()
{
	return(m_Recorder.GetServersDlg());
}

inline CJobControlDlg& CMainFrame::GetJobControlDlg()
{
	return(m_Recorder.GetJobControlDlg());
}

inline CMixerDlg& CMainFrame::GetMixerDlg()
{
	return(m_MixerDlg);
}

inline CMidiSetupDlg& CMainFrame::GetMidiSetupDlg()
{
	return(m_MidiSetupDlg);
}

inline CSlideShowDlg& CMainFrame::GetSlideShowDlg()
{
	return(m_SlideShowDlg);
}

inline LPCTSTR CMainFrame::GetStatusMsg() const
{
	return(m_StatusMsg);
}

inline bool CMainFrame::IsFullScreen() const
{
	return(m_IsFullScreen);
}

inline bool CMainFrame::IsExclusive() const
{
	return(m_Mixer.IsExclusive());
}

inline bool CMainFrame::IsDualMonitor() const
{
	return(m_Mixer.IsDualMonitor());
}

inline bool CMainFrame::IsSingleMonitorExclusive() const
{
	return(IsExclusive() && !IsDualMonitor());
}

inline bool CMainFrame::IsRecording() const
{
	return(m_Recorder.IsRecording());
}

inline UINT CMainFrame::GetCurTool() const
{
	return(m_CurTool);
}

inline HMENU CMainFrame::GetChildFrameMenu() const
{
	return(m_ChildFrameMenu);
}

inline CRecentFileList& CMainFrame::GetMRUPalette()
{
	return(m_MRUPalette);
}

inline CRecentItemList& CMainFrame::GetMRUImgSize()
{
	return(m_MRUImgSize);
}

inline CRecentItemList& CMainFrame::GetMRUQuality()
{
	return(m_MRUQuality);
}

inline CRecentItemList& CMainFrame::GetMRUAntialias()
{
	return(m_MRUAntialias);
}

inline HACCEL CMainFrame::GetAccelTable() const
{
	return(m_hAccelTable);
}

inline void CMainFrame::SetAccelTable(HACCEL Accel)
{
	m_hAccelTable = Accel;
}

inline bool CMainFrame::IsDistributed() const
{
	return(m_Recorder.IsDistributed());
}

inline void CMainFrame::DoIdleLayout()
{
	if (m_nIdleFlags & idleLayout)
		RecalcLayout();
}

inline CMixer& CMainFrame::GetMixer()
{
	return(m_Mixer);
}

inline int CMainFrame::ParmToEnum(double Val, int Enums)
{
	int ival = trunc(Val * Enums);
	return(CLAMP(ival, 0, Enums - 1));
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__05007120_C9FD_4AA0_962E_022C961FB642__INCLUDED_)
