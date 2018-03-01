// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		01feb09	in OnItemChange, support float
		02		14mar09	add get/set options
		03		01jul09	in Init, image list must be a member
		04		23jul09	add Mandelbar
		
		parameters control
 
*/

// ParamsCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "ParamsCtrl.h"
#include "FormatIO.h"
#include "FracticeDoc.h"
#include "FracticeView.h"	// for undo codes

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CParamsCtrl

IMPLEMENT_DYNAMIC(CParamsCtrl, CGridCtrl);

const CParamsCtrl::COL_INFO CParamsCtrl::m_ColInfo[COLUMNS] = {
	{IDS_PARM_PARAM,	LVCFMT_LEFT,	80},
	{IDS_PARM_VALUE,	LVCFMT_LEFT,	60},
};

const int CParamsCtrl::m_DeepZoomComboStr[] = {
	IDS_PARM_NO,
	IDS_PARM_YES,
	0	// terminator
};

const CParamsCtrl::COMBO_INFO CParamsCtrl::m_DeepZoomComboInfo = {
	m_DeepZoomComboStr,		0
};

// this list must match the fractal type enum in Engine.h
const int CParamsCtrl::m_FractalTypeComboStr[] = {
	IDS_FT_MANDELBROT,
	IDS_FT_NEWTON,
	IDS_FT_MANDELBAR,
	0	// terminator
};

const CParamsCtrl::COMBO_INFO CParamsCtrl::m_FractalTypeComboInfo = {
	m_FractalTypeComboStr,	CBS_SORT
};

#undef PARMDEF
#define PARMDEF(name, member, type, defval, minval, maxval, combo) \
{IDS_PARM_##name, FIO_##type, offsetof(CSnapshot, member), minval, maxval, CFracticeView::UCODE_##name, combo},

const CParamsCtrl::PARM_INFO CParamsCtrl::m_ParmInfo[NUM_PARMS] = {
#include "ParamData.h"
};

CParamsCtrl::CParamsCtrl()
{
	m_UndoCode = 0;
	m_ModCount = 0;
	m_LastModIdx = 0;
	m_Options = 0;
}

CParamsCtrl::~CParamsCtrl()
{
}

void CParamsCtrl::Init()
{
	CString	s;
	int	i;
	for (i = 0; i < COLUMNS; i++) {
		const COL_INFO&	ci = m_ColInfo[i];
		s.LoadString(ci.Title);
		InsertColumn(i, s, ci.Align, ci.Width);
	}
	for (i = 0; i < NUM_PARMS; i++) {
		s.LoadString(m_ParmInfo[i].NameID);
		InsertItem(i, s);
	}
	int	style = GetExtendedStyle();
	SetExtendedStyle(style | LVS_EX_GRIDLINES);
	m_ImgList.Create(1, ROW_HEIGHT, ILC_COLOR4, 1, 1);
	SetImageList(&m_ImgList, LVSIL_SMALL); 	// set row height
}

void CParamsCtrl::Update()
{
	CString	s;
	for (int i = 0; i < NUM_PARMS; i++) {
		const PARM_INFO&	pi = m_ParmInfo[i];
		CFormatIO::ValToStr(pi.Type, ((BYTE *)&m_Snap) + pi.Offset, s);
		if (pi.ComboInfo != NULL)
			s.LoadString(pi.ComboInfo->ItemID[_ttoi(s)]);
		SetItemText(i, 1, s);
	}
}

void CParamsCtrl::SetSnapshot(const CSnapshot& Snap)
{
	m_Snap = Snap;
	Update();
}

void CParamsCtrl::SetEmpty()
{
	m_Snap.SetEmpty();
	for (int i = 0; i < NUM_PARMS; i++)
		SetItemText(i, 1, _T(""));
}

void CParamsCtrl::OnBeginEdit()
{
	m_ModCount = 0;
	m_LastModIdx = 0;
}

bool CParamsCtrl::OnItemChange(int Row, int Col, LPCTSTR Text)
{
	ASSERT(Row >= 0 && Row < NUM_PARMS);
	const PARM_INFO&	pi = m_ParmInfo[Row];
	CString	s;
	if (pi.ComboInfo != NULL) {
		int	sel = FindComboString(Row, Text);
		if (sel < 0)
			return(FALSE);	// logic error
		s.Format(_T("%d"), sel);
		Text = s;
	} else {
		if ((m_Options & OP_ALLOW_EMPTY) && !_tcslen(Text))
			return(TRUE);	// text is empty and it's allowed
		SetWindowText(Text);
		CDataExchange	dx(GetParent(), TRUE);
		int	idc = GetDlgCtrlID();
		HWND	hWnd = dx.PrepareEditCtrl(idc);
		if (hWnd != NULL) {
			TRY {
				switch (pi.Type) {
				case FIO_FLOAT:
				case FIO_DOUBLE:
					{
						double	val;
						DDX_Text(&dx, idc, val);
						DDV_MinMaxDouble(&dx, val, double(pi.MinVal), double(pi.MaxVal));
					}
					break;
				default:
					int	val;
					DDX_Text(&dx, idc, val);
					DDV_MinMaxInt(&dx, val, pi.MinVal, pi.MaxVal);
				}
			}
			CATCH(CException, e)
			{
				return(FALSE);
			}
			END_CATCH
		}
	}
	m_UndoCode = pi.UndoCode;
	m_ModCount++;
	m_LastModIdx = Row;
	return(CFormatIO::StrToVal(pi.Type, Text, ((BYTE *)&m_Snap) + pi.Offset));
}

int CParamsCtrl::FindComboString(int Row, LPCTSTR Text)
{
	ASSERT(Row >= 0 && Row < NUM_PARMS);
	const PARM_INFO&	pi = m_ParmInfo[Row];
	CString	s;
	if (pi.ComboInfo != NULL) {
		const int	*pItemID = pi.ComboInfo->ItemID;
		for (int i = 0; pItemID[i]; i++) {
			s.LoadString(pItemID[i]);
			if (s == Text)
				return(i);
		}
	}
	return(-1);
}

CString	CParamsCtrl::GetComboString(int Row, int SelIdx)
{
	ASSERT(Row >= 0 && Row < NUM_PARMS);
	const PARM_INFO&	pi = m_ParmInfo[Row];
	CString	s;
	if (pi.ComboInfo != NULL)
		s.LoadString(pi.ComboInfo->ItemID[SelIdx]);
	return(s);
}

bool CParamsCtrl::GetComboStrings(int Row, CStringArray& ComboStr, UINT& Style)
{
	ASSERT(Row >= 0 && Row < NUM_PARMS);
	const PARM_INFO&	pi = m_ParmInfo[Row];
	if (pi.ComboInfo == NULL)
		return(FALSE);
	CString	s;
	const int	*pItemID = pi.ComboInfo->ItemID;
	for (int i = 0; pItemID[i]; i++) {
		s.LoadString(pItemID[i]);
		ComboStr.Add(s);
	}
	Style |= pi.ComboInfo->Style;
	return(TRUE);
}

BEGIN_MESSAGE_MAP(CParamsCtrl, CGridCtrl)
	//{{AFX_MSG_MAP(CParamsCtrl)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CParamsCtrl message handlers
