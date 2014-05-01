// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*	
		chris korda

		revision history:
		rev		date	comments
		00		02aug07	initial version
		01		23nov07	support Unicode
		02		15jan08	replace OnNotify with individual handlers
		03		30jan08	while dragging, if Esc pressed, cancel drag
        04      23oct08	customize
		05		11jan09	move batch processing here

		job control dialog

*/

// JobControlDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Fractice.h"
#include "JobControlDlg.h"
#include "PathStr.h"
#include "MainFrm.h"
#include "FracticeDoc.h"
#include "FracticeView.h"
#include "PosterProgressDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CJobControlDlg dialog

IMPLEMENT_DYNAMIC(CJobControlDlg, CToolDlg);

// this array init must match job state enum
const int CJobControlDlg::m_JobStateID[JOB_STATES] = {
	IDS_JOB_WAITING,
	IDS_JOB_POSTPONED,
	IDS_JOB_RUNNING,
	IDS_JOB_ABORTED,
	IDS_JOB_FAILED,
	IDS_JOB_DONE
};

// all dialog controls must have MFC objects and must be listed here
const CCtrlResize::CTRL_LIST CJobControlDlg::m_CtrlList[] = {
	{IDC_JOB_LIST,		BIND_ALL},
	{IDOK,				BIND_RIGHT},
	{IDC_JOB_MOVE_UP,	BIND_RIGHT},
	{IDC_JOB_MOVE_DOWN, BIND_RIGHT},
	{IDC_JOB_POSTPONE,	BIND_RIGHT},
	{IDC_JOB_DELETE,	BIND_RIGHT},
	{IDC_JOB_START,		BIND_RIGHT},
	{IDC_JOB_ABORT,		BIND_RIGHT},
	{IDC_JOB_SKIP,		BIND_RIGHT},
	{IDC_JOB_PROG_CAP,	BIND_BOTTOM | BIND_LEFT},
	{IDC_JOB_PROGRESS,	BIND_BOTTOM | BIND_LEFT | BIND_RIGHT},
	{IDC_JOB_PERCENT,	BIND_BOTTOM | BIND_RIGHT},
	{0, 0}	// list terminator
};

// this array init must match list column enum
const CJobControlDlg::LIST_COL CJobControlDlg::m_ListCol[] = {
	{IDS_JOB_NAME,		LVCFMT_LEFT, 100},
	{IDS_JOB_SOURCE,	LVCFMT_LEFT, 75},
	{IDS_JOB_DEST,		LVCFMT_LEFT, 75},
	{IDS_JOB_START,		LVCFMT_LEFT, 50},
	{IDS_JOB_END,		LVCFMT_LEFT, 50},
	{IDS_JOB_STATUS,	LVCFMT_LEFT, 100},
};

const LPCTSTR CJobControlDlg::JOB_FILE_PATH = _T("Fractice.frj");
const LPCTSTR CJobControlDlg::JOB_FILE_EXT = _T(".frj");

#define	RK_JOB_SHUTDOWN		_T("JobShutdown")

CJobControlDlg::CJobControlDlg(CWnd* pParent /*=NULL*/)
	: CToolDlg(IDD, IDR_MAINFRAME, _T("JobControlDlg"), pParent)
{
	//{{AFX_DATA_INIT(CJobControlDlg)
	//}}AFX_DATA_INIT
	m_Recorder = NULL;
	m_InitRect.SetRectEmpty();
	m_JobFileRead = FALSE;
	m_Shutdown = theApp.RdRegBool(RK_JOB_SHUTDOWN, FALSE);
	m_WasAborted = FALSE;
	m_BatchMode = FALSE;
	m_BatchStop = FALSE;
	m_hPosterDlg = NULL;
}

CJobControlDlg::~CJobControlDlg()
{
	theApp.WrRegBool(RK_JOB_SHUTDOWN, m_Shutdown);
}

BOOL CJobControlDlg::Create(CRecorder *Recorder, UINT nIDTemplate, CWnd* pParentWnd)
{
	m_Recorder = Recorder;
	return(CToolDlg::Create(nIDTemplate, pParentWnd));
}

void CJobControlDlg::UpdateUI(int JobIdx)
{
	int	jobs = GetJobCount();
	if (JobIdx < 0)
		JobIdx = GetCurSel();
	// update job positioning buttons
	m_MoveUpBtn.EnableWindow(JobIdx > 0);
	m_MoveDownBtn.EnableWindow(JobIdx >= 0 && JobIdx < jobs - 1);
	// update job status buttons; don't let user change status of running job
	bool	StatChgOK = JobIdx >= 0 && !IsRunning(JobIdx);
	m_PostponeBtn.EnableWindow(StatChgOK);
	m_DeleteBtn.EnableWindow(StatChgOK);
	// update job control buttons
	m_StartBtn.EnableWindow(jobs && !m_BatchMode && FindWaiting() >= 0);
	m_AbortBtn.EnableWindow(jobs && m_BatchMode);
	m_SkipBtn.EnableWindow(JobIdx >= 0 && m_BatchMode);
}

void CJobControlDlg::UpdateList()
{
	m_List.SetItemCountEx(GetJobCount(), 0);	// invalidate all items
	UpdateUI();
}

void CJobControlDlg::SetStatus(int JobIdx, int Status)
{
	m_Job.m_Info[JobIdx].m_Status = Status;
	m_List.Update(JobIdx);
	UpdateUI();
}

void CJobControlDlg::SetCurSel(int JobIdx)
{
	m_List.SetSelectionMark(JobIdx);
	int	mask = LVIS_SELECTED | LVIS_FOCUSED;
	m_List.SetItemState(JobIdx, mask, mask);
}

void CJobControlDlg::Add(CJobInfo& Info)
{
	Info.m_ID = ++m_Job.m_NextID;
	if (Info.m_Name.IsEmpty())
		Info.m_Name.Format(_T("Job %d"), Info.m_ID);
	m_Job.m_Info.Add(Info);
	UpdateList();
}

void CJobControlDlg::Delete(int JobIdx)
{
	m_Job.m_Info.RemoveAt(JobIdx);
	if (!GetJobCount())		// if queue is empty
		m_Job.m_NextID = 0;	// reset unique ID sequence
	UpdateList();
}

void CJobControlDlg::Move(int Src, int Dest)
{
	if (Src == Dest)
		return;	// nothing to do
	CJobInfo	info = m_Job.m_Info[Src];
	m_Job.m_Info.RemoveAt(Src);
	m_Job.m_Info.InsertAt(min(Dest, GetJobCount()), info);
	SetCurSel(Dest);
	UpdateList();
}

void CJobControlDlg::ToggleStatus(int JobIdx, int Status1, int Status2)
{
	SetStatus(JobIdx, GetStatus(JobIdx) == Status1 ? Status2 : Status1);
}

void CJobControlDlg::UpdateStatuses(int OldStatus, int NewStatus)
{
	for (int i = 0; i < GetJobCount(); i++) {
		if (GetStatus(i) == OldStatus)
			SetStatus(i, NewStatus);
	}
}

void CJobControlDlg::SetProgressPos(int Pos)
{
	m_Progress.SetPos(Pos);
	CString	s;
	s.Format(_T("%d%%"), Pos);
	m_Percent.SetWindowText(s);
}

int CJobControlDlg::FindByID(int JobID) const
{
	int	jobs = GetJobCount();
	for (int i = 0; i < jobs; i++) {
		if (m_Job.m_Info[i].m_ID == JobID)
			return(i);
	}
	return(-1);
}

int CJobControlDlg::FindByStatus(int Status) const
{
	int	jobs = GetJobCount();
	for (int i = 0; i < jobs; i++) {
		if (m_Job.m_Info[i].m_Status == Status)
			return(i);
	}
	return(-1);
}

bool CJobControlDlg::StartJob(int JobIdx)
{
	if (FindRunning() >= 0)
		return(FALSE);	// can't start a job if one is already running
	CJobInfo& info = m_Job.m_Info[JobIdx];
	info.m_ErrorMsg.Empty();
	info.m_Start = CTime::GetCurrentTime();
	info.m_End = 0;
	SetStatus(JobIdx, JOB_RUNNING);
	return(TRUE);
}

bool CJobControlDlg::EndJob(int Status, LPCTSTR ErrorMsg)
{
	int	JobIdx = FindRunning();
	if (JobIdx < 0)
		return(FALSE);
	CJobInfo& info = m_Job.m_Info[JobIdx];
	info.m_End = CTime::GetCurrentTime();
	info.m_ErrorMsg = ErrorMsg;
	SetStatus(JobIdx, Status);
	if (!m_BatchStop)
		StartNextJob();
	return(TRUE);
}

bool CJobControlDlg::FinishJob()
{
	return(EndJob(JOB_DONE));
}

bool CJobControlDlg::AbortJob()
{
	return(EndJob(JOB_ABORTED));
}

bool CJobControlDlg::SkipJob()
{
	int	JobIdx = FindRunning();
	if (JobIdx < 0)
		return(FALSE);
	if (IsPoster(JobIdx)) {
		if (m_hPosterDlg != NULL)
			return(::PostMessage(m_hPosterDlg, WM_CLOSE, 0, 0) != 0);
		return(FALSE);
	}
	return(m_Recorder->Stop());	// abort current recording
}

bool CJobControlDlg::FailJob(LPCTSTR ErrorMsg)
{
	return(EndJob(JOB_FAILED, ErrorMsg));
}

bool CJobControlDlg::ShowErrorMsg(int JobIdx)
{
	CJobInfo& info = m_Job.m_Info[JobIdx];
	if (info.m_ErrorMsg.IsEmpty())
		return(FALSE);
	CString	caption, msg;
	caption = CString(theApp.m_pszAppName) + _T(" - ") + info.m_Name;
	msg = CString((LPCTSTR)IDS_JOB_STOPPED_ERROR) + _T("\n\n") + info.m_ErrorMsg;
	MessageBox(msg, caption, MB_ICONINFORMATION);
	return(TRUE);
}

void CJobControlDlg::Abort()
{
	m_WasAborted = TRUE;
	SetBatchMode(FALSE);
	UpdateUI();	// update UI to reflect final state
}

CString CJobControlDlg::FormatTime(CTime Time)
{
	CString	s;
	if (Time != 0) {
		s = Time.Format("%#I:%M");
		s += Time.GetHour() < 12 ? "a" : "p";
	} else
		s = "-";
	return(s);
}

bool CJobControlDlg::SetBatchMode(bool Enable)
{
	if (Enable == m_BatchMode)
		return(TRUE);	// nothing to do
	bool	retc = FALSE;
	if (Enable) {
		if (!GetJobCount())
			return(FALSE);	// nothing to do
		if (!m_Recorder->StopCheck())	// can't run batch jobs while recording
			return(FALSE);
		// job control shutdown save check belongs here
		m_Recorder->GetJobInfo(m_PreJobInfo);	// save recorder state
		m_BatchMode = TRUE;	// enable batch mode
		m_BatchStop = FALSE;
		retc = StartNextJob();
	} else {	// disable batch mode
		m_BatchStop = TRUE;	// so stop doesn't start next job
		retc = SkipJob();
		m_BatchMode = FALSE;	// disable batch mode
		m_Recorder->SetJobInfo(m_PreJobInfo);	// restore recorder state
		UpdateUI();
		if (GetShutdown() && !WasAborted())
			theApp.GetMain()->PostMessage(UWM_SHUTDOWN, TRUE);	// give warning
	}
	return(retc);
}

bool CJobControlDlg::StartNextJob()
{
	int	JobIdx;
	while ((JobIdx = FindWaiting()) >= 0) {
		StartJob(JobIdx);
		const CJobInfo&	info = m_Job.m_Info[JobIdx];
		if (IsPoster(JobIdx)) {	// if making a poster
			const CSnapshot& snap = info.m_RecInfo.m_Snap[0];	// single snapshot
			WORD	BitCount = info.m_RecInfo.m_BitCount;
			// m_FrameRate is actually DPI in this case; see RecordInfo.h
			float	DPI = info.m_RecInfo.m_FrameRate;
			CPosterProgressDlg	*dlg = new CPosterProgressDlg;
			if (dlg->Create(snap, info.m_RecordPath, BitCount, DPI)) {
				m_hPosterDlg = dlg->m_hWnd;	// save poster dialog window handle
				return(TRUE);	// started poster
			}
		} else {	// recording a movie
			m_Recorder->SetJobInfo(info);	// restore state for this job
			if (m_Recorder->Record(info.m_RecInfo, NULL, info.m_RecordPath))
				return(TRUE);	// started recording
		}
		SetStatus(JobIdx, CJobControlDlg::JOB_FAILED);
	}
	SetBatchMode(FALSE);	// no waiting jobs
	return(FALSE);
}

void CJobControlDlg::DoDataExchange(CDataExchange* pDX)
{
	CToolDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CJobControlDlg)
	DDX_Control(pDX, IDC_JOB_PERCENT, m_Percent);
	DDX_Control(pDX, IDC_JOB_PROG_CAP, m_ProgCap);
	DDX_Control(pDX, IDOK, m_OKBtn);
	DDX_Control(pDX, IDC_JOB_START, m_StartBtn);
	DDX_Control(pDX, IDC_JOB_SKIP, m_SkipBtn);
	DDX_Control(pDX, IDC_JOB_PROGRESS, m_Progress);
	DDX_Control(pDX, IDC_JOB_POSTPONE, m_PostponeBtn);
	DDX_Control(pDX, IDC_JOB_MOVE_UP, m_MoveUpBtn);
	DDX_Control(pDX, IDC_JOB_MOVE_DOWN, m_MoveDownBtn);
	DDX_Control(pDX, IDC_JOB_LIST, m_List);
	DDX_Control(pDX, IDC_JOB_ABORT, m_AbortBtn);
	DDX_Control(pDX, IDC_JOB_DELETE, m_DeleteBtn);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CJobControlDlg, CToolDlg)
	//{{AFX_MSG_MAP(CJobControlDlg)
	ON_BN_CLICKED(IDC_JOB_ABORT, OnAbort)
	ON_WM_CLOSE()
	ON_NOTIFY(NM_DBLCLK, IDC_JOB_LIST, OnDblclkList)
	ON_BN_CLICKED(IDC_JOB_DELETE, OnDelete)
	ON_WM_DESTROY()
	ON_COMMAND(ID_JOB_EDIT_CLEAR_LIST, OnEditClearList)
	ON_COMMAND(ID_JOB_EDIT_DELETE_DONE, OnEditDeleteDone)
	ON_COMMAND(ID_JOB_EDIT_DONE_WAITING, OnEditDoneWaiting)
	ON_COMMAND(ID_JOB_EDIT_FAILED_WAITING, OnEditFailedWaiting)
	ON_COMMAND(ID_JOB_EDIT_POSTPONED_WAITING, OnEditPostponedWaiting)
	ON_COMMAND(ID_JOB_EDIT_WAITING_POSTPONED, OnEditWaitingPostponed)
	ON_COMMAND(ID_JOB_FILE_LOAD_LIST, OnFileLoadList)
	ON_COMMAND(ID_JOB_FILE_SAVE_LIST, OnFileSaveList)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_JOB_LIST, OnGetdispinfoList)
	ON_WM_GETMINMAXINFO()
	ON_WM_INITMENUPOPUP()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_JOB_LIST, OnItemchangedList)
	ON_BN_CLICKED(IDC_JOB_MOVE_DOWN, OnMoveDown)
	ON_BN_CLICKED(IDC_JOB_MOVE_UP, OnMoveUp)
	ON_COMMAND(ID_JOB_OPTS_SHUTDOWN, OnOptsShutdown)
	ON_BN_CLICKED(IDC_JOB_POSTPONE, OnPostpone)
	ON_NOTIFY(NM_RCLICK, IDC_JOB_LIST, OnRclickList)
	ON_NOTIFY(ULVN_REORDER, IDC_JOB_LIST, OnReorderList)
	ON_WM_SHOWWINDOW()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_JOB_SKIP, OnSkip)
	ON_BN_CLICKED(IDC_JOB_START, OnStart)
	ON_UPDATE_COMMAND_UI(ID_JOB_EDIT_DELETE, OnUpdateDelete)
	ON_UPDATE_COMMAND_UI(ID_JOB_OPTS_SHUTDOWN, OnUpdateOptsShutdown)
	ON_UPDATE_COMMAND_UI(ID_JOB_EDIT_POSTPONE, OnUpdatePostpone)
	ON_UPDATE_COMMAND_UI(ID_JOB_EDIT_SKIP, OnUpdateSkip)
	ON_UPDATE_COMMAND_UI(ID_JOB_VIEW_ERROR, OnUpdateViewError)
	ON_COMMAND(ID_JOB_VIEW_ERROR, OnViewError)
	ON_COMMAND(ID_JOB_EDIT_DELETE, OnDelete)
	ON_COMMAND(ID_JOB_EDIT_POSTPONE, OnPostpone)
	ON_COMMAND(ID_JOB_EDIT_SKIP, OnSkip)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CJobControlDlg message handlers

BOOL CJobControlDlg::OnInitDialog() 
{
	CToolDlg::OnInitDialog();
	
	m_Resize.AddControlList(this, m_CtrlList);
	for (int i = 0; i < COLS; i++) {	// create list columns
		const LIST_COL& lc = m_ListCol[i];
		m_List.InsertColumn(i, LDS(lc.TitleID), lc.Align, lc.Width);
	}
	GetWindowRect(m_InitRect);	// save dialog's initial size
	SetIcon(theApp.LoadIcon(IDR_MAINFRAME), FALSE);
	DWORD	ExStyle = m_List.GetExtendedStyle();
	ExStyle |= LVS_EX_FULLROWSELECT;
	m_List.SetExtendedStyle(ExStyle);

	// read job file
	CPathStr	Path;
	theApp.GetAppDataFolder(Path);
	Path.Append(JOB_FILE_PATH);
	m_JobPath = Path;
	if (PathFileExists(m_JobPath)) {	// if job file exists
		m_JobFileRead = m_Job.Read(m_JobPath);	// read jobs
		if (!m_JobFileRead) {	// if read failed, assume file is damaged
			DeleteFile(m_JobPath);	// try to delete job file
			CJobList	DefJob;
			m_Job = DefJob;	// reset job list, removing any corrupt data
		}
	}
	int	JobIdx = FindRunning();
	if (JobIdx >= 0)	// a job was left running
		SetStatus(JobIdx, JOB_ABORTED);	// abort it

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CJobControlDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	if (bShow && !m_WasShown) {	// if first showing
		UpdateList();
		PostMessage(WM_SIZE);	// resize controls
	}
	CToolDlg::OnShowWindow(bShow, nStatus);
}

void CJobControlDlg::OnDestroy() 
{
	if (GetJobCount() || m_JobFileRead)	// if we have jobs or job file was read
		m_Job.Write(m_JobPath);	// write jobs
	CToolDlg::OnDestroy();
}

void CJobControlDlg::OnSize(UINT nType, int cx, int cy) 
{
	CToolDlg::OnSize(nType, cx, cy);
	m_Resize.OnSize();
}

void CJobControlDlg::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	if (!m_InitRect.IsRectNull())	// wait until rect is initialized
		lpMMI->ptMinTrackSize = CPoint(m_InitRect.Width(), m_InitRect.Height());
}

BOOL CJobControlDlg::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST) {
		if (m_List.IsDragging()) {	// if dragging
			if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
				m_List.CancelDrag();
			return(TRUE);	// override normal keyboard behavior
		}
	}
	return CToolDlg::PreTranslateMessage(pMsg);
}

void CJobControlDlg::OnGetdispinfoList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LVITEM&	ListItem = pDispInfo->item;
	int	ItemIdx = ListItem.iItem;
	if (ListItem.mask & LVIF_TEXT) {
		const CJobInfo& info = m_Job.m_Info[ItemIdx];
		switch (ListItem.iSubItem) {
		case COL_NAME:
			_tcscpy(ListItem.pszText, info.m_Name);
			break;
		case COL_SOURCE:
			_tcscpy(ListItem.pszText, info.m_Source);
			break;
		case COL_DEST:
			_tcscpy(ListItem.pszText, info.m_Dest);
			break;
		case COL_START:
			_tcscpy(ListItem.pszText, FormatTime(info.m_Start));
			break;
		case COL_END:
			_tcscpy(ListItem.pszText, FormatTime(info.m_End));
			break;
		case COL_STATUS:
			{
				CString	s((LPCTSTR)m_JobStateID[info.m_Status]);
				_tcscpy(ListItem.pszText, s);
			}
			break;
		}
	}
	*pResult = 0;
}

void CJobControlDlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
	UpdateMenu(this, pPopupMenu);	// call menu's UI handlers
 	CToolDlg::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);
}

void CJobControlDlg::OnStart() 
{
	SetProgressPos(0);
	m_WasAborted = FALSE;
	SetBatchMode(TRUE);
}

void CJobControlDlg::OnClose() 
{
	ShowWindow(SW_HIDE);
}

void CJobControlDlg::OnCancel() 
{
	ShowWindow(SW_HIDE);
}

void CJobControlDlg::OnOK() 
{
	ShowWindow(SW_HIDE);
}

void CJobControlDlg::OnPostpone() 
{
	int	JobIdx = GetCurSel();
	if (JobIdx >= 0 && !IsRunning(JobIdx))	// can't change status of running job
		ToggleStatus(JobIdx, JOB_POSTPONED, JOB_WAITING);
}

void CJobControlDlg::OnMoveDown() 
{
	int	JobIdx = GetCurSel();
	if (JobIdx >= 0)
		Move(JobIdx, JobIdx + 1);
}

void CJobControlDlg::OnMoveUp() 
{
	int	JobIdx = GetCurSel();
	if (JobIdx >= 0)
		Move(JobIdx, JobIdx - 1);
}

void CJobControlDlg::OnSkip() 
{
	SkipJob();
}

void CJobControlDlg::OnAbort() 
{
	Abort();
}

void CJobControlDlg::OnViewError() 
{
	int	JobIdx = GetCurSel();
	if (JobIdx >= 0)
		ShowErrorMsg(JobIdx);
}

void CJobControlDlg::OnDelete() 
{
	int	JobIdx = GetCurSel();
	if (JobIdx >= 0)
		Delete(JobIdx);
}

void CJobControlDlg::OnItemchangedList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	if (pNMListView->iItem >= 0 && (pNMListView->uNewState & LVIS_SELECTED)) {
		UpdateUI(pNMListView->iItem);
	}
	*pResult = 0;
}

void CJobControlDlg::OnReorderList(NMHDR* pNMHDR, LRESULT* pResult)
{
	Move(GetCurSel(), m_List.GetInsertPos());
	*pResult = 0;
}

void CJobControlDlg::OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int	JobIdx = GetCurSel();
	if (JobIdx >= 0 && !IsRunning(JobIdx)) {	// can't change status of running job
		if (GetStatus(JobIdx) == JOB_FAILED)
			ShowErrorMsg(JobIdx);
		ToggleStatus(JobIdx, JOB_WAITING, JOB_POSTPONED);
	}
	*pResult = 0;
}

void CJobControlDlg::OnFileLoadList() 
{
	CString	Filter((LPCTSTR)IDS_JOB_FILTER);
	CString	Title((LPCTSTR)IDS_JOB_LOAD_LIST);
	CFileDialog	fd(TRUE, JOB_FILE_EXT, NULL, OFN_HIDEREADONLY, Filter);
	fd.m_ofn.lpstrTitle = Title;
	if (fd.DoModal() == IDOK) {
		m_Job.Read(fd.GetPathName());
		UpdateList();
	}
}

void CJobControlDlg::OnFileSaveList() 
{
	CString	Filter((LPCTSTR)IDS_JOB_FILTER);
	CString	Title((LPCTSTR)IDS_JOB_SAVE_LIST);
	CFileDialog	fd(FALSE, JOB_FILE_EXT, NULL, OFN_OVERWRITEPROMPT, Filter);
	fd.m_ofn.lpstrTitle = Title;
	if (fd.DoModal() == IDOK) {
		m_Job.Write(fd.GetPathName());
	}
}

void CJobControlDlg::OnEditClearList() 
{
	m_Job.m_Info.RemoveAll();
	UpdateList();
}

void CJobControlDlg::OnEditDeleteDone() 
{
	int	JobIdx;
	while ((JobIdx = FindByStatus(JOB_DONE)) >= 0)
		Delete(JobIdx);
}

void CJobControlDlg::OnEditPostponedWaiting() 
{
	UpdateStatuses(JOB_POSTPONED, JOB_WAITING);
}

void CJobControlDlg::OnEditWaitingPostponed() 
{
	UpdateStatuses(JOB_WAITING, JOB_POSTPONED);
}

void CJobControlDlg::OnEditDoneWaiting() 
{
	UpdateStatuses(JOB_DONE, JOB_WAITING);
}

void CJobControlDlg::OnEditFailedWaiting() 
{
	UpdateStatuses(JOB_FAILED, JOB_WAITING);
	UpdateStatuses(JOB_ABORTED, JOB_WAITING);
}

void CJobControlDlg::OnOptsShutdown() 
{
	m_Shutdown ^= 1;
}

void CJobControlDlg::OnUpdateOptsShutdown(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_Shutdown);
}

void CJobControlDlg::OnRclickList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LPNMLISTVIEW	lpnmlv = (LPNMLISTVIEW)pNMHDR;
	if (lpnmlv->iItem >= 0) {
		CMenu	menu;
		menu.LoadMenu(IDR_JOB_CONTROL_CTX);
		CMenu	*mp = menu.GetSubMenu(0);
		CPoint	pt;
		GetCursorPos(&pt);
		UpdateMenu(this, mp);	// call menu's UI handlers
		mp->TrackPopupMenu(0, pt.x, pt.y, this);
	}
	*pResult = 0;
}

void CJobControlDlg::OnUpdatePostpone(CCmdUI* pCmdUI) 
{
	int JobIdx = GetCurSel();
	if (JobIdx >= 0) {
		pCmdUI->Enable(!IsRunning(JobIdx));
		pCmdUI->SetCheck(GetStatus(JobIdx) == JOB_POSTPONED);
	}
}

void CJobControlDlg::OnUpdateDelete(CCmdUI* pCmdUI) 
{
	int JobIdx = GetCurSel();
	pCmdUI->Enable(JobIdx >= 0 && !IsRunning(JobIdx));
}

void CJobControlDlg::OnUpdateSkip(CCmdUI* pCmdUI) 
{
	int JobIdx = GetCurSel();
	pCmdUI->Enable(JobIdx >= 0 && m_BatchMode);
}

void CJobControlDlg::OnUpdateViewError(CCmdUI* pCmdUI) 
{
	int JobIdx = GetCurSel();
	pCmdUI->Enable(JobIdx >= 0 && !m_Job.m_Info[JobIdx].m_ErrorMsg.IsEmpty());
}
