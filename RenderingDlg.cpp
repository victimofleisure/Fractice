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
		03		06jan10	W64: in BuildJobList, use GetJobCount

		rendering progress dialog
 
*/

// RenderingDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Fractice.h"
#include "RenderingDlg.h"
#include "MainFrm.h"
#include "FracticeDoc.h"
#include "FracticeView.h"
#include "PathStr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRenderingDlg dialog

IMPLEMENT_DYNAMIC(CRenderingDlg, CDialog);

CRenderingDlg::CRenderingDlg(UINT Flags, CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
	//{{AFX_DATA_INIT(CRenderingDlg)
	//}}AFX_DATA_INIT
	m_Flags = Flags;
	m_View = NULL;
	m_Hist = NULL;
	m_PrevEngineWnd = NULL;
	m_CurJob = 0;
	m_CacheImages = FALSE;
	m_SaveEscTimes = FALSE;
	m_RenderCount = 0;
}

void CRenderingDlg::SetExportInfo(LPCTSTR Folder, LPCTSTR Prefix)
{
	m_ExportFolder = Folder;
	m_ExportPrefix = Prefix;
}

bool CRenderingDlg::BuildJobList()
{
	m_View = theApp.GetMain()->GetView();
	if (m_View == NULL)
		return(FALSE);
	COptionsDlg&	opts = theApp.GetMain()->GetOptionsDlg();
	m_CacheImages = opts.GetCacheImages();
	m_SaveEscTimes = opts.GetSaveEscTimes();
	m_RenderCount = 0;
	m_Hist = m_View->GetHistoryView();
	m_Hist->GetSelection(m_JobList);
	int	jobs = GetJobCount();
	for (int i = 0; i < jobs; i++) {
		CSnapshot&	snap = m_Hist->GetItem(m_JobList[i]);
		if (!snap.HasImage() || (!snap.HasEscTimes() && m_SaveEscTimes))
			m_RenderCount++;	// count item as a render
	}
	if (!Exporting()) {	// if not exporting
		if (!m_RenderCount)	// if no items need rendering
			return(TRUE);	// nothing to do
		if (!m_CacheImages) {	// if not caching images
			// multiple render is pointless since images wouldn't be saved
			AfxMessageBox(IDS_REND_CANT_MULTI_RENDER);
			return(FALSE);
		}
	}
	return(TRUE);
}

bool CRenderingDlg::FinishJob()
{
	if (Exporting()) {
		CString	s;
		s.Format(_T("%08d.bmp"), m_CurJob);
		CPathStr	Path(m_ExportFolder);
		Path.Append(m_ExportPrefix + s);
		bool	CanExport = TRUE;
		if (PathFileExists(Path)) {	// if file already exists
			CString	msg;
			AfxFormatString1(msg, IDS_FILE_OVERWRITE_PROMPT, Path);
			int	retc = AfxMessageBox(msg, MB_YESNOCANCEL | MB_DEFBUTTON2);
			if (retc == IDCANCEL) {	// if user canceled at overwrite prompt
				OnCancel();	// cancel dialog
				return(FALSE);
			}
			CanExport = (retc == IDYES);
		}
		if (CanExport) {	// if OK to export
			if (!m_View->ExportBitmap(Path)) {	// if export failed
				EndDialog(IDABORT);	// end dialog and return abort
				return(FALSE);	// error was already handled
			}
		}
	}
	if (m_Flags & CF_CREATE_THUMBS) {
		CSnapshot&	Snap = m_Hist->GetItem(m_JobList[m_CurJob]);
		Snap.m_Thumb->DeleteObject();
		m_View->CreateThumb(Snap.m_Thumb, Snap.m_ThumbFrame);
	}
	m_CurJob++;
	m_Progress.SetPos(m_CurJob);	// update progress bar
	return(TRUE);	// job is finished
}

bool CRenderingDlg::StartNextJob()
{
	while (m_CurJob < GetJobCount()) {
		CString	s;
		s.Format(IDS_REND_STATUS, m_CurJob, GetJobCount(), 
			round(m_CurJob * 100.0 / GetJobCount()));
		m_Status.SetWindowText(s);	// update status text
		CSnapshot&	Snap = m_Hist->GetItem(m_JobList[m_CurJob]);
		if (m_CurJob > 0) {
			// if previous item has escape times and differs from this item in
			// color only, share previous item's escape times, avoiding render
			CSnapshot&	PrevSnap = m_Hist->GetItem(m_JobList[m_CurJob - 1]);
			if (PrevSnap.HasEscTimes() && Snap.EqualIgnoreColor(PrevSnap))
				Snap.m_EscTimes = PrevSnap.m_EscTimes;	// add a reference
		}
		if (Exporting()) {	// if exporting bitmaps
			m_View->SetSnapshotCached(Snap, TRUE);	// render; allow color remap
		} else {	// not exporting
			if (m_SaveEscTimes && !Snap.HasEscTimes())	// if escape times needed
				Snap.m_Image.SetEmpty();	// delete image to force render
			m_View->SetSnapshotCached(Snap, TRUE);	// render; allow color remap
		}
		if (!m_View->IsIdle())	// if engine is busy doing whatever
			return(TRUE);	// engine will send us UWM_RENDERDONE when it's done
		// engine is idle; assume image and/or escape times were cached
		if (!FinishJob())	// if error finishing job
			return(FALSE);	// abort; assume dialog is already ended
	}
	OnOK();	// no more jobs; success
	return(TRUE);
}

void CRenderingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRenderingDlg)
	DDX_Control(pDX, IDC_PROGRESS, m_Progress);
	DDX_Control(pDX, IDC_REND_STATUS, m_Status);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CRenderingDlg, CDialog)
	//{{AFX_MSG_MAP(CRenderingDlg)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_MESSAGE(UWM_STARTNEXTJOB, OnStartNextJob)
	ON_MESSAGE(UWM_RENDERDONE, OnRenderDone)
	ON_MESSAGE(UWM_STRIPDONE, OnStripDone)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRenderingDlg message handlers

BOOL CRenderingDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	if (!m_Caption.IsEmpty())
		SetWindowText(m_Caption);
	if (BuildJobList()) {	// if job list builds OK
		if (m_RenderCount || Exporting()) {	// if we have work to do
			m_Progress.SetRange32(0, GetJobCount());	// set progress bar range
			// intercept all messages sent by the view's engine; our handlers
			// will forward the messages to the view
			m_PrevEngineWnd = m_View->GetEngine().GetNotifyWnd();
			m_View->GetEngine().SetNotifyWnd(m_hWnd);
			// we don't call StartNextJob directly, because if all the images
			// are cached, StartNextJob finishes all the jobs and calls OnOK
			// before returning, in which case we'll never have been visible;
			// post a message instead, and call StartNextJob from the handler
			PostMessage(UWM_STARTNEXTJOB);
		} else	// job list is empty
			OnOK();	// nothing to do
	} else	// error building job list
		EndDialog(IDABORT);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CRenderingDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	if (m_View != NULL && m_PrevEngineWnd != NULL)
		m_View->GetEngine().SetNotifyWnd(m_PrevEngineWnd);
}

void CRenderingDlg::OnOK() 
{
	if (m_Hist != NULL) {
		// set view position to item at selection mark
		m_Hist->SetViewPos(-1);	// spoof no-op test in SetViewPos
		int	pos = m_Hist->GetSelectionMark();
		m_Hist->RenderItem(pos, CHistoryView::RF_UNDOABLE);
	}
	CDialog::OnOK();
}

void CRenderingDlg::OnCancel() 
{
	if (m_View != NULL)
		m_View->CancelRender();	
	CDialog::OnCancel();
}

LRESULT CRenderingDlg::OnStartNextJob(WPARAM wParam, LPARAM lParam)
{
	StartNextJob();
	return(0);
}

LRESULT CRenderingDlg::OnRenderDone(WPARAM wParam, LPARAM lParam)
{
	m_View->SendMessage(UWM_RENDERDONE, wParam, lParam);	// forward message to view
	if (FinishJob())	// if job finished OK
		StartNextJob();	// start next one
	return(0);
}

LRESULT CRenderingDlg::OnStripDone(WPARAM wParam, LPARAM lParam)
{
	m_View->SendMessage(UWM_STRIPDONE, wParam, lParam);	// forward message to view
	return(0);
}
