// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		29oct08	add title and folder args to ctor
		02		06jan10	W64: DoModal return value is 64-bit
		03		06jan10	W64: in BuildExtList, cast array size to 32-bit

        Save As dialog with enhanced support for multiple file types
 
*/

// SaveAsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "SaveAsDlg.h"
#include "PathStr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSaveAsDlg

IMPLEMENT_DYNAMIC(CSaveAsDlg, CFileDialog);

CSaveAsDlg::CSaveAsDlg(BOOL bOpenFileDialog, LPCTSTR lpszDefExt,
					   LPCTSTR lpszFileName, DWORD dwFlags, LPCTSTR lpszFilter,
					   CWnd* pParentWnd, LPCTSTR lpszTitle, CString *psFolder) :
	CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd),
	m_Title(lpszTitle),
	m_psFolder(psFolder)
{
	//{{AFX_DATA_INIT(CSaveAsDlg)
	//}}AFX_DATA_INIT
}

void CSaveAsDlg::BuildExtList()
{
	LPCTSTR	p = m_ofn.lpstrFilter;
	if (p != NULL) {
		bool	pattern = FALSE;
		while (*p) {	// for each filter description/pattern pair
			if (pattern)	// if filter pattern
				m_Ext.Add(p + 1);	// add to list; remove leading asterik
			p += _tcslen(p) + 1;	// advance to next string
			pattern ^= 1;	// ignore filter descriptions
		}
		if (m_Ext.GetSize()) {	// if at least one extension
			int	idx = m_ofn.nFilterIndex - 1;	// get extension index
			int	exts = INT64TO32(m_Ext.GetSize());
			idx = CLAMP(idx, 0, exts - 1);
			m_ofn.lpstrDefExt = m_Ext[idx];	// set initial default extension
		}
	}
}

void CSaveAsDlg::DoDataExchange(CDataExchange* pDX)
{
	CFileDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSaveAsDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSaveAsDlg, CFileDialog)
	//{{AFX_MSG_MAP(CSaveAsDlg)
	//}}AFX_MSG_MAP
	ON_NOTIFY(CDN_TYPECHANGE, 0, OnTypeChange)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSaveAsDlg message handlers

W64INT CSaveAsDlg::DoModal()
{
	BuildExtList();
	if (m_psFolder != NULL) {
		m_ofn.lpstrInitialDir = *m_psFolder;
		m_ofn.Flags |= OFN_NOCHANGEDIR;	// otherwise folder stays locked
	}
	W64INT	retc = CFileDialog::DoModal();
	if (retc == IDOK) {
		if (m_psFolder != NULL)
			GetFolder(*m_psFolder);
	}
	return(retc);
}

void CSaveAsDlg::OnTypeChange(NMHDR* pNMHDR, LRESULT* pResult)
{
	int	idx = m_ofn.nFilterIndex - 1;
	if (idx >= 0 && idx < m_Ext.GetSize()) {	// if valid extension index
		LPCTSTR	pExt = m_Ext[idx];	// get corresponding extension
		pExt++;	// remove leading dot; for details, see CDM_SETDEFEXT
		CommDlg_OpenSave_SetDefExt(m_hWnd, pExt);	// set default extension
	}
}

void CSaveAsDlg::GetFolder(CString& Folder)
{
	CPathStr	s(GetPathName());
	s.RemoveFileSpec();
	Folder = s;
}
