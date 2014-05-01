// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		06jan09	don't save image if already cached in history
		02		07feb09	in OnSaveDocument, save escape times
		03		20feb09	in OnSaveDocument, handle exceptions
		04		13mar09	add project snapshot

        document class
 
*/

// FracticeDoc.cpp : implementation of the CFracticeDoc class
//

#include "stdafx.h"
#include "Fractice.h"

#include "MainFrm.h"
#include "FracticeDoc.h"
#include "FracticeView.h"

#include "PathStr.h"
#include "SaveAsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFracticeDoc

DWORD	CFracticeDoc::m_DocsCreated;
CFracticeDoc	*CFracticeDoc::m_FirstNewDoc;

IMPLEMENT_DYNCREATE(CFracticeDoc, CDocument)

BEGIN_MESSAGE_MAP(CFracticeDoc, CDocument)
	//{{AFX_MSG_MAP(CFracticeDoc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFracticeDoc construction/destruction

CFracticeDoc::CFracticeDoc()
{
	m_DocsCreated++;
}

CFracticeDoc::~CFracticeDoc()
{
	m_FirstNewDoc = NULL;
}

BOOL CFracticeDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	if (m_DocsCreated == 1)
		m_FirstNewDoc = this;
	m_Snap.SetEmpty();
	Reset();

	return TRUE;
}

CFracticeView *CFracticeDoc::GetView() const
{
	POSITION	pos = GetFirstViewPosition();
	CFracticeView	*View = DYNAMIC_DOWNCAST(CFracticeView, GetNextView(pos));
	ASSERT(View);
	return(View);
}

/////////////////////////////////////////////////////////////////////////////
// CFracticeDoc serialization

void CFracticeDoc::Serialize(CArchive& ar)
{
	CProject::Serialize(ar);
}

/////////////////////////////////////////////////////////////////////////////
// CFracticeDoc diagnostics

#ifdef _DEBUG
void CFracticeDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CFracticeDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFracticeDoc commands

bool CFracticeDoc::ReadSnapshot(LPCTSTR Path)
{
	m_History.RemoveAll();
	return(m_Snap.Read(Path));
}

bool CFracticeDoc::WriteSnapshot(LPCTSTR Path)
{
	GetView()->GetSnapshot(m_Snap);
	return(m_Snap.Write(Path));
}

bool CFracticeDoc::IsSnapshot(LPCTSTR Path)
{
	return(!_tcsicmp(PathFindExtension(Path), SNAPSHOT_EXT));
}

void CFracticeDoc::Close()
{
	POSITION pos = GetFirstViewPosition();
	while (pos != NULL) {	// for each view of this document
		CView	*pView = GetNextView(pos);
		CFrameWnd	*pFrame = pView->GetParentFrame();	// find its parent frame
		if (pFrame != NULL)
			pFrame->PostMessage(WM_CLOSE);	// and close it
	}
}

BOOL CFracticeDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	BOOL	retc;
	if (IsSnapshot(lpszPathName))	// if path is a snapshot
		retc = ReadSnapshot(lpszPathName);
	else {	// path is a project
		CMainFrame::CStatusMsg	status(IDS_DOC_OPENING);
		retc = CDocument::OnOpenDocument(lpszPathName);	// do base class open
	}
	// if open succeeded and first new document is unmodified
	if (retc && m_FirstNewDoc != NULL && !m_FirstNewDoc->IsModified())
		m_FirstNewDoc->Close();	// close first new document
	return(retc);
}

BOOL CFracticeDoc::SaveProject(LPCTSTR lpszPathName)
{
	TRY {
		// OnSaveDocument is supposed to handle exceptions, but it nonetheless
		// throws a CFileException when attempting to save to a read-only file,
		// and fails to remove its temporary file from the destination folder. 
		// The problem is located in a internal MFC class called CMirrorFile.
		// CMirrorFile::Abort calls CFile::Remove to delete the mirror file,
		// but neglects to use a TRY/CATCH block. This is an error, because 
		// CFile::Remove throws exceptions, and the whole point of Abort is
		// that it's supposed to ignore failures. CMirrorFile::Abort is called
		// by CDocument::ReleaseFile, which is called from a CATCH_ALL handler
		// in OnSaveDocument, and neither are expecting an exception, so the
		// exception goes uncaught unless the derived document handles it. The
		// error is found in MFC versions at least up to 2005; see doccore.cpp.
		return(CDocument::OnSaveDocument(lpszPathName));
	}
	CATCH (CFileException, e) {
		// assume exception has already been reported by MFC
	}
	END_CATCH
	return(FALSE);	// an exception occurred
}

BOOL CFracticeDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	if (IsSnapshot(lpszPathName)) {	// if path is a snapshot
		if (!WriteSnapshot(lpszPathName))
			return(FALSE);
		SetModifiedFlag(FALSE);
		return(TRUE);
	}
	// path is a project
	CMainFrame::CStatusMsg	status(IDS_DOC_SAVING);
	CFracticeView	*view = GetView();
	view->GetProject(*this);	// get project data
	BOOL	retc;
	//
	// The project normally won't have a snapshot, unless a) the view doesn't
	// have a current history item, b) the view has a current history item but
	// the item doesn't include a cached image, or c) the current history item
	// differs from the view's current state.
	//
	if (HasSnapshot()) {	// if project has a snapshot
		//
		// Store the view's current image, and optionally its escape times, in
		// the project snapshot. Both the image and escape times are potentially
		// huge, so instead of duplicating them, swap objects with the view's
		// engine, according to the following scheme:
		//
		// 1) Create a dummy image, and optionally dummy escape times.
		// 2) Exchange objects with the engine, giving the engine the dummies.
		// 3) Serialize our project snapshot, which contains the real objects.
		// 4) Exchange again, restoring the real objects to the engine.
		// 5) Destroy our dummy objects.
		//
		m_Snapshot->m_Image.CreateObj();	// create dummy image
		CEscTimeArray	*pEscTimes;
		// if saving escape times
		if (theApp.GetMain()->GetOptionsDlg().GetSaveEscTimes()) {
			m_Snapshot->m_EscTimes.CreateObj();	// create dummy escape times
			pEscTimes = m_Snapshot->m_EscTimes;	// set escape times pointer
		} else	// not saving escape times
			pEscTimes = NULL;
		// exchange objects with engine
		view->GetEngine().Swap(m_Snapshot->m_Image, pEscTimes);
		retc = SaveProject(lpszPathName);	// do base class save
		// exchange objects with engine again, restoring previous state
		view->GetEngine().Swap(m_Snapshot->m_Image, pEscTimes);
		m_Snapshot.SetEmpty();	// destroy the dummies
	} else {	// normal case; project doesn't have a snapshot
		retc = SaveProject(lpszPathName);	// do base class save
	}
	m_History.RemoveAll();	// remove our history references
	return(retc);
}

BOOL CFracticeDoc::DoSave(LPCTSTR lpszPathName, BOOL bReplace)
{
	CString newName = lpszPathName;
	if (newName.IsEmpty()) {
		CSaveAsDlg	fd(FALSE, NULL, m_strPathName, OFN_OVERWRITEPROMPT, 
			LDS(IDS_DOC_SAVE_FILTER), NULL, NULL, &theApp.m_DocFolder);
		int	fmt;
		if (m_strPathName.IsEmpty())
			fmt = theApp.GetMain()->GetOptionsDlg().GetDefDocFmt();
		else
			fmt = IsSnapshot(m_strPathName);
		fd.m_ofn.nFilterIndex = fmt + 1;
		if (fd.DoModal() != IDOK)
			return(FALSE);
		newName = fd.GetPathName();
	}
	if (!OnSaveDocument(newName)) {
		if (lpszPathName == NULL)
			CFile::Remove(newName);
		return(FALSE);
	}
	if (bReplace)
		SetPathName(newName);
	return(TRUE);
}

HMENU CFracticeDoc::GetDefaultMenu()
{
	return(theApp.GetMain()->GetChildFrameMenu());
}

BOOL CFracticeDoc::SaveModified() 
{
	if (!theApp.GetMain()->GetOptionsDlg().GetSaveChgsWarn())
		return TRUE;	// close modified document without warning
	return CDocument::SaveModified();	// normal behavior
}

LPCTSTR CFracticeDoc::GetFileName() const
{
	return(PathFindFileName(GetPathName()));
}
