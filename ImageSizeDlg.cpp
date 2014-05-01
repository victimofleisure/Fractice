// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version

        image size dialog
 
*/

// ImageSizeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "ImageSizeDlg.h"
#include "Snapshot.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CImageSizeDlg dialog

IMPLEMENT_DYNAMIC(CImageSizeDlg, CDialog);

CImageSizeDlg::CImageSizeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
	//{{AFX_DATA_INIT(CImageSizeDlg)
	//}}AFX_DATA_INIT
	m_Width = 0;
	m_Height = 0;
}

void CImageSizeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CImageSizeDlg)
	//}}AFX_DATA_MAP
	DDX_Text(pDX, IDC_IMG_SIZE_WIDTH_EDIT, m_Width);
	DDV_MinMaxInt(pDX, m_Width, MIN_IMAGE_WIDTH, MAX_IMAGE_WIDTH);
	DDX_Text(pDX, IDC_IMG_SIZE_HEIGHT_EDIT, m_Height);
	DDV_MinMaxInt(pDX, m_Height, MIN_IMAGE_HEIGHT, MAX_IMAGE_HEIGHT);
}

BEGIN_MESSAGE_MAP(CImageSizeDlg, CDialog)
	//{{AFX_MSG_MAP(CImageSizeDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImageSizeDlg message handlers

