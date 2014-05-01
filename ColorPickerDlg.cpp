// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
		00		17oct07	initial version
		01		08nov07	move 360 degree normalization into DColor object
		02		15nov07	only set color if edit control was modified
		03		29nov07	support Unicode
		04		05mar08	add IsDragging, send notification instead of app msg
		05		09mar08	don't clamp hue to range
		06		24dec08	support disabled state

		color picker dialog
 
*/

// ColorPickerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "ColorPickerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CColorPickerDlg dialog

IMPLEMENT_DYNAMIC(CColorPickerDlg, CDialog);

CColorPickerDlg::CColorPickerDlg(int ResID, CWnd* pParent)
	: CDialog(ResID, pParent)
{
	//{{AFX_DATA_INIT(CColorPickerDlg)
	//}}AFX_DATA_INIT
}

double CColorPickerDlg::GetEditVal(CEdit& Edit) const
{
	CString	s;
	Edit.GetWindowText(s);
	double	val = _tcstod(s, NULL);
	return(CLAMP(val, 0, 1));
}

void CColorPickerDlg::SetEditVal(CEdit& Edit, double Val) const
{
	CString	s;
	s.Format(_T("%.3f"), Val);
	Edit.SetWindowText(s);
}

void CColorPickerDlg::UpdateColor()
{
	DRGB	rgb;
	m_ColorPicker.GetColor(rgb);
	CString	s;
	SetEditVal(m_EditR, rgb.m_r);
	SetEditVal(m_EditG, rgb.m_g);
	SetEditVal(m_EditB, rgb.m_b);
	DHLS	hls;
	m_ColorPicker.GetColor(hls);
	SetEditVal(m_EditH, hls.m_h);
	SetEditVal(m_EditL, hls.m_l);
	SetEditVal(m_EditS, hls.m_s);
	m_Swatch.Invalidate();
}

void CColorPickerDlg::SetColor(const DRGB& Color)
{
	m_ColorPicker.SetColor(Color);
	if (m_hWnd)
		UpdateColor();
}

void CColorPickerDlg::SetColor(const DHLS& Color)
{
	m_ColorPicker.SetColor(Color);
	if (m_hWnd)
		UpdateColor();
}

void CColorPickerDlg::SetColor(COLORREF Color)
{
	DRGB	rgb(Color);
	SetColor(rgb);
}

void CColorPickerDlg::SetRGB(int Chan, double Val)
{
	DRGB	rgb;
	m_ColorPicker.GetColor(rgb);
	rgb[Chan] = Val;
	m_ColorPicker.SetColor(rgb);
	UpdateColor();
}

void CColorPickerDlg::SetHLS(int Chan, double Val)
{
	DHLS	hls;
	m_ColorPicker.GetColor(hls);
	hls[Chan] = Val;
	m_ColorPicker.SetColor(hls);
	UpdateColor();
}

bool CColorPickerDlg::IsDragging() const
{
	return(m_ColorPicker.IsDragging() || m_ShadePicker.IsDragging());
}

void CColorPickerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CColorPickerDlg)
	DDX_Control(pDX, IDC_CLPK_COLOR, m_ColorPicker);
	DDX_Control(pDX, IDC_CLPK_SHADE, m_ShadePicker);
	DDX_Control(pDX, IDC_CLPK_SWATCH, m_Swatch);
	DDX_Control(pDX, IDC_CLPK_EDIT_R, m_EditR);
	DDX_Control(pDX, IDC_CLPK_EDIT_G, m_EditG);
	DDX_Control(pDX, IDC_CLPK_EDIT_B, m_EditB);
	DDX_Control(pDX, IDC_CLPK_EDIT_H, m_EditH);
	DDX_Control(pDX, IDC_CLPK_EDIT_L, m_EditL);
	DDX_Control(pDX, IDC_CLPK_EDIT_S, m_EditS);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CColorPickerDlg, CDialog)
	//{{AFX_MSG_MAP(CColorPickerDlg)
	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
	ON_WM_LBUTTONDOWN()
	ON_BN_CLICKED(IDC_CLPK_COLOR, OnColorChange)
	ON_BN_CLICKED(IDC_CLPK_SHADE, OnColorChange)
	ON_EN_KILLFOCUS(IDC_CLPK_EDIT_R, OnKillFocusR)
	ON_EN_KILLFOCUS(IDC_CLPK_EDIT_G, OnKillFocusG)
	ON_EN_KILLFOCUS(IDC_CLPK_EDIT_B, OnKillFocusB)
	ON_EN_KILLFOCUS(IDC_CLPK_EDIT_H, OnKillFocusH)
	ON_EN_KILLFOCUS(IDC_CLPK_EDIT_L, OnKillFocusL)
	ON_EN_KILLFOCUS(IDC_CLPK_EDIT_S, OnKillFocusS)
	//}}AFX_MSG_MAP
	ON_NOTIFY(CPN_COLORPICK, IDC_CLPK_COLOR, OnColorPick)
	ON_NOTIFY(CPN_COLORPICK, IDC_CLPK_SHADE, OnColorPick)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CColorPickerDlg message handlers

BOOL CColorPickerDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_ColorPicker.SetBuddy(&m_ShadePicker);
	m_ShadePicker.SetBuddy(&m_ColorPicker);
	UpdateColor();
	m_ColorPicker.UpdateColor();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CColorPickerDlg::OnColorPick(NMHDR* pNMHDR, LRESULT* pResult)
{
	UpdateColor();
}

void CColorPickerDlg::OnColorChange() 
{
	UpdateData(TRUE);
}

HBRUSH CColorPickerDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	if (pWnd->m_hWnd == m_Swatch.m_hWnd // if control is m_Swatch
	&& m_ColorPicker.IsWindowEnabled()) {	// and picker is enabled	
		m_SwatchBrush.DeleteObject();
		DRGB	rgb;
		m_ColorPicker.GetColor(rgb);
		m_SwatchBrush.CreateSolidBrush(rgb);
		return m_SwatchBrush;
	}
	return hbr;
}

void CColorPickerDlg::OnPaint() 
{
	CPaintDC	dc(this);
	m_ShadePicker.UpdateMark();
}

void CColorPickerDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CPoint	pt(point);
	MapWindowPoints(&m_ShadePicker, &pt, 1);
	m_ShadePicker.DragTest(pt);
	CDialog::OnLButtonDown(nFlags, point);
}

void CColorPickerDlg::OnKillFocusR() 
{
	if (m_EditR.GetModify())
		SetRGB(DRGB::R, GetEditVal(m_EditR));
}

void CColorPickerDlg::OnKillFocusG() 
{
	if (m_EditG.GetModify())
		SetRGB(DRGB::G, GetEditVal(m_EditG));
}

void CColorPickerDlg::OnKillFocusB() 
{
	if (m_EditB.GetModify())
		SetRGB(DRGB::B, GetEditVal(m_EditB));
}

void CColorPickerDlg::OnKillFocusH() 
{
	if (m_EditH.GetModify()) {
		CString	s;
		m_EditH.GetWindowText(s);
		SetHLS(DHLS::H, _tcstod(s, NULL));	// don't clamp hue to range
	}
}

void CColorPickerDlg::OnKillFocusL() 
{
	if (m_EditL.GetModify())
		SetHLS(DHLS::L, GetEditVal(m_EditL));
}

void CColorPickerDlg::OnKillFocusS() 
{
	if (m_EditS.GetModify())
		SetHLS(DHLS::S, GetEditVal(m_EditS));
}
