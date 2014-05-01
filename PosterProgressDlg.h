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
		02		06jan10	W64: make OnTimer 64-bit compatible

        poster progress dialog
 
*/

#if !defined(AFX_POSTERPROGRESSDLG_H__1B16FC8C_CDB8_439A_B889_F3EC7F39DAED__INCLUDED_)
#define AFX_POSTERPROGRESSDLG_H__1B16FC8C_CDB8_439A_B889_F3EC7F39DAED__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PosterProgressDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPosterProgressDlg dialog

#include "PosterEngine.h"
#include "WndTimer.h"
#include "Snapshot.h"
#include "Benchmark.h"
#include "RunAvg.h"

class CJobControlDlg;

class CPosterProgressDlg : public CDialog
{
	DECLARE_DYNAMIC(CPosterProgressDlg);
// Construction
public:
	CPosterProgressDlg(CWnd* pParent = NULL);
	BOOL	Create(const CSnapshot& Snap, LPCTSTR Path, WORD BitCount, float Resolution);

// Attributes
	static	CPosterProgressDlg	*GetThis();
	CPosterEngine&	GetEngine();

// Operations
	bool	StopCheck();
	bool	PauseCheck();
	bool	Pause();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPosterProgressDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CPosterProgressDlg)
	enum { IDD = IDD_POSTER_PROGRESS };
	CStatic	m_PctDoneStat;
	CStatic	m_PixelRateStat;
	CStatic	m_RemainingStat;
	CStatic	m_ImageSizeStat;
	CStatic	m_FilenameStat;
	CStatic	m_ElapsedStat;
	CStatic	m_RowsDoneStat;
	CProgressCtrl	m_Progress;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CPosterProgressDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(W64UINT nIDEvent);
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnClose();
	afx_msg void OnPause();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	//}}AFX_MSG
	afx_msg LRESULT	OnRenderDone(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnStripDone(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnAbortRender(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

// Types
	class CPausedPosterInfo : WObject {
	public:
	// Public data
		CSnapshot	m_Snap;
		CDWordArray	m_RowIndex;

	// Constants
		enum {
			ARCHIVE_VERSION = 1		// archive version 
		};

	// Operations
		void	Serialize(CArchive& ar);
	};

// Constants
	enum {
		RUNAVG_SIZE = 10,		// size of running average, in samples
		TIMER_ID = 1,			// progress timer identifier
		TIMER_PERIOD = 1000,	// progress timer period
	};

// Data members
	static	CPosterProgressDlg	*m_This;	// our global instance
	CMainFrame	*m_Main;		// pointer to main frame
	CJobControlDlg	*m_JobDlg;	// pointer to job control dialog
	CPosterEngine	m_Engine;	// poster rendering engine
	CSnapshot	m_Snap;			// snapshot to be rendered
	CString		m_DestPath;		// path of destination file
	CSize	m_FrameSize;		// frame size in pixels
	WORD	m_BitCount;			// bitmap color depth, in bits
	float	m_Resolution;		// poster resolution, in DPI
	UINT	m_FrameUID;			// frame unique identifier
	int		m_RowsDone;			// number of rows completed so far
	CWndTimer	m_Timer;		// timer for progress bar
	bool	m_Running;			// true if job is in progress
	bool	m_BatchMode;		// true if running a batch job
	CBenchmark	m_Clock;		// performance counter for elapsed time
	CRunAvg<float>	m_AvgSPR;	// running average of seconds per row
	double	m_SampleStart;		// elapsed time at start of sample

// Helpers
	static	CString	GetExceptionMsg(CException *e);
	void	ReportError(LPCTSTR Msg);
	void	ReportError(UINT StrID);
	void	ReportException(CException *e);
	bool	StartJob();
	void	EndJob();
	void	FailJob(LPCTSTR Msg);
	void	AbortJob();
	void	OnJobDone();
	static	void	SetTime(CStatic& Ctrl, int Secs);
	void	InitStats();
	void	EndStats();
	bool	SerializePausedJob(bool Store, CPausedPosterInfo& Info);
};

inline CPosterProgressDlg *CPosterProgressDlg::GetThis()
{
	return(m_This);
}

inline CPosterEngine& CPosterProgressDlg::GetEngine()
{
	return(m_Engine);
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_POSTERPROGRESSDLG_H__1B16FC8C_CDB8_439A_B889_F3EC7F39DAED__INCLUDED_)
