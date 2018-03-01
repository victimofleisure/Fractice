// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14mar09	initial version
		01		15mar09	in ApplyChanges, fix missing indirection
		02		06jan10	W64: in ApplyChanges, cast SelIdx array size to 32-bit

        transform history dialog
 
*/

// TransformDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Fractice.h"
#include "TransformDlg.h"
#include "FracticeDoc.h"
#include "FracticeView.h"
#include "RenderingDlg.h"
#include "MultiFileDlg.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTransformDlg dialog

IMPLEMENT_DYNAMIC(CTransformDlg, CDialog);

CTransformDlg::CTransformDlg(CFracticeView& View, CWnd* pParent)
	: CDialog(CTransformDlg::IDD, pParent), m_View(View)
{
	//{{AFX_DATA_INIT(CTransformDlg)
	m_PalettePath = _T("");
	m_Render = TRUE;
	//}}AFX_DATA_INIT
	ZeroMemory(&m_ParmMod, sizeof(m_ParmMod));
	m_ParmModCount = 0;
	m_FilterIndex = 1;
}

void CTransformDlg::ReadParams()
{
	m_ParmModCount = 0;
	for (int i = 0; i < NUM_PARMS; i++) {	// for each parameter
		CString	s = m_ParamsCtrl.GetItemText(i, 1);
		m_ParmMod[i] = !s.IsEmpty();	// store whether parameter was specified
		if (m_ParmMod[i])	// if parameter was specified
			m_ParmModCount++;	// count it
	}
}

bool CTransformDlg::ReadPalette()
{
	if (m_PalettePath.IsEmpty())
		return(FALSE);
	if (DPalette::IsNativeFormat(m_PalettePath)) {	// if palette in native format
		if (!m_NewPalette.Read(m_PalettePath))
			return(FALSE);
	} else {	// not in native format, try importing it
		UINT	flags = theApp.GetMain()->GetOptionsDlg().GetPalImpExpFlags();
		if (!m_NewPalette.Import(m_PalettePath, flags))
			return(FALSE);
	}
	return(TRUE);
}

#undef PARMDEF
#define PARMDEF(name, member, type, defval, minval, maxval, combo) \
{ if (m_ParmMod[PARM_##name]) snap.member = ParamsSnap.member; }

void CTransformDlg::ApplyChanges()
{
	CHistoryView	*Hist = m_View.GetHistoryView();
	CDWordArray	SelIdx;
	Hist->GetSelection(SelIdx);
	int	sels = INT64TO32(SelIdx.GetSize());
	CSnapshot	ParamsSnap;
	CSnapshot	def;
	m_ParamsCtrl.GetSnapshot(ParamsSnap);
	bool	ColorChangeOnly = def.EqualIgnoreColor(ParamsSnap);
	for (int i = 0; i < sels; i++) {	// for each selected history item
		CSnapshot&	snap = Hist->GetItem(SelIdx[i]);	// lookup via index
		snap.m_Image.SetEmpty();	// delete cached image
		if (!ColorChangeOnly)	// unless this is a color-only change
			snap.m_EscTimes.SetEmpty();	// delete escape times too
		// invoke PARMDEF macro for each parameter; for each parameter with
		// a true m_ParmMod flag, copy the corresponding snapshot member from
		// ParamsSnap to the destination history item
		#include "ParamData.h"
		if (!m_PalettePath.IsEmpty())	// if palette was specified
			snap.m_Palette = m_NewPalette;	// apply new palette
	}
	if (m_Render) {
		CRenderingDlg	dlg(CRenderingDlg::CF_CREATE_THUMBS);
		dlg.DoModal();	// do multiple render
	}
}

void CTransformDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTransformDlg)
	DDX_Control(pDX, IDC_TRAN_PARAMS_CTRL, m_ParamsCtrl);
	DDX_Text(pDX, IDC_TRAN_PALETTE_EDIT, m_PalettePath);
	DDX_Check(pDX, IDC_TRAN_RENDER_CHK, m_Render);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CTransformDlg, CDialog)
	//{{AFX_MSG_MAP(CTransformDlg)
	ON_BN_CLICKED(IDC_TRAN_PALETTE_BROWSE, OnPaletteBrowse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTransformDlg message handlers

BOOL CTransformDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_ParamsCtrl.Init();
	m_ParamsCtrl.SetOptions(CParamsCtrl::OP_ALLOW_EMPTY);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTransformDlg::OnOK() 
{
	UpdateData();
	ReadParams();	// get parameters from control
	if (!m_ParmModCount && m_PalettePath.IsEmpty()) {
		AfxMessageBox(IDS_TRAN_NO_PARAMS);
		return;	// user didn't specify any changes
	}
	if (!m_PalettePath.IsEmpty() && !ReadPalette())
		return;	// invalid palette file
	if (AfxMessageBox(IDS_TRAN_CANT_UNDO, MB_YESNO) != IDYES)
		return;	// user chickened out
	CDialog::OnOK();	// close our window
	ApplyChanges();	// do the work
}

void CTransformDlg::OnPaletteBrowse() 
{
	CMultiFileDlg	fd(TRUE, PALETTE_EXT, NULL, OFN_HIDEREADONLY, 
		LDS(IDS_ALL_PAL_FILTER), NULL, theApp.m_PaletteFolder);
	fd.m_ofn.nFilterIndex = m_FilterIndex;
	if (fd.DoModal() == IDOK) {
		m_FilterIndex = fd.m_ofn.nFilterIndex;
		m_PalettePath = fd.GetPathName();
		UpdateData(FALSE);
	}
}
