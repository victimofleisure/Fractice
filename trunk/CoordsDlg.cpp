// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version

        viewer coordinates dialog
 
*/

// CoordsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "CoordsDlg.h"
#include "BigNum.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCoordsDlg dialog

IMPLEMENT_DYNAMIC(CCoordsDlg, CPersistDlg);

const CCtrlResize::CTRL_LIST CCoordsDlg::m_CtrlList[] = {
	{IDC_COORDS_X,	BIND_LEFT | BIND_RIGHT},
	{IDC_COORDS_Y,	BIND_LEFT | BIND_RIGHT},
	{IDC_COORDS_Z,	BIND_LEFT | BIND_RIGHT},
	{IDOK,			BIND_RIGHT},
	{IDCANCEL,		BIND_RIGHT},
	{0, 0}	// list terminator
};

CCoordsDlg::CCoordsDlg(CWnd* pParent /*=NULL*/)
	: CPersistDlg(IDD, NULL, _T("CoordsDlg"), pParent)
{
	//{{AFX_DATA_INIT(CCoordsDlg)
	m_x = _T("");
	m_y = _T("");
	m_z = _T("");
	//}}AFX_DATA_INIT
	m_InitRect.SetRectEmpty();
}


void CCoordsDlg::DoDataExchange(CDataExchange* pDX)
{
	CPersistDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCoordsDlg)
	DDX_Text(pDX, IDC_COORDS_X, m_x);
	DDX_Text(pDX, IDC_COORDS_Y, m_y);
	DDX_Text(pDX, IDC_COORDS_Z, m_z);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCoordsDlg, CPersistDlg)
	//{{AFX_MSG_MAP(CCoordsDlg)
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCoordsDlg message handlers

BOOL CCoordsDlg::OnInitDialog() 
{
	CPersistDlg::OnInitDialog();
	
	GetWindowRect(m_InitRect);
	m_Resize.AddControlList(this, m_CtrlList);
	
	return TRUE;	// return TRUE unless you set the focus to a control
					// EXCEPTION: OCX Property Pages should return FALSE
}

void CCoordsDlg::OnOK() 
{
	UpdateData();
	BigNum	n;
	n.FromStr(m_z);
	if (n <= 0) {
		AfxMessageBox(IDS_BAD_ZOOM);
		GotoDlgCtrl(GetDlgItem(IDC_COORDS_Z));
		return;
	}
	CPersistDlg::OnOK();
}

void CCoordsDlg::OnSize(UINT nType, int cx, int cy) 
{
	CPersistDlg::OnSize(nType, cx, cy);
	m_Resize.OnSize();
}

void CCoordsDlg::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	if (!m_InitRect.IsRectNull()) {	// wait until rect is initialized
		lpMMI->ptMinTrackSize.x = m_InitRect.Width();
		lpMMI->ptMinTrackSize.y = m_InitRect.Height();
		lpMMI->ptMaxTrackSize.y = m_InitRect.Height();
	}
}
