// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      11jan09	initial version

        image size/resolution dialog with unit conversion
 
*/

// ImageSizeResDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "ImageSizeResDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CImageSizeResDlg dialog

IMPLEMENT_DYNAMIC(CImageSizeResDlg, CDialog);

const float CImageSizeResDlg::CM_PER_INCH = 2.54f;

CImageSizeResDlg::CImageSizeResDlg(UINT nIDTemplate, CWnd* pParent /*=NULL*/)
	: CDialog(nIDTemplate, pParent)
{
	//{{AFX_DATA_INIT(CImageSizeResDlg)
	//}}AFX_DATA_INIT
	m_Canceled = FALSE;
}

CImageSizeResDlg::~CImageSizeResDlg()
{
}

float CImageSizeResDlg::NormRes(float Val, int ResUnit)
{
	// convert resolution from its current unit to pixels per inch
	switch (ResUnit) {
	case RU_PIXELS_INCH:
		break;	// nothing to do
	case RU_PIXELS_CM:
		Val *= CM_PER_INCH;
		break;
	default:
		ASSERT(0);	// invalid resolution unit
	}
	return(Val);
}

float CImageSizeResDlg::DenormRes(float Val, int ResUnit)
{
	// convert resolution from pixels per inch to its current unit
	switch (ResUnit) {
	case RU_PIXELS_INCH:
		break;	// nothing to do
	case RU_PIXELS_CM:
		Val /= CM_PER_INCH;
		break;
	default:
		ASSERT(0);	// invalid resolution unit
	}
	return(Val);
}

float CImageSizeResDlg::Norm(float Val, int SizeUnit, float DPI)
{
	// convert size from its current unit to pixels
	switch (SizeUnit) {
	case SU_PIXELS:
		break;	// nothing to do
	case SU_INCHES:
		Val *= DPI;
		break;
	case SU_CM:
		Val = Val / CM_PER_INCH * DPI;
		break;
	default:
		ASSERT(0);	// invalid size unit
	}
	return(Val);
}

float CImageSizeResDlg::Denorm(float Val, int SizeUnit, float DPI)
{
	// convert size from pixels to its current unit
	switch (SizeUnit) {
	case SU_PIXELS:
		break;	// nothing to do
	case SU_INCHES:
		Val /= DPI;
		break;
	case SU_CM:
		Val = Val / DPI * CM_PER_INCH;
		break;
	default:
		ASSERT(0);	// invalid size unit
	}
	return(Val);
}

void CImageSizeResDlg::UpdateImgSize()
{
	float	DPI = NormRes(m_Res, m_ResUnit);
	m_ImageSize.cx = round(Norm(m_Width, m_WidthUnit, DPI));
	m_ImageSize.cy = round(Norm(m_Height, m_HeightUnit, DPI));
}

void CImageSizeResDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CImageSizeResDlg)
	//}}AFX_DATA_MAP
	if (m_WidthUnit == SU_PIXELS)	// if width is in pixels
		DDX_Text(pDX, IDC_IMSR_WIDTH_EDIT, m_ImageSize.cx);	// get integer
	else	// width not in pixels
		DDX_Text(pDX, IDC_IMSR_WIDTH_EDIT, m_Width);	// get float
	if (m_HeightUnit == SU_PIXELS)	// if height is in pixels
		DDX_Text(pDX, IDC_IMSR_HEIGHT_EDIT, m_ImageSize.cy);	// get integer
	else	// height not in pixels
		DDX_Text(pDX, IDC_IMSR_HEIGHT_EDIT, m_Height);	// get float
	DDX_Text(pDX, IDC_IMSR_RESOLUTION_EDIT, m_Res);
	DDX_CBIndex(pDX, IDC_IMSR_WIDTH_UNIT, m_WidthUnit);
	DDX_CBIndex(pDX, IDC_IMSR_HEIGHT_UNIT, m_HeightUnit);
	DDX_CBIndex(pDX, IDC_IMSR_RESOLUTION_UNIT, m_ResUnit);
}

BEGIN_MESSAGE_MAP(CImageSizeResDlg, CDialog)
	//{{AFX_MSG_MAP(CImageSizeResDlg)
	ON_CBN_SELCHANGE(IDC_IMSR_RESOLUTION_UNIT, OnSelChangeResUnit)
	ON_CBN_SELCHANGE(IDC_IMSR_HEIGHT_UNIT, OnSelChangeSizeUnit)
	ON_CBN_SELCHANGE(IDC_IMSR_WIDTH_UNIT, OnSelChangeSizeUnit)
	ON_EN_KILLFOCUS(IDC_IMSR_WIDTH_EDIT, OnKillfocusWidthEdit)
	ON_EN_KILLFOCUS(IDC_IMSR_HEIGHT_EDIT, OnKillfocusHeightEdit)
	ON_EN_KILLFOCUS(IDC_IMSR_RESOLUTION_EDIT, OnKillfocusResolutionEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImageSizeResDlg message handlers

BOOL CImageSizeResDlg::OnInitDialog() 
{
	m_Canceled = FALSE;
	m_PrevInfo = *this;	// save state in case user cancels
	UpdateImgSize();
	CDialog::OnInitDialog();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CImageSizeResDlg::OnOK() 
{
	CDialog::OnOK();
	UpdateImgSize();
}

void CImageSizeResDlg::OnCancel() 
{
	m_Canceled = TRUE;	// prevent kill focus handlers from validating data
	CDialog::OnCancel();
	IMAGE_SIZE_RES_INFO&	info = *this;
	info = m_PrevInfo;	// restore previous state
}

void CImageSizeResDlg::OnSelChangeSizeUnit()
{
	float	DPI = NormRes(m_Res, m_ResUnit);	// convert resolution to DPI
	float	w = Norm(m_Width, m_WidthUnit, DPI);	// convert width to pixels
	float	h = Norm(m_Height, m_HeightUnit, DPI);	// convert height to pixels
	UpdateData();	// retrieve data from controls
	m_ImageSize = CSize(round(w), round(h));
	m_Width = Denorm(w, m_WidthUnit, DPI);	// convert width to selected unit
	m_Height = Denorm(h, m_HeightUnit, DPI);	// convert height to selected unit
	UpdateData(FALSE);	// update controls from data
}

void CImageSizeResDlg::OnSelChangeResUnit()
{
	float	DPI = NormRes(m_Res, m_ResUnit);	// convert resolution to DPI
	UpdateData();	// retrieve data from controls
	m_Res = DenormRes(DPI, m_ResUnit);	// convert DPI to selected unit
	UpdateData(FALSE);	// update controls from data
}

void CImageSizeResDlg::OnKillfocusWidthEdit() 
{
	if (!m_Canceled) {
		UpdateData();	// retrieve data from controls
		if (m_WidthUnit == SU_PIXELS)	// if new width is in pixels
			m_Width = float(m_ImageSize.cx);	// update real width from integer width
	}
}

void CImageSizeResDlg::OnKillfocusHeightEdit() 
{
	if (!m_Canceled) {
		UpdateData();	// retrieve data from controls
		if (m_HeightUnit == SU_PIXELS)	// if new height is in pixels
			m_Height = float(m_ImageSize.cy);	// update real height from integer height
	}
}

void CImageSizeResDlg::OnKillfocusResolutionEdit() 
{
	if (!m_Canceled)
		UpdateData();	// retrieve data from controls
}
