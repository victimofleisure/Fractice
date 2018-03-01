// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      02feb09	initial version
		01		03feb09	coalesce edits
		02		23mar09	add view argument to normalized accessors

        color mapping dialog
 
*/

// ColorMappingDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Fractice.h"
#include "ColorMappingDlg.h"
#include "MainFrm.h"
#include "FracticeDoc.h"
#include "FracticeView.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CColorMappingDlg dialog

IMPLEMENT_DYNAMIC(CColorMappingDlg, CChildDlg);

const double CColorMappingDlg::CYCLE_LEN_BASE = 10;
const double CColorMappingDlg::CYCLE_LEN_QUAL_RATIO = 128;

const CCtrlResize::CTRL_LIST CColorMappingDlg::m_CtrlList[] = {
	{IDC_CMAP_CYCLE_LEN_SLIDER,		BIND_LEFT | BIND_RIGHT},
	{IDC_CMAP_COLOR_OFS_SLIDER,		BIND_LEFT | BIND_RIGHT},
	{0, 0}	// list terminator
};

enum {	// not a member because view is forward declared
	VIEW_EDIT_FLAGS = CFracticeView::EF_ADD_SNAPSHOT | CFracticeView::EF_COALESCE
};

CColorMappingDlg::CColorMappingDlg(CWnd* pParent /*=NULL*/)
	: CChildDlg(IDD, pParent)
{
	//{{AFX_DATA_INIT(CColorMappingDlg)
	//}}AFX_DATA_INIT
	m_View = NULL;
	m_Quality = 0;
	m_CycleLen = 0;
	m_ColorOfs = 0;
	m_AllowUpdate = TRUE;
}

static inline double InvPow(double base, double x)
{
	return(log(x) / log(base));	// inverse of pow(base, x)
}

double CColorMappingDlg::Norm(double Val, double Base, double MinVal, double MaxVal)
{
	double	Scale = (MaxVal - MinVal) / (Base - 1);
	if (!Scale)
		return(0);	// avoid divide by zero
	return(InvPow(Base, (Val - MinVal) / Scale + 1));
}

double CColorMappingDlg::Denorm(double Val, double Base, double MinVal, double MaxVal)
{
	double	Scale = (MaxVal - MinVal) / (Base - 1);
	return((pow(Base, Val) - 1) * Scale + MinVal);
}

inline double CColorMappingDlg::GetCycleLenBase() const
{
	return(CYCLE_LEN_BASE * (m_Quality / CYCLE_LEN_QUAL_RATIO));
}

double CColorMappingDlg::PosToCycleLen(double Pos) const
{
	return(Denorm(Pos / CYCLE_LEN_RANGE, GetCycleLenBase(), 1, m_Quality));
}

double CColorMappingDlg::CycleLenToPos(double CycleLen) const
{
	return(Norm(CycleLen, GetCycleLenBase(), 1, m_Quality) * CYCLE_LEN_RANGE);
}

inline double CColorMappingDlg::PosToColorOfs(double Pos) const
{
	return(Pos / COLOR_OFS_RANGE);
}

inline double CColorMappingDlg::ColorOfsToPos(double ColorOfs) const
{
	return(ColorOfs * COLOR_OFS_RANGE);
}

void CColorMappingDlg::SetCycleLengthNorm(CFracticeView *View, double Val)
{
	if (View != NULL) {
		double	len = PosToCycleLen(Val * CYCLE_LEN_RANGE);
		View->SetCycleLength(round(len), 0);
	}
}

void CColorMappingDlg::SetColorOffsetNorm(CFracticeView *View, double Val)
{
	if (View != NULL)
		View->SetColorOffset(Val, 0);
}

void CColorMappingDlg::Update()
{
	if (!m_AllowUpdate)	// if updates are disabled
		return;	// nothing to do
	int	CycleLen;
	int	Quality;
	double	ColorOfs;
	if (m_View != NULL) {
		if (!m_View->GetImageSize().cx)	// if view isn't fully initialized
			return;	// defer update to avoid flicker
		CycleLen = m_View->GetCycleLength();
		Quality = m_View->GetQuality();
		ColorOfs = m_View->GetColorOffset();
	} else {
		CycleLen = 0;
		Quality = 0;
		ColorOfs = 0;
	}
	if (Quality != m_Quality) {	// if quality changed
		// redraw cycle length slider tics; spacing is logarithmic
		m_CycleLenSlider.ClearTics();	// remove existing slider tics
		m_Quality = Quality;	// order matters: CycleLenToPos uses m_Quality
		for (int i = 1; i < CYCLE_LEN_TICS; i++) {
			double	cyclen = double(i) / CYCLE_LEN_TICS * Quality;
			int	pos = round(CycleLenToPos(cyclen));	// calculate position
			m_CycleLenSlider.SetTic(pos);	// add slider tic
		}
		m_CycleLen = -1;	// force cycle length update
	}
	if (CycleLen != m_CycleLen) {	// if cycle length changed
		m_CycleLenSlider.SetPos(round(CycleLenToPos(CycleLen)));	// set slider
		m_CycleLenEdit.SetVal(CycleLen);	// set edit control
		m_CycleLen = CycleLen;
	}
	if (ColorOfs != m_ColorOfs) {	// if color offset changed
		m_ColorOfsSlider.SetPos(round(ColorOfsToPos(ColorOfs)));	// set slider
		m_ColorOfsEdit.SetVal(ColorOfs);	// set edit control
		m_ColorOfs = ColorOfs;
	}
}

void CColorMappingDlg::EnableCtrls(bool Enable)
{
	theApp.EnableChildWindows(*this, Enable);
}

void CColorMappingDlg::SetView(CFracticeView *View)
{
	if (View == m_View)
		return;	// nothing to do
	bool	PrevEnab = m_View != NULL;
	m_View = View;
	Update();
	bool	Enable = View != NULL;
	if (Enable != PrevEnab)	// if enable changed
		EnableCtrls(Enable);
}

void CColorMappingDlg::DoDataExchange(CDataExchange* pDX)
{
	CChildDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CColorMappingDlg)
	DDX_Control(pDX, IDC_CMAP_CYCLE_LEN_EDIT, m_CycleLenEdit);
	DDX_Control(pDX, IDC_CMAP_COLOR_OFS_EDIT, m_ColorOfsEdit);
	DDX_Control(pDX, IDC_CMAP_COLOR_OFS_SLIDER, m_ColorOfsSlider);
	DDX_Control(pDX, IDC_CMAP_CYCLE_LEN_SLIDER, m_CycleLenSlider);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CColorMappingDlg, CChildDlg)
	//{{AFX_MSG_MAP(CColorMappingDlg)
	ON_WM_HSCROLL()
	ON_NOTIFY(NEN_CHANGED, IDC_CMAP_CYCLE_LEN_EDIT, OnChangedCycleLenEdit)
	ON_NOTIFY(NEN_CHANGED, IDC_CMAP_COLOR_OFS_EDIT, OnChangedColorOfsEdit)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(UWM_DLGBARUPDATE, OnDlgBarUpdate)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CColorMappingDlg message handlers

BOOL CColorMappingDlg::OnInitDialog() 
{
	CChildDlg::OnInitDialog();
	
	m_CycleLenSlider.SetRange(0, CYCLE_LEN_RANGE);
	m_CycleLenEdit.SetRange(1, MAX_CYCLE_LEN);
	m_ColorOfsSlider.SetRange(0, COLOR_OFS_RANGE);
	m_ColorOfsSlider.SetTicFreq(COLOR_OFS_RANGE / COLOR_OFS_TICS);
	m_ColorOfsEdit.SetRange(MIN_COLOR_OFFSET, MAX_COLOR_OFFSET);
	m_Resize.AddControlList(this, m_CtrlList);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CColorMappingDlg::OnSize(UINT nType, int cx, int cy) 
{
	CChildDlg::OnSize(nType, cx, cy);
	m_Resize.OnSize();
}

LRESULT	CColorMappingDlg::OnDlgBarUpdate(WPARAM wParam, LPARAM lParam)
{
	Update();
	return(0);
}

void CColorMappingDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CSliderCtrl	*pSlider = (CSliderCtrl *)pScrollBar;
	// when we update the view, our Update function will get called, but we've
	// already updated our controls, so we need to disable Update in this case
	m_AllowUpdate = FALSE;	// suppress duplicate update from view
	if (pSlider == &m_CycleLenSlider) {	// if cycle length slider changed
		int	pos = m_CycleLenSlider.GetPos();	// get slider position
		m_CycleLen = round(PosToCycleLen(pos));	// convert to cycle length
		// don't update view while slider is tracking, color mapping is too slow
		if (m_View != NULL && nSBCode != SB_THUMBTRACK)
			m_View->SetCycleLength(m_CycleLen, VIEW_EDIT_FLAGS);	// update view
		m_CycleLenEdit.SetVal(m_CycleLen);	// update edit control
	} else if (pSlider == &m_ColorOfsSlider) {	// if color offset slider changed
		int	pos = m_ColorOfsSlider.GetPos();	// get slider position
		m_ColorOfs = PosToColorOfs(pos);	// convert to color offset
		// don't update view while slider is tracking, color mapping is too slow
		if (m_View != NULL && nSBCode != SB_THUMBTRACK)
			m_View->SetColorOffset(m_ColorOfs, VIEW_EDIT_FLAGS);	// update view
		m_ColorOfsEdit.SetVal(m_ColorOfs);	// update edit control
	}
	m_AllowUpdate = TRUE;	// re-enable updates
	CChildDlg::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CColorMappingDlg::OnChangedCycleLenEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	m_AllowUpdate = FALSE;	// suppress duplicate update from view
	m_CycleLen = m_CycleLenEdit.GetIntVal();	// get edit control value
	if (m_View != NULL)
		m_View->SetCycleLength(m_CycleLen, VIEW_EDIT_FLAGS);	// update view
	m_CycleLenSlider.SetPos(round(CycleLenToPos(m_CycleLen)));	// update slider
	m_AllowUpdate = TRUE;	// re-enable updates
	*pResult = 0;
}

void CColorMappingDlg::OnChangedColorOfsEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	m_AllowUpdate = FALSE;	// suppress duplicate update from view
	m_ColorOfs = m_ColorOfsEdit.GetVal();	// get edit control value
	if (m_View != NULL)
		m_View->SetColorOffset(m_ColorOfs, VIEW_EDIT_FLAGS);	// update view
	m_ColorOfsSlider.SetPos(round(ColorOfsToPos(m_ColorOfs)));	// update slider
	m_AllowUpdate = TRUE;	// re-enable updates
	*pResult = 0;
}
