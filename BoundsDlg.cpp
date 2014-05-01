// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version

        bounding rectangle dialog
 
*/

// BoundsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "BoundsDlg.h"
#include "BigPoint.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBoundsDlg dialog

IMPLEMENT_DYNAMIC(CBoundsDlg, CPersistDlg);

const CCtrlResize::CTRL_LIST CBoundsDlg::m_CtrlList[] = {
	{IDC_BOUNDS_X1,	BIND_LEFT | BIND_RIGHT},
	{IDC_BOUNDS_X2,	BIND_LEFT | BIND_RIGHT},
	{IDC_BOUNDS_Y1,	BIND_LEFT | BIND_RIGHT},
	{IDC_BOUNDS_Y2,	BIND_LEFT | BIND_RIGHT},
	{IDOK,			BIND_RIGHT},
	{IDCANCEL,		BIND_RIGHT},
	{0, 0}	// list terminator
};

CBoundsDlg::CBoundsDlg(CWnd* pParent /*=NULL*/)
	: CPersistDlg(IDD, NULL, _T("BoundsDlg"), pParent)
{
	//{{AFX_DATA_INIT(CBoundsDlg)
	m_x1 = _T("");
	m_x2 = _T("");
	m_y1 = _T("");
	m_y2 = _T("");
	//}}AFX_DATA_INIT
	m_InitRect.SetRectEmpty();
}

void CBoundsDlg::DoDataExchange(CDataExchange* pDX)
{
	CPersistDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBoundsDlg)
	DDX_Text(pDX, IDC_BOUNDS_X1, m_x1);
	DDX_Text(pDX, IDC_BOUNDS_X2, m_x2);
	DDX_Text(pDX, IDC_BOUNDS_Y1, m_y1);
	DDX_Text(pDX, IDC_BOUNDS_Y2, m_y2);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CBoundsDlg, CPersistDlg)
	//{{AFX_MSG_MAP(CBoundsDlg)
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBoundsDlg message handlers

BOOL CBoundsDlg::OnInitDialog() 
{
	CPersistDlg::OnInitDialog();
	
	GetWindowRect(m_InitRect);
	m_Resize.AddControlList(this, m_CtrlList);
	
	return TRUE;	// return TRUE unless you set the focus to a control
					// EXCEPTION: OCX Property Pages should return FALSE
}

void CBoundsDlg::OnOK() 
{
	UpdateData();
	BigPoint	p1, p2;
	p1.x.FromStr(m_x1);
	p1.y.FromStr(m_y1);
	p2.x.FromStr(m_x2);
	p2.y.FromStr(m_y2);
	if (p1.x == p2.x || p1.y == p2.y) {
		AfxMessageBox(IDS_BAD_COORDS);
		return;
	}
	CPersistDlg::OnOK();
}

void CBoundsDlg::OnSize(UINT nType, int cx, int cy) 
{
	CPersistDlg::OnSize(nType, cx, cy);
	m_Resize.OnSize();
}

void CBoundsDlg::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	if (!m_InitRect.IsRectNull()) {	// wait until rect is initialized
		lpMMI->ptMinTrackSize.x = m_InitRect.Width();
		lpMMI->ptMinTrackSize.y = m_InitRect.Height();
		lpMMI->ptMaxTrackSize.y = m_InitRect.Height();
	}
}
