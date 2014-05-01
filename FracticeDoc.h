// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version

        document class
 
*/

// FracticeDoc.h : interface of the CFracticeDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_FRACTICEDOC_H__842350E1_F227_4CC1_9AB0_D05D3F3BC19C__INCLUDED_)
#define AFX_FRACTICEDOC_H__842350E1_F227_4CC1_9AB0_D05D3F3BC19C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Project.h"

class CFracticeView;

class CFracticeDoc : public CDocument, public CProject
{
protected: // create from serialization only
	CFracticeDoc();
	DECLARE_DYNCREATE(CFracticeDoc)

public:
// Constants
	enum {	// document formats; order must match 1) order of filters in
			// IDS_DOC_SAVE_FILTER string, and 2) order of default file
			// format radio buttons in options dialog
		DOCF_PROJECT,	// entire project, including history
		DOCF_SNAPSHOT,	// final state of view only
		DOC_FORMATS		// number of document formats
	};

// Public data
public:
	CSnapshot	m_Snap;		// if doc is a snapshot, final state of view

// Attributes
	bool	IsSnapshot() const;
	static	bool	IsSnapshot(LPCTSTR Path);
	LPCTSTR	GetFileName() const;

// Operations
public:
	bool	ReadSnapshot(LPCTSTR Path);
	bool	WriteSnapshot(LPCTSTR Path);
	void	Close();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFracticeDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual HMENU GetDefaultMenu();
	protected:
	virtual BOOL SaveModified();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CFracticeDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	//{{AFX_MSG(CFracticeDoc)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Overrides
	BOOL	DoSave(LPCTSTR lpszPathName, BOOL bReplace);

// Data members
	static	DWORD	m_DocsCreated;	// number of documents created
	static	CFracticeDoc	*m_FirstNewDoc;	// first new document

// Helpers
	CFracticeView	*GetView() const;
	BOOL	SaveProject(LPCTSTR lpszPathName);
};

inline bool CFracticeDoc::IsSnapshot() const
{
	return(!m_History.GetSize());
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FRACTICEDOC_H__842350E1_F227_4CC1_9AB0_D05D3F3BC19C__INCLUDED_)
