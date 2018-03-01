// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		06feb09	in OnContextMenu, add color cycling
		02		01jul09	in OnSize, invalidate gradient

        palette dialog
 
*/

// PaletteDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Fractice.h"
#include "PaletteDlg.h"
#include "FracticeDoc.h"
#include "FracticeView.h"
#include "MainFrm.h"
#include "MultiFileDlg.h"
#include "shlwapi.h"
#include "PaletteNameDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPaletteDlg dialog

IMPLEMENT_DYNAMIC(CPaletteDlg, CGradientDlg);

#define RK_PALETTE_PREVIEW		_T("PalettePreview")
#define RK_PALETTE_FILE_PREVIEW	_T("PaletteFilePreview")

CPaletteDlg::CPaletteDlg(CWnd* pParent /*=NULL*/)
	: CGradientDlg(IDD, pParent)
{
	//{{AFX_DATA_INIT(CPaletteDlg)
	//}}AFX_DATA_INIT
	m_Main = NULL;
	m_View = NULL;
	m_Palette = NULL;
	m_InitRect.SetRectEmpty();
	m_BkColor = 0;
	m_WasShown = FALSE;
	m_Preview = theApp.RdRegBool(RK_PALETTE_PREVIEW, TRUE);
	m_Modified = FALSE;
	m_CtrlsEnabled = TRUE;
}

CPaletteDlg::~CPaletteDlg()
{
	theApp.WrRegBool(RK_PALETTE_PREVIEW, m_Preview);
}

void CPaletteDlg::SetView(CFracticeView *View)
{
	if (View == m_View)
		return;	// nothing to do
	bool	Enable = View != NULL;
	if (m_View != NULL)
		GetCtrls(m_View->GetPalette());
	if (Enable) {
		m_Palette = &View->GetPalette();
		if (View->GetImageSize().cx) {	// if view is fully initialized
			SetCtrls(*m_Palette);
		}
	} else {
		m_Palette = NULL;
		ResetCtrls();
	}
	// delay enabling/disabling child controls to avoid flicker
	if (!Enable || m_View == NULL)	// if enable changed
		PostMessage(UWM_ENABLECTRLS, Enable);
	m_View = View;
}

void CPaletteDlg::EnableCtrls(bool Enable)
{
	if (Enable == m_CtrlsEnabled)
		return;	// nothing to do
	theApp.EnableChildWindows(*this, Enable);
	m_ApplyBtn.EnableWindow(Enable && m_Modified);
	m_CtrlsEnabled = Enable;
}

void CPaletteDlg::UpdateView()
{
	if (m_Palette == NULL)
		return;
	if (!IsDragging()) {	// if not dragging
		DPalette	NewPal;
		GetCtrls(NewPal);
		if (NewPal != *m_Palette) {	// if controls differ from palette 
			NewPal.m_Modified = TRUE;
			m_View->NotifyUndoableEdit(CFracticeView::UCODE_EDIT_PALETTE);
			*m_Palette = NewPal;
			if (IsPreviewing())	// if previewing
				m_View->ApplyPalette(0);	// don't add snapshot to history
			SetModify(TRUE);
		}
	}
}

void CPaletteDlg::UpdateColor()
{
	CGradientDlg::UpdateColor();
	UpdateView();
}

void CPaletteDlg::UpdateGradient()
{
	CGradientDlg::UpdateGradient();
	UpdateView();
}

void CPaletteDlg::GetCtrls(DPalette& Palette)
{
	UpdateData(TRUE);	// retrieve data from controls
	GetGradient(Palette);
	Palette.m_BkColor = m_BkColor;
	Palette.m_ColorMode = m_ColorMode;
	Palette.m_SpaceEvenly = m_Grad.GetSpaceEvenly();
	Palette.m_Name = m_PaletteName;
	Palette.m_CurSel = m_Grad.GetCurSel();
	Palette.m_Modified = m_Modified;
}

void CPaletteDlg::SetCtrls(const DPalette& Palette)
{
	m_BkColor = Palette.m_BkColor;
	m_ColorMode = Palette.m_ColorMode;
	m_SpaceEvenlyChk.SetCheck(Palette.m_SpaceEvenly);
	SetPaletteName(Palette.m_Name);
	UpdateData(FALSE);	// update controls from data
	SetGradient(Palette);
	if (Palette.m_CurSel >= 0)
		m_Grad.SetCurSel(Palette.m_CurSel);
	else {
		if (GetCount())
			m_Grad.SetCurSel(0);
	}
	SetModify(Palette.m_Modified);
}

void CPaletteDlg::InitCtrls()
{
	SetCtrls(*m_Palette);
}

void CPaletteDlg::ResetCtrls()
{
	DPalette	pal;
	SetCtrls(pal);
	SetColor(DRGB(0));
	m_PosEdit.SetVal(0);
}

int CPaletteDlg::FindMenuItem(const CMenu& menu, UINT ItemID)
{
	int	items = menu.GetMenuItemCount();
	for (int i = 0; i < items; i++) {
		if (menu.GetMenuItemID(i) == ItemID)
			return(i);
	}
	return(-1);
}

void CPaletteDlg::SetPaletteName(const CString& Name)
{
	CString	cap(m_OrigCaption);
	if (!Name.IsEmpty())
		cap += CString(" - ") + Name;
	GetParent()->SetWindowText(cap);	// update host bar's caption
	m_PaletteName = Name;
}

bool CPaletteDlg::ChooseFile(CString& Path, bool& Preview, CWnd *NotifyWnd, bool Import)
{
	LPCTSTR	DefExt;
	int	FilterID;
	CString	*Folder;
	if (Import) {
		DefExt = IMPORT_PAL_EXT;
		FilterID = IDS_IMP_PAL_FILTER;
		Folder = &theApp.m_ImportFolder;
	} else {
		DefExt = PALETTE_EXT;
		FilterID = IDS_PALETTE_FILTER;
		Folder = &theApp.m_PaletteFolder;
	}
	CMultiFileDlg	fd(TRUE, DefExt, NULL, OFN_HIDEREADONLY,
		LDS(FilterID), NotifyWnd, NULL, Folder);
	fd.SetExtendedStyle(CMultiFileDlg::ES_SHOWPREVIEW);
	fd.SetNotifyWnd(NotifyWnd);
	Preview = theApp.RdRegBool(RK_PALETTE_FILE_PREVIEW, FALSE);
	fd.SetPreview(Preview);
	bool	retc;
	if (fd.DoModal() == IDOK) {
		Path = fd.GetPathName();
		retc = TRUE;
	} else
		retc = FALSE;
	Preview = fd.GetPreview();
	theApp.WrRegBool(RK_PALETTE_FILE_PREVIEW, Preview); 
	return(retc);
}

void CPaletteDlg::SetModify(bool Enable)
{
	if (Enable == m_Modified)
		return;
	m_Modified = Enable;
	m_ApplyBtn.EnableWindow(Enable);
}

void CPaletteDlg::DoDataExchange(CDataExchange* pDX)
{
	CGradientDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPaletteDlg)
	DDX_Control(pDX, ID_APPLY_NOW, m_ApplyBtn);
	DDX_Control(pDX, IDC_PAL_PREVIEW_CHK, m_PreviewChk);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPaletteDlg, CGradientDlg)
	//{{AFX_MSG_MAP(CPaletteDlg)
	ON_BN_CLICKED(IDC_PAL_BK_COLOR, OnBkColor)
	ON_BN_CLICKED(IDC_PAL_PREVIEW_CHK, OnPreview)
	ON_WM_DESTROY()
    ON_WM_GETMINMAXINFO()
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_WM_CONTEXTMENU()
	ON_BN_CLICKED(ID_APPLY_NOW, OnApplyNow)
	ON_BN_CLICKED(IDC_PAL_EDIT_NAME, OnEditName)
	//}}AFX_MSG_MAP
	ON_MESSAGE(UWM_ENABLECTRLS, OnEnableCtrls)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPaletteDlg message handlers

BOOL CPaletteDlg::OnInitDialog() 
{
	CGradientDlg::OnInitDialog();

	m_Main = theApp.GetMain();
	m_OrigCaption.LoadString(IDS_CBT_PALETTE);
	m_PreviewChk.SetCheck(m_Preview);
	GetWindowRect(m_InitRect);
	m_Resize.SetParentWnd(this);
	m_Resize.AddControl(IDC_GRAD_GRADIENT, BIND_LEFT | BIND_RIGHT);
	m_Resize.FixControls();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPaletteDlg::OnDestroy() 
{
	m_Preview = IsPreviewing();
	m_Palette = NULL;
	CGradientDlg::OnDestroy();
}

void CPaletteDlg::OnOK() 
{
}

void CPaletteDlg::OnCancel() 
{
}

void CPaletteDlg::OnClose() 
{
}

LRESULT	CPaletteDlg::OnEnableCtrls(WPARAM wParam, LPARAM lParam)
{
	EnableCtrls(wParam != 0);
	return 0;
}

void CPaletteDlg::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	if (!m_InitRect.IsRectNull()) {	// wait until rect is initialized
		lpMMI->ptMinTrackSize.x = m_InitRect.Width();
		lpMMI->ptMinTrackSize.y = m_InitRect.Height();
		lpMMI->ptMaxTrackSize.y = m_InitRect.Height();
	}
}

void CPaletteDlg::OnSize(UINT nType, int cx, int cy) 
{
	CGradientDlg::OnSize(nType, cx, cy);
	if (!m_InitRect.IsRectNull()) {	// wait until rect is initialized
		m_Resize.OnSize();
		m_Grad.Invalidate();	// control resize copies bits, not helpful here
		m_Grad.InvalidateMarkers();
		// also invalidate from right edge of marker area to right edge of dialog
		// otherwise making window narrower leaves a trail of marker debris
		CRect	r;
		m_Grad.GetMarkerRect(r);
		m_Grad.MapWindowPoints(this, r);
		r.left = r.right;
		r.right = cx;
		InvalidateRect(r);
	}
}

BOOL CPaletteDlg::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message >= WM_KEYFIRST || pMsg->message <= WM_KEYLAST) {
		if (theApp.HandleDlgKeyMsg(pMsg))	// give main accelerators a try
			return(TRUE);	// message was translated, stop dispatching
	}
	return CGradientDlg::PreTranslateMessage(pMsg);
}

void CPaletteDlg::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	CRect	r;
	GetClientRect(r);
	CPoint	pt(point);
	ScreenToClient(&pt);
	// if within client area but not within gradient control
	if (r.PtInRect(pt) && !CtrlHitTest(point) && m_View != NULL) {
		CMenu	menu, *mp;
		menu.LoadMenu(IDR_PALETTE);	// show palette popup menu
		mp = menu.GetSubMenu(0);
		mp->CheckMenuItem(MF_BYCOMMAND | ID_IMG_CYCLE_COLORS, 
			m_View->GetCycleColors() ? MF_CHECKED : MF_UNCHECKED);
		CCmdUI	cui;
		cui.m_pMenu = mp;
		cui.m_nID = ID_IMG_MRU_PALETTE1;
		cui.m_nIndex = FindMenuItem(*mp, ID_IMG_MRU_PALETTE1);
		cui.m_nIndexMax = mp->GetMenuItemCount();	// required
		cui.DoUpdate(m_View, FALSE);	// update most recently used palettes
		mp->TrackPopupMenu(0, point.x, point.y, m_View);
	} else
		CGradientDlg::OnContextMenu(pWnd, point);	// delegate to base class
}

void CPaletteDlg::OnPreview() 
{
	m_Preview = m_PreviewChk.GetCheck() != 0;
	UpdateView();
}

BOOL CPaletteDlg::CBkColorDlg::OnInitDialog()
{
	SetWindowText(LDS(IDS_PAL_BKGND_COLOR));
	// our parent (the palette dialog) isn't a popup; it's a child of the
	// palette control bar, which is a child of the frame, so our default
	// centering would be relative to the frame, but that looks unnatural
	CenterWindow(GetParent());	// center relative to palette dialog
	return CColorPickerDlg::OnInitDialog();
}

void CPaletteDlg::CBkColorDlg::UpdateColor()
{
	CColorPickerDlg::UpdateColor();
	if (m_PalDlg->IsPreviewing() && !IsDragging()) {
		m_PalDlg->SetBkColor(GetColor());
		m_PalDlg->UpdateView();
	}
}

void CPaletteDlg::OnBkColor() 
{
	COLORREF	BkColor = m_BkColor;
	CBkColorDlg	dlg;
	dlg.m_PalDlg = this;
	dlg.SetColor(m_BkColor);
	bool	WasModified = m_Modified;
	if (dlg.DoModal() == IDOK) {
		m_BkColor = dlg.GetColor();
		UpdateView();
	} else {
		if (IsPreviewing()) {
			m_BkColor = BkColor;
			UpdateView();
		}
		SetModify(WasModified);
	}
}

void CPaletteDlg::OnEditName() 
{
	class CMyPaletteNameDlg: public CPaletteNameDlg {
	public:
		BOOL OnInitDialog() {	// see comment in CBkColorDlg::OnInitDialog
			CenterWindow(GetParent());	// center relative to palette dialog
			return CPaletteNameDlg::OnInitDialog();
		}
	};
	CMyPaletteNameDlg	dlg;
	dlg.m_Name = m_PaletteName;
	if (dlg.DoModal() == IDOK) {
		SetPaletteName(dlg.m_Name);
		m_View->NotifyUndoableEdit(CFracticeView::UCODE_EDIT_PALETTE);
		if (IsPreviewing())
			m_Palette->m_Name = dlg.m_Name;	// instead of UpdateView
		SetModify(TRUE);
	}
}

void CPaletteDlg::OnApplyNow() 
{
	SetModify(FALSE);
	GetCtrls(*m_Palette);
	m_View->ApplyPalette();
}
