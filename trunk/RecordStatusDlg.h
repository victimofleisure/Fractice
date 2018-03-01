// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		06jan10	W64: make OnTimer 64-bit compatible

        record status dialog
 
*/

#if !defined(AFX_RECORDSTATUSDLG_H__00913E36_FA6E_4984_91FB_F2B069EC893C__INCLUDED_)
#define AFX_RECORDSTATUSDLG_H__00913E36_FA6E_4984_91FB_F2B069EC893C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RecordStatusDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRecordStatusDlg dialog

#include "ToolDlg.h"
#include "RecordDlg.h"
#include "RunAvg.h"
#include "Benchmark.h"
#include "WndTimer.h"

class CFracticeView;
class CRecorder;

class CRecordStatusDlg : public CToolDlg
{
	DECLARE_DYNAMIC(CRecordStatusDlg);
// Construction
public:
	CRecordStatusDlg(CWnd* pParent = NULL);   // standard constructor

// Attributes
	void	SetView(CFracticeView *View);
	double	GetElapsedTime() const;

// Operations
	void	OnRecord();
	void	OnStop();
	void	Reset();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRecordStatusDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CRecordStatusDlg)
	enum { IDD = IDD_RECORD_STATUS };
	CStatic	m_PctDoneStat;
	CStatic	m_Rendering;
	CStatic	m_FrameRate;
	CButton	m_AbortBtn;
	CProgressCtrl	m_Progress;
	CStatic	m_Remaining;
	CStatic	m_Recorded;
	CStatic	m_Elapsed;
	CStatic	m_Duration;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CRecordStatusDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnAbort();
	afx_msg void OnTimer(W64UINT nIDEvent);
	afx_msg void OnDestroy();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Constants
	enum {
		RUNAVG_SIZE = 10,		// size of running average, in samples
		TIMER_ID = 1,			// timer identifier
		TIMER_PERIOD = 1000,	// timer period in milliseconds
	};

// Member data
	CFracticeView	*m_View;	// pointer to view
	CRecorder	*m_Recorder;	// pointer to recorder
	CRecordInfo	m_RecInfo;		// record info
	CWndTimer	m_Timer;		// timer instance
	CRunAvg<float>	m_RunAvgFR;	// running average of actual frame rate
	CBenchmark	m_Clock;		// performance counter
	UINT	m_PrevFramesDone;	// previous number of frames recorded
	int		m_PercentDone;		// percentage of recording completed
	bool	m_DirtyStats;		// true if statistics need updating

// Helpers
	void	TimerHook();
	void	UpdateCtrls();
	static	void	SetTime(CStatic& Ctrl, int Secs);
	static	void	SetTimeFrames(CStatic& Ctrl, int Secs, int Frames);
};

inline void CRecordStatusDlg::SetView(CFracticeView *View)
{
	m_View = View;
}

inline double CRecordStatusDlg::GetElapsedTime() const
{
	return(m_Clock.Elapsed());
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RECORDSTATUSDLG_H__00913E36_FA6E_4984_91FB_F2B069EC893C__INCLUDED_)
