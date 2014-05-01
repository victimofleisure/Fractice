// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      11jan09	initial version
		01		16jan09	add pause/resume
		02		01feb09	in StartJob, add color offset
		03		26feb09	all engine messages are now posted
		04		26feb09	in OnAbortRender, delete error string
		05		21mar09	in OnSysCommand, mask off unreliable low nibble
		06		06jan10	W64: make OnTimer 64-bit compatible
		07		06jan10	W64: in OnStripDone, cast thread index to 32-bit

        poster dialog
 
*/

// PosterProgressDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Fractice.h"
#include "PosterProgressDlg.h"
#include "MainFrm.h"
#include "PathStr.h"
#include "RecordDlg.h"	// for time formatting

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPosterProgressDlg dialog

IMPLEMENT_DYNAMIC(CPosterProgressDlg, CDialog);

CPosterProgressDlg	*CPosterProgressDlg::m_This;

CPosterProgressDlg::CPosterProgressDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
	//{{AFX_DATA_INIT(CPosterProgressDlg)
	//}}AFX_DATA_INIT
	m_Main = NULL;
	m_JobDlg = NULL;
	m_BitCount = 0;
	m_Resolution = 0;
	m_FrameUID = 0;
	m_RowsDone = 0;
	m_Running = FALSE;
	m_BatchMode = FALSE;
	m_SampleStart = 0;
}

CString	CPosterProgressDlg::GetExceptionMsg(CException *e)
{
	TCHAR	msg[256];
	e->GetErrorMessage(msg, sizeof(msg));
	return(CString(msg));
}

void CPosterProgressDlg::ReportError(LPCTSTR Msg)
{
	if (m_BatchMode)
		m_JobDlg->FailJob(Msg);	// may start a new job before returning
	else
		AfxMessageBox(Msg);
}

void CPosterProgressDlg::ReportError(UINT StrID)
{
	CString	msg;
	msg.LoadString(StrID);
	ReportError(msg);
}

void CPosterProgressDlg::ReportException(CException *e)
{
	ReportError(GetExceptionMsg(e));
}

BOOL CPosterProgressDlg::Create(const CSnapshot& Snap, LPCTSTR Path, WORD BitCount, float Resolution)
{
	m_Snap = Snap;
	m_DestPath = Path;
	m_BitCount = BitCount;
	m_Resolution = Resolution;
	if (!CDialog::Create(IDD))
		return(FALSE);
	ShowWindow(SW_SHOW);
	return(TRUE);
}

bool CPosterProgressDlg::StartJob()
{
	// launch poster rendering engine
	UINT	Threads = m_Main->GetOptionsDlg().GetThreadCount();
	if (!m_Engine.LaunchThreads(m_hWnd, Threads)) {
		ReportError(IDS_CANT_LAUNCH_ENGINE);
		return(FALSE);
	}
	// maybe resume a paused poster
	CPathStr	Path(m_DestPath);
	Path.RenameExtension(ROW_INDEX_EXT);
	if (PathFileExists(Path)) {	// if paused poster exists in destination folder
		CMainFrame::CStatusMsg	status(IDS_PSTR_RESUMING);
		CPausedPosterInfo	Info;
		if (!SerializePausedJob(FALSE, Info))	// read paused poster info
			return(FALSE);	// error was already reported
		if (!m_Engine.ResumePosterJob(Info.m_RowIndex)) {
			ReportError(IDS_PSTR_CANT_RESUME);
			return(FALSE);
		}
		m_Snap = Info.m_Snap;
		DeleteFile(Path);	// delete row index file
	}
	// get bounds and set math precision
	BigRect	Bounds(m_Snap.m_MathPrec, 0, 0);
	m_Snap.GetBounds(Bounds);	// get bounds from snapshot
	m_Engine.SetMathPrec(m_Snap.m_MathPrec);	// set our engine's precision
	m_Engine.SetPalette(m_Snap.m_Palette, m_Snap.m_Quality, 
		m_Snap.m_CycleLen, m_Snap.m_ColorOffset);
	// row data file path is same as destination file but different extension
	CPathStr	RowFilePath(m_DestPath);
	RowFilePath.RenameExtension(ROW_DATA_EXT);
	// build render info
	m_RowsDone = 0;
	m_FrameUID = GetTickCount();
	m_FrameSize = CSize(m_Snap.m_ImageSize.cx * m_Snap.m_Antialias, 
		m_Snap.m_ImageSize.cy * m_Snap.m_Antialias);
	RENDER_INFO	ri;
	ri.FrameBuf = NULL;
	ri.FractalType = m_Snap.m_FractalType;
	ri.FrameSize = m_FrameSize;
	ri.FrameUID = m_FrameUID;
	ri.Quality = m_Snap.m_Quality;
	ri.DeepZoom = m_Snap.m_DeepZoom;
	ri.Exponent = m_Snap.m_Exponent;
	m_Clock.Reset();
	TRY {
		m_Engine.BeginPosterJob(ri, Bounds, m_Snap, RowFilePath);
	}
	CATCH(CFileException, e)
	{
		ReportException(e);
		return(FALSE);
	}
	END_CATCH
	// update UI
	InitStats();
	m_Progress.SetRange32(0, m_FrameSize.cy);
	m_Timer.Create(m_hWnd, TIMER_ID, TIMER_PERIOD);
	m_Main->OnRecord(TRUE);
	m_Main->GetServersDlg().ResetStatistics();
	m_This = this;	// register global instance
	m_Running = TRUE;	// job is started
	return(TRUE);
}

void CPosterProgressDlg::OnJobDone()
{
	TRY {
		m_Engine.EndPosterJob();
	}
	CATCH(CFileException, e)
	{
		ASSERT(0);	// shouldn't happen
	}
	END_CATCH
	m_Main->OnRecord(FALSE);
	m_Main->GetServersDlg().ResetStatistics();
	m_This = NULL;	// unregister global instance
	m_Running = FALSE;	// job is finished
}

void CPosterProgressDlg::EndJob()
{
	if (m_Running) {
		OnJobDone();
		if (m_BatchMode)
			m_JobDlg->FinishJob();	// may start a new job before returning
	}
}

void CPosterProgressDlg::AbortJob()
{
	if (m_Running) {
		OnJobDone();
		if (m_BatchMode)
			m_JobDlg->AbortJob();	// may start a new job before returning
	}
}

void CPosterProgressDlg::FailJob(LPCTSTR Msg)
{
	if (m_Running) {
		OnJobDone();
		ReportError(Msg);	// may start a new job before returning
	}
}

void CPosterProgressDlg::SetTime(CStatic& Ctrl, int Secs)
{
	CString	s;
	CRecordDlg::SecsToTime(Secs, s);
	Ctrl.SetWindowText(s);
}

void CPosterProgressDlg::InitStats()
{
	m_FilenameStat.SetWindowText(PathFindFileName(m_DestPath));
	CString	s;
	s.Format(_T("%d x %d (%dx)"), m_Snap.m_ImageSize.cx, m_Snap.m_ImageSize.cy,
		m_Snap.m_Antialias);
	m_ImageSizeStat.SetWindowText(s);
	m_AvgSPR.Create(RUNAVG_SIZE);
}

void CPosterProgressDlg::EndStats()
{
	OnTimer(0);	// show final statistics
	GetDlgItem(IDCANCEL)->EnableWindow(FALSE);	// disable cancel button
}

bool CPosterProgressDlg::StopCheck()
{
	if (AfxMessageBox(LDS(IDS_PSTR_STOP_CHECK), MB_YESNO | MB_DEFBUTTON2) == IDNO)
		return(FALSE);
	OnClose();
	return(TRUE);
}

bool CPosterProgressDlg::PauseCheck()
{
	if (AfxMessageBox(IDS_PSTR_PAUSE_CHECK, MB_YESNO | MB_DEFBUTTON2) == IDNO)
		return(FALSE);
	return(Pause());
}

void CPosterProgressDlg::CPausedPosterInfo::Serialize(CArchive& ar)
{
	if (ar.IsStoring()) {
		ar << ARCHIVE_VERSION;
	} else {
		UINT	Version;
		ar >> Version;
		if (Version > ARCHIVE_VERSION)
			AfxThrowArchiveException(CArchiveException::badIndex, ar.m_strFileName);
	}
	m_Snap.Serialize(ar);
	m_RowIndex.Serialize(ar);
}

bool CPosterProgressDlg::SerializePausedJob(bool Store, CPausedPosterInfo& Info)
{
	CPathStr	Path(m_DestPath);
	Path.RenameExtension(ROW_INDEX_EXT);
	TRY {
		CFile	fp(Path, Store ? 
			(CFile::modeCreate | CFile::modeWrite) : CFile::modeRead);
		CArchive	ar(&fp, Store ? CArchive::store : CArchive::load);
		Info.Serialize(ar);
	}
	CATCH(CArchiveException, e)
	{
		ReportException(e);
		return(FALSE);
	}
	CATCH(CFileException, e)
	{
		ReportException(e);
		return(FALSE);
	}
	END_CATCH
	return(TRUE);
}

bool CPosterProgressDlg::Pause()
{
	CMainFrame::CStatusMsg	status(IDS_PSTR_PAUSING);
	CPausedPosterInfo	Info;
	m_Engine.GetRowIndex(Info.m_RowIndex);
	Info.m_Snap = m_Snap;
	SerializePausedJob(TRUE, Info);	// store
	OnClose();
	return(TRUE);
}

void CPosterProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPosterProgressDlg)
	DDX_Control(pDX, IDC_POPR_PCT_DONE, m_PctDoneStat);
	DDX_Control(pDX, IDC_POPR_PIXEL_RATE, m_PixelRateStat);
	DDX_Control(pDX, IDC_POPR_REMAINING, m_RemainingStat);
	DDX_Control(pDX, IDC_POPR_IMAGE_SIZE, m_ImageSizeStat);
	DDX_Control(pDX, IDC_POPR_FILENAME, m_FilenameStat);
	DDX_Control(pDX, IDC_POPR_ELAPSED, m_ElapsedStat);
	DDX_Control(pDX, IDC_POPR_ROWS_DONE, m_RowsDoneStat);
	DDX_Control(pDX, IDC_POPR_PROGRESS, m_Progress);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPosterProgressDlg, CDialog)
	//{{AFX_MSG_MAP(CPosterProgressDlg)
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_POPR_PAUSE, OnPause)
	ON_WM_SYSCOMMAND()
	//}}AFX_MSG_MAP
	ON_MESSAGE(UWM_RENDERDONE, OnRenderDone)
	ON_MESSAGE(UWM_STRIPDONE, OnStripDone)
	ON_MESSAGE(UWM_ABORTRENDER, OnAbortRender)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPosterProgressDlg message handlers

BOOL CPosterProgressDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	m_Main = theApp.GetMain();
	m_JobDlg = &m_Main->GetJobControlDlg();
	m_BatchMode = m_JobDlg->GetBatchMode();
	if (!StartJob())
		PostMessage(WM_CLOSE);	// DestroyWindow would assert

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPosterProgressDlg::PostNcDestroy() 
{
	CDialog::PostNcDestroy();
	delete this;	// destroy our instance; assume we were created dynamically
}

void CPosterProgressDlg::OnPause() 
{
	PauseCheck();
}

void CPosterProgressDlg::OnOK()
{
	PauseCheck();
}

void CPosterProgressDlg::OnCancel() 
{
	StopCheck();
}

void CPosterProgressDlg::OnClose() 
{
	AbortJob();
	DestroyWindow();	// instead of base class OnClose; we're modeless
}

void CPosterProgressDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	switch (nID & 0xFFF0) {	// mask off unreliable low nibble
	case SC_CLOSE:	// must handle close because OnCancel is not called
		StopCheck();
		break;
	default:
		CDialog::OnSysCommand(nID, lParam);
	}
}

LRESULT CPosterProgressDlg::OnStripDone(WPARAM wParam, LPARAM lParam)
{
	if (wParam != m_FrameUID)	// if unexpected frame
		return(0);	// ignore message
	m_Engine.WriteLocalStrip(INT64TO32(lParam));
	return(0);
}

LRESULT CPosterProgressDlg::OnRenderDone(WPARAM wParam, LPARAM lParam)
{
	// wParam: frame UID, lParam: none
	if (wParam != m_FrameUID)	// if unexpected frame
		return(0);	// ignore message
	EndStats();
	CMainFrame::CStatusMsg	status(IDS_PSTR_WRITING_BITMAP);
	TRY {
		if (m_Engine.WritePoster(m_DestPath, m_BitCount, m_Resolution))
			EndJob();
		else
			FailJob(LDS(IDS_CANT_WRITE_BITMAP));
	}
	CATCH(CFileException, e)
	{
		FailJob(GetExceptionMsg(e));
	}
	END_CATCH
	DestroyWindow();	// we're modeless
	return(0);
}

LRESULT CPosterProgressDlg::OnAbortRender(WPARAM wParam, LPARAM lParam)
{
	LPTSTR	pMsg = (LPTSTR)lParam;
	if (wParam == m_FrameUID)	// if expected frame
		ReportError(pMsg);	// report error message
	delete pMsg;	// we're responsible for deleting error string
	DestroyWindow();	// we're modeless
	return(0);
}

void CPosterProgressDlg::OnTimer(W64UINT nIDEvent) 
{
	double	Elapsed = m_Clock.Elapsed();
	SetTime(m_ElapsedStat, round(Elapsed));	// display elapsed time
	int	RowsDone = m_Engine.GetStripRowsDone();
	if (RowsDone != m_RowsDone) {	// if rows done changed
		CString	s;
		s.Format(_T("%d (%d)"), RowsDone, m_FrameSize.cy);
		m_RowsDoneStat.SetWindowText(s);
		m_Progress.SetPos(RowsDone);	// update progress bar
		int	PctDone = round(RowsDone * 100.0 / m_FrameSize.cy);
		s.Format(_T("%d%%"), PctDone);
		m_PctDoneStat.SetWindowText(s);
		if (m_JobDlg->GetBatchMode())	// if running a batch job
			m_JobDlg->SetProgressPos(PctDone);	// update job dialog's bar too
		double	SampleLen = Elapsed - m_SampleStart;
		int	SampleRows = RowsDone - m_RowsDone;
		ASSERT(SampleRows);	// assuming RowsDone != m_RowsDone
		double	SecsPerRow = SampleLen / SampleRows;	// sample seconds per row
		m_AvgSPR.Update(float(SecsPerRow));	// update running average with sample
		float	AvgSPR = m_AvgSPR.GetAvg();	// average seconds per row
		int	Remaining = round((m_FrameSize.cy - RowsDone) * AvgSPR);
		SetTime(m_RemainingStat, Remaining);	// display remaining time
		float	PixelsPerSec = 1.0f / (AvgSPR * m_Snap.m_Antialias)
			* m_Snap.m_ImageSize.cx;	// inversely proportional to antialiasing
		s.Format(_T("%g"), PixelsPerSec);
		m_PixelRateStat.SetWindowText(s);	// display pixel rate
		m_RowsDone = RowsDone;
		m_SampleStart = Elapsed;
	}
}
