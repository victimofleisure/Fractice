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

        poster dialog
 
*/

// PosterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Fractice.h"
#include "PosterDlg.h"
#include "Params.h"
#include "MultiFileDlg.h"
#include "PosterProgressDlg.h"
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
// CPosterDlg dialog

IMPLEMENT_DYNAMIC(CPosterDlg, CImageSizeResDlg);

#define RK_PSTR_WIDTH		_T("PosterWidth")
#define RK_PSTR_WIDTH_UNIT	_T("PosterWidthUnit")
#define RK_PSTR_HEIGHT		_T("PosterHeight")
#define RK_PSTR_HEIGHT_UNIT	_T("PosterHeightUnit")
#define RK_PSTR_RES			_T("PosterRes")
#define RK_PSTR_RES_UNIT	_T("PosterResUnit")
#define RK_PSTR_ANTIALIAS	_T("PosterAntialias")
#define RK_PSTR_QUEUE_JOB	_T("PosterQueueJob")

CPosterDlg::CPosterDlg(CWnd* pParent /*=NULL*/)
	: CImageSizeResDlg(IDD, pParent)
{
	//{{AFX_DATA_INIT(CPosterDlg)
	//}}AFX_DATA_INIT
	m_Width			= theApp.RdRegFloat(RK_PSTR_WIDTH, 3200);
	m_WidthUnit		= theApp.RdRegInt(RK_PSTR_WIDTH_UNIT, 0);
	m_Height		= theApp.RdRegFloat(RK_PSTR_HEIGHT, 2400);
	m_HeightUnit	= theApp.RdRegInt(RK_PSTR_HEIGHT_UNIT, 0);
	m_Res			= theApp.RdRegFloat(RK_PSTR_RES, 300);
	m_ResUnit		= theApp.RdRegInt(RK_PSTR_RES_UNIT, 0);
	m_Antialias		= theApp.RdRegInt(RK_PSTR_ANTIALIAS, 3);
	m_QueueJob		= theApp.RdRegInt(RK_PSTR_QUEUE_JOB, 0);
	m_BitCount = CRecordInfo::DEF_BIT_COUNT;
}

CPosterDlg::~CPosterDlg()
{
	theApp.WrRegFloat(RK_PSTR_WIDTH, m_Width);
	theApp.WrRegInt(RK_PSTR_WIDTH_UNIT, m_WidthUnit);
	theApp.WrRegFloat(RK_PSTR_HEIGHT, m_Height);
	theApp.WrRegInt(RK_PSTR_HEIGHT_UNIT, m_HeightUnit);
	theApp.WrRegFloat(RK_PSTR_RES, m_Res);
	theApp.WrRegInt(RK_PSTR_RES_UNIT, m_ResUnit);
	theApp.WrRegInt(RK_PSTR_ANTIALIAS, m_Antialias);
	theApp.WrRegInt(RK_PSTR_QUEUE_JOB, m_QueueJob);
}

bool CPosterDlg::RenderPoster(LPCTSTR Path)
{
	CPosterProgressDlg	*PstrProgDlg = new CPosterProgressDlg;
	if (!PstrProgDlg->Create(m_Snap, Path, m_BitCount, GetDPI()))
		return(FALSE);
	return(TRUE);
}

void CPosterDlg::QueuePoster(LPCTSTR Path)
{
	CJobInfo	info;
	info.m_RecInfo.m_Snap.SetSize(1);	// single snapshot indicates poster job
	info.m_RecInfo.m_Snap[0] = m_Snap;
	info.m_RecInfo.m_FrameRate = GetDPI();	// repurpose frame rate for DPI
	info.m_RecInfo.m_BitCount = m_BitCount;
	info.m_RecordPath = Path;
	info.m_Dest = PathFindFileName(Path);
	CFracticeView	*View = theApp.GetMain()->GetView();
	if (View != NULL)
		info.m_Source = View->GetDocument()->GetTitle();
	theApp.GetMain()->GetJobControlDlg().Add(info);
}

void CPosterDlg::DoDataExchange(CDataExchange* pDX)
{
	CImageSizeResDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPosterDlg)
	DDX_Control(pDX, IDC_PSTR_SNAP_PIC, m_SnapPic);
	DDX_Check(pDX, IDC_PSTR_QUEUE_JOB, m_QueueJob);
	//}}AFX_DATA_MAP
	DDX_Text(pDX, IDC_PSTR_ANTIALIAS_EDIT, m_Antialias);
	DDV_MinMaxInt(pDX, m_Antialias, MIN_ANTIALIAS, MAX_ANTIALIAS);
}

BEGIN_MESSAGE_MAP(CPosterDlg, CImageSizeResDlg)
	//{{AFX_MSG_MAP(CPosterDlg)
	ON_BN_CLICKED(IDC_PSTR_RESUME, OnResume)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPosterDlg message handlers

BOOL CPosterDlg::OnInitDialog() 
{
	m_PrevAntialias = m_Antialias;
	CImageSizeResDlg::OnInitDialog();
	if (m_Snap.m_Thumb != NULL)
		m_SnapPic.SetBitmap(*m_Snap.m_Thumb, &m_Snap.m_ThumbFrame);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPosterDlg::OnOK()
{
	// keep dialog open while prompting for filename, so user can verify settings
	if (UpdateData()) {	// emulate base class OnOK
		UpdateImgSize();
		m_Snap.m_ImageSize = m_ImageSize;	// override snapshot's size
		m_Snap.m_Antialias = m_Antialias;	// and antialiasing factor
		CMultiFileDlg	fd(FALSE, BITMAP_EXT, NULL, OFN_OVERWRITEPROMPT,
			LDS(IDS_BMP_FILTER), NULL, NULL, &theApp.m_ExportFolder);
		if (fd.DoModal() == IDOK) {
			EndDialog(IDOK);
			if (m_QueueJob)
				QueuePoster(fd.GetPathName());
			else
				RenderPoster(fd.GetPathName());
		}
	}
}

void CPosterDlg::OnCancel() 
{
	CImageSizeResDlg::OnCancel();
	m_Antialias = m_PrevAntialias;
}

void CPosterDlg::OnResume() 
{
	CMultiFileDlg	fd(TRUE, ROW_INDEX_EXT, NULL, OFN_HIDEREADONLY,
		LDS(IDS_PSTR_PAUSED_FILTER), NULL, NULL, &theApp.m_ExportFolder);
	if (fd.DoModal() == IDOK) {
		EndDialog(IDOK);
		CPathStr	path(fd.GetPathName());
		path.RenameExtension(BITMAP_EXT);
		RenderPoster(path);
	}
}
