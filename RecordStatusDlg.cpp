// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		18dec09	use FastIsVisible
		02		25dec09	in TimerHook, test frame count instead of duration
		03		25dec09	in OnShowWindow, if dirty stats, update controls
		04		06jan10	W64: make OnTimer 64-bit compatible

        record status dialog
 
*/

// RecordStatusDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Fractice.h"
#include "RecordStatusDlg.h"
#include "RecordDlg.h"
#include "FracticeDoc.h"
#include "FracticeView.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRecordStatusDlg dialog

IMPLEMENT_DYNAMIC(CRecordStatusDlg, CToolDlg);

CRecordStatusDlg::CRecordStatusDlg(CWnd* pParent /*=NULL*/)
	: CToolDlg(IDD, IDR_MAINFRAME, _T("RecordStatusDlg"), pParent)
{
	//{{AFX_DATA_INIT(CRecordStatusDlg)
	//}}AFX_DATA_INIT
	m_View = NULL;
	m_Recorder = NULL;
	m_PrevFramesDone = 0;
	m_PercentDone = 0;
	m_DirtyStats = FALSE;
}

void CRecordStatusDlg::SetTime(CStatic& Ctrl, int Secs)
{
	CString	s;
	CRecordDlg::SecsToTime(Secs, s);
	Ctrl.SetWindowText(s);
}

void CRecordStatusDlg::SetTimeFrames(CStatic& Ctrl, int Secs, int Frames)
{
	CString	s, t;
	CRecordDlg::SecsToTime(Secs, t);
	s.Format(_T("%s (%d)"), t, Frames);
	Ctrl.SetWindowText(s);
}

void CRecordStatusDlg::Reset()
{
	m_Duration.SetWindowText(_T(""));
	m_Recorded.SetWindowText(_T(""));
	m_Remaining.SetWindowText(_T(""));
	m_Rendering.SetWindowText(_T(""));
	m_Elapsed.SetWindowText(_T(""));
	m_FrameRate.SetWindowText(_T(""));
	m_PctDoneStat.SetWindowText(_T(""));
	m_RunAvgFR.Reset();
	m_Progress.SetPos(0);
	m_PrevFramesDone = 0;
	m_PercentDone = 0;
	m_DirtyStats = FALSE;
}

void CRecordStatusDlg::OnRecord()
{
	Reset();
	m_Recorder->GetRecordInfo(m_RecInfo);
	m_View->SetLastRenderTime(0);
	SetTimeFrames(m_Duration, m_RecInfo.m_Duration, m_RecInfo.m_FrameCount);
	SetTime(m_Elapsed, 0);
	SetTimeFrames(m_Recorded, 0, 0);
	m_AbortBtn.EnableWindow(TRUE);
	m_AbortBtn.SetFocus();
	m_Timer.Create(m_hWnd, TIMER_ID, TIMER_PERIOD);
	m_Clock.Reset();
}

void CRecordStatusDlg::OnStop()
{
	TimerHook();	// set final values in dialog 
	m_AbortBtn.EnableWindow(FALSE);
	m_Timer.Destroy();
}

void CRecordStatusDlg::TimerHook()
{
	UINT	FramesDone = m_Recorder->GetFramesDone();
	if (m_RecInfo.m_FrameCount) {	// avoid potential divide by zero
		if (FramesDone != m_PrevFramesDone) {	// if done frames count changed
			m_PrevFramesDone = FramesDone;
			m_DirtyStats = TRUE;	// update statistics
			int	conns = m_Recorder->GetServersDlg().GetConnectionCount();
			if (conns) {
				double	SumFR = 0;
				for (int i = 0; i < conns; i++) {	// for each connected server
					CServer&	srv = m_Recorder->GetServersDlg().GetConnection(i);
					if (srv.GetPendingCount()) {	// skip idle servers
						double	t = srv.GetRenderTime();
						if (t)
							SumFR += 1 / t;
					}
				}
				double	t = m_View->GetLastRenderTime();
				if (t)
					SumFR += 1 / t;
				m_RunAvgFR.Update(static_cast<float>(SumFR));
			} else {
				double RenderTime = m_View->GetLastRenderTime();	// sample render time
				if (RenderTime) {	// avoid divide by zero
					float	SampFR = static_cast<float>(1 / RenderTime);
					m_RunAvgFR.Update(SampFR);	// update average frame rate
				}
			}
			m_PercentDone = round(FramesDone * 100.0 / m_RecInfo.m_FrameCount);
			if (m_Recorder->GetBatchMode())
				m_Recorder->GetJobControlDlg().SetProgressPos(m_PercentDone);
		}
	}
	if (FastIsVisible())
		UpdateCtrls();
}

void CRecordStatusDlg::UpdateCtrls()
{
	SetTime(m_Elapsed, round(m_Clock.Elapsed()));
	if (m_DirtyStats) {	// if statistics need updating
		m_DirtyStats = FALSE;
		SetTime(m_Rendering, round(m_Recorder->GetTotalRenderTime()));
		UINT	FramesDone = m_Recorder->GetFramesDone();
		int		FramesRemain = m_RecInfo.m_FrameCount - FramesDone;
		float	AvgFR = m_RunAvgFR.GetAvg();	// get average frame rate
		SetTime(m_Remaining, round(AvgFR ? FramesRemain / AvgFR : 0));
		CString	s;
		s.Format(_T("%.3f"), AvgFR);
		m_FrameRate.SetWindowText(s);
		int	RecTime = round(FramesDone / m_RecInfo.m_FrameRate);
		SetTimeFrames(m_Recorded, RecTime, FramesDone);
		m_Progress.SetPos(m_PercentDone);
		s.Format(_T("%d%%"), m_PercentDone);
		m_PctDoneStat.SetWindowText(s);
	}
}

void CRecordStatusDlg::DoDataExchange(CDataExchange* pDX)
{
	CToolDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRecordStatusDlg)
	DDX_Control(pDX, IDC_RST_PCT_DONE, m_PctDoneStat);
	DDX_Control(pDX, IDC_RST_RENDERING, m_Rendering);
	DDX_Control(pDX, IDC_RST_FRAME_RATE, m_FrameRate);
	DDX_Control(pDX, IDC_RST_ABORT, m_AbortBtn);
	DDX_Control(pDX, IDC_RST_PROGRESS, m_Progress);
	DDX_Control(pDX, IDC_RST_REMAINING, m_Remaining);
	DDX_Control(pDX, IDC_RST_RECORDED, m_Recorded);
	DDX_Control(pDX, IDC_RST_ELAPSED, m_Elapsed);
	DDX_Control(pDX, IDC_RST_DURATION, m_Duration);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CRecordStatusDlg, CToolDlg)
	//{{AFX_MSG_MAP(CRecordStatusDlg)
	ON_BN_CLICKED(IDC_RST_ABORT, OnAbort)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRecordStatusDlg message handlers

BOOL CRecordStatusDlg::OnInitDialog() 
{
	CToolDlg::OnInitDialog();
	Reset();
	m_RunAvgFR.Create(RUNAVG_SIZE);
	m_Recorder = &theApp.GetMain()->GetRecorder();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CRecordStatusDlg::OnDestroy() 
{
	m_Timer.Destroy();
	CToolDlg::OnDestroy();
}

void CRecordStatusDlg::OnTimer(W64UINT nIDEvent) 
{
	TimerHook();
}

void CRecordStatusDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CToolDlg::OnShowWindow(bShow, nStatus);
	if (bShow && m_Recorder != NULL && (m_Recorder->IsRecording() || m_DirtyStats))
		UpdateCtrls();
}

void CRecordStatusDlg::OnAbort() 
{
	m_Recorder->StopCheck();
}
