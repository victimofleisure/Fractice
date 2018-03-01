// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      17dec09	initial version

        select images dialog
 
*/

// SelectImagesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "SelectImagesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSelectImagesDlg dialog

IMPLEMENT_DYNAMIC(CSelectImagesDlg, CPersistDlg);

const CCtrlResize::CTRL_LIST CSelectImagesDlg::m_CtrlList[] = {
	{IDC_SELIMG_LIST,	BIND_ALL},
	{IDOK,				BIND_RIGHT | BIND_BOTTOM},
	{IDCANCEL,			BIND_RIGHT | BIND_BOTTOM},
	{0, 0}	// list terminator
};

CSelectImagesDlg::CSelectImagesDlg(CImageList& ImgList, LPCTSTR Caption, LPCTSTR RegKey, CWnd* pParent /*=NULL*/)
	: CPersistDlg(IDD, 0, RegKey, pParent), m_ImgList(ImgList), m_Caption(Caption)
{
	//{{AFX_DATA_INIT(CSelectImagesDlg)
	//}}AFX_DATA_INIT
	m_InitRect.SetRectEmpty();
}

void CSelectImagesDlg::DoDataExchange(CDataExchange* pDX)
{
	CPersistDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectImagesDlg)
	DDX_Control(pDX, IDC_SELIMG_LIST, m_List);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSelectImagesDlg, CPersistDlg)
	//{{AFX_MSG_MAP(CSelectImagesDlg)
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_NOTIFY(LVN_GETDISPINFO, IDC_SELIMG_LIST, OnGetdispinfoList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectImagesDlg message handlers

BOOL CSelectImagesDlg::OnInitDialog() 
{
	CPersistDlg::OnInitDialog();
	
	if (m_Caption != NULL)	// if custom caption
		SetWindowText(m_Caption);
	// init resizing stuff
	SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME), FALSE);
	GetWindowRect(m_InitRect);
	m_Resize.AddControlList(this, m_CtrlList);
	// init list control
	m_List.SetImageList(&m_ImgList, LVSIL_NORMAL);
	IMAGEINFO	info;
	if (m_ImgList.GetImageInfo(0, &info)) {
		CSize	szImg = CRect(info.rcImage).Size();
		// set spacing between icons; use smallest value that doesn't
		// cause unexpected behavior when navigating via cursor keys
		static const int MIN_GUTTER = 16;	// seems to be ok
		m_List.SetIconSpacing(szImg.cx + MIN_GUTTER, szImg.cy + MIN_GUTTER);
	}
	m_List.SetItemCountEx(m_ImgList.GetImageCount());

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSelectImagesDlg::OnOK() 
{
	if (m_List.GetSelectedCount()) {
		POSITION	pos = m_List.GetFirstSelectedItemPosition();
		while (pos != NULL)
			m_Selection.Add(m_List.GetNextSelectedItem(pos));
		CPersistDlg::OnOK();
	} else	// no items selected
		AfxMessageBox(IDS_SELECT_ITEMS);
}

void CSelectImagesDlg::OnSize(UINT nType, int cx, int cy) 
{
	CPersistDlg::OnSize(nType, cx, cy);
	m_Resize.OnSize();
}

void CSelectImagesDlg::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	if (!m_InitRect.IsRectNull()) {	// wait until rect is initialized
		lpMMI->ptMinTrackSize.x = m_InitRect.Width();
		lpMMI->ptMinTrackSize.y = m_InitRect.Height();
	}
}

void CSelectImagesDlg::OnGetdispinfoList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM	*pItem= &pDispInfo->item;
	if (pItem->mask & LVIF_TEXT) {
		// item text not implemented
	}
	if (pItem->mask & LVIF_IMAGE) {
		pItem->iImage = pItem->iItem;
	}
	*pResult = 0;
}
