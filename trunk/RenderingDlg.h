// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      24feb09	initial version
		01		26feb09	add export
		02		13mar09	improve render avoidance
		03		06jan10	W64: in GetJobCount, cast job array size to 32-bit

		rendering progress dialog
 
*/

#if !defined(AFX_RENDERINGDLG_H__371804AF_56CF_433C_BCFD_C543787BC927__INCLUDED_)
#define AFX_RENDERINGDLG_H__371804AF_56CF_433C_BCFD_C543787BC927__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RenderingDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRenderingDlg dialog

class CFracticeView;
class CHistoryView;

class CRenderingDlg : public CDialog
{
	DECLARE_DYNAMIC(CRenderingDlg);
// Construction
public:
	CRenderingDlg(UINT Flags = 0, CWnd* pParent = NULL);

// Constants
	enum {	// constructor flags
		CF_EXPORT_BITMAPS	= 0x01,		// export bitmaps
		CF_CREATE_THUMBS	= 0x02,		// create thumbnails
	};

// Attributes
	void	SetExportInfo(LPCTSTR Folder, LPCTSTR Prefix);
	void	SetCaption(LPCTSTR Caption);
	bool	Exporting() const;
	int		GetJobCount() const;
	int		GetRenderCount() const;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRenderingDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CRenderingDlg)
	enum { IDD = IDD_RENDERING };
	CProgressCtrl	m_Progress;
	CStatic	m_Status;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CRenderingDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	virtual void OnCancel();
	virtual void OnOK();
	//}}AFX_MSG
	afx_msg LRESULT	OnStartNextJob(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnRenderDone(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnStripDone(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

// Constants
	enum {
		UWM_STARTNEXTJOB = WM_APP + 1000
	};

// Member data
	UINT	m_Flags;			// flags specified in ctor; see flags enum
	CFracticeView	*m_View;	// pointer to current view
	CHistoryView	*m_Hist;	// pointer to current history view
	CString	m_Caption;			// optional dialog caption
	HWND	m_PrevEngineWnd;	// backup of engine's target window
	CDWordArray	m_JobList;		// history indices of items to be rendered
	int		m_CurJob;			// index of our current position in job list
	CString	m_ExportFolder;		// if we're exporting, destination folder path
	CString	m_ExportPrefix;		// if we're exporting, export filename prefix
	bool	m_CacheImages;		// true if we're caching images
	bool	m_SaveEscTimes;		// true if we're saving escape times
	int		m_RenderCount;		// number of jobs that need rendering

// Helpers
	bool	BuildJobList();
	bool	StartNextJob();
	bool	FinishJob();
};

inline void CRenderingDlg::SetCaption(LPCTSTR Caption)
{
	m_Caption = Caption;
}

inline bool CRenderingDlg::Exporting() const
{
	return(m_Flags & CF_EXPORT_BITMAPS);
}

inline int CRenderingDlg::GetJobCount() const
{
	return(INT64TO32(m_JobList.GetSize()));
}

inline int CRenderingDlg::GetRenderCount() const
{
	return(m_RenderCount);
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RENDERINGDLG_H__371804AF_56CF_433C_BCFD_C543787BC927__INCLUDED_)
