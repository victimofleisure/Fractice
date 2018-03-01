// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      24feb09	initial version
		01		11mar09	use modal accelerators during drag
		02		05jan10	standardize OnInitDialog prototype
		
		customized file browser control bar
 
*/

// FilesBar.cpp : implementation file
//

#include "stdafx.h"
#include "Fractice.h"
#include "FilesBar.h"
#include "Persist.h"
#include "PersistDlg.h"	// for UpdateMenu
#include "MainFrm.h"
#include "FracticeDoc.h"
#include "FracticeView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFilesBar dialog

IMPLEMENT_DYNAMIC(CFilesBar, CFileBrowserControlBar);

#define RK_FB_CUR_PANE		_T("FBCurPane")
#define RK_FB_PANE_STATE	_T("FBPaneState")

static const LPCTSTR ProjectExt[] = {PROJECT_EXT, SNAPSHOT_EXT};

const CFilesBar::PANE_INFO CFilesBar::PaneInfo[PANES] = {
	{IDS_FB_PROJECTS,	LVS_SINGLESEL,	ProjectExt,	sizeof(ProjectExt) / sizeof(LPCTSTR)},
	{IDS_FB_PALETTES,	LVS_SINGLESEL,	NULL,		0},	// wildcard
};

const CFilesBar::PANE_STATE CFilesBar::m_DefaultPaneState = {
	CFileBrowserListCtrl::VTP_REPORT, CDirList::SORT_NAME, CDirList::ORDER_ASC, {0, 1, 2, 3}
};

// alphabetical order by ID name; must match Resource.h and message map ranges
const int CFilesBar::m_ViewTypeMap[] = {
	CFileBrowserListCtrl::VTP_ICON,			// ID_FB_VIEW_ICON
	CFileBrowserListCtrl::VTP_LIST,			// ID_FB_VIEW_LIST
	CFileBrowserListCtrl::VTP_REPORT,		// ID_FB_VIEW_REPORT
	CFileBrowserListCtrl::VTP_SMALLICON,	// ID_FB_VIEW_SMALL_ICON
};

// alphabetical order by ID name; must match Resource.h and message map ranges
const int CFilesBar::m_SortTypeMap[] = {
	CDirList::SORT_LAST_WRITE,	// ID_FB_SORT_DATE
	CDirList::SORT_NAME,		// ID_FB_SORT_NAME
	CDirList::SORT_LENGTH,		// ID_FB_SORT_SIZE
	CDirList::SORT_FILE_TYPE	// ID_FB_SORT_TYPE
};

CFilesBar::CFilesBar()
{
	//{{AFX_DATA_INIT(CFilesBar)
	//}}AFX_DATA_INIT
	m_CurPane = CPersist::GetInt(REG_SETTINGS, RK_FB_CUR_PANE, 0);
	m_CurPane = CLAMP(m_CurPane, 0, PANES - 1);	// in case we remove a pane
	for (int i = 0; i < PANES; i++)
		m_PaneState[i] = m_DefaultPaneState;
	DWORD	sz = sizeof(m_PaneState);
	CPersist::GetBinary(REG_SETTINGS, RK_FB_PANE_STATE, &m_PaneState, &sz);
	m_Main = NULL;
	m_hMainAccel = NULL;
}

CFilesBar::~CFilesBar()
{
	CPersist::WriteInt(REG_SETTINGS, RK_FB_CUR_PANE, m_CurPane);
	CPersist::WriteBinary(REG_SETTINGS, RK_FB_PANE_STATE, m_PaneState, sizeof(m_PaneState));
}

void CFilesBar::OpenFile(LPCTSTR Path, CPoint *DropPoint)
{
	switch (GetCurPane()) {
	case PANE_PROJECTS:
		theApp.OpenDocumentFile(Path);
		break;
	case PANE_PALETTES:
		theApp.GetMain()->OpenPalette(Path, DropPoint);
		break;
	default:
		ASSERT(0);
	}
}

void CFilesBar::DoDataExchange(CDataExchange* pDX)
{
	CFileBrowserControlBar::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFilesBar)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFilesBar, CFileBrowserControlBar)
	//{{AFX_MSG_MAP(CFilesBar)
	ON_WM_DESTROY()
	ON_WM_MENUSELECT()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_INITDIALOG, OnInitDialog)
	ON_WM_ENTERMENULOOP()
	ON_WM_EXITMENULOOP()
	ON_NOTIFY(FBLCN_OPENITEM, IDC_FB_LIST, OnFBOpenItem)
	ON_NOTIFY(FBLCN_RENAMEITEM, IDC_FB_LIST, OnFBRenameItem)
	ON_NOTIFY(FBLCN_DRAGMOVE, IDC_FB_LIST, OnFBDragMove)
	ON_NOTIFY(FBLCN_DRAGBEGIN, IDC_FB_LIST, OnFBDragBegin)
	ON_NOTIFY(FBLCN_DRAGEND, IDC_FB_LIST, OnFBDragEnd)
	ON_NOTIFY(FBLCN_CONTEXTMENU, IDC_FB_LIST, OnFBContextMenu)
	ON_COMMAND_RANGE(ID_FB_VIEW_ICON, ID_FB_VIEW_SMALL_ICON, OnFBViewType)
	ON_UPDATE_COMMAND_UI_RANGE(ID_FB_VIEW_ICON, ID_FB_VIEW_SMALL_ICON, OnUpdateFBViewType)
	ON_COMMAND_RANGE(ID_FB_SORT_DATE, ID_FB_SORT_TYPE, OnFBSortType)
	ON_UPDATE_COMMAND_UI_RANGE(ID_FB_SORT_DATE, ID_FB_SORT_TYPE, OnUpdateFBSortType)
	ON_COMMAND(ID_FB_PARENT_FOLDER, OnFBParentFolder)
	ON_UPDATE_COMMAND_UI(ID_FB_PARENT_FOLDER, OnUpdateFBParentFolder)
	ON_COMMAND(ID_FB_RENAME, OnFBRename)
	ON_UPDATE_COMMAND_UI(ID_FB_RENAME, OnUpdateFBRename)
	ON_UPDATE_COMMAND_UI(ID_FB_FILE_PROPS, OnUpdateFBFileProps)
	ON_COMMAND(ID_FB_FILE_OPEN, OnFBFileOpen)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFilesBar message handlers

LRESULT CFilesBar::OnInitDialog(WPARAM wParam, LPARAM lParam)
{
	CFileBrowserControlBar::OnInitDialog(wParam, lParam);

	m_Main = theApp.GetMain();
	for (int i = 0; i < PANES; i++) {
		const PANE_INFO&	info = PaneInfo[i];
		CString	PaneName;
		PaneName.LoadString(info.NameId);
		InsertPane(i, PaneName, DEFAULT_LIST_STYLE | info.Style);
		CStringArray	ExtFilter;
		ExtFilter.SetSize(info.ExtCount);
		for (int j = 0; j < info.ExtCount; j++)
			ExtFilter[j] = info.ExtFilter[j];
		CFileBrowserListCtrl&	list = GetList(i);
		list.SetExtFilter(ExtFilter);
		ListView_SetExtendedListViewStyleEx(list.m_hWnd, 
			LVS_EX_HEADERDRAGDROP, LVS_EX_HEADERDRAGDROP);
		SetPaneState(i, m_PaneState[i]);
	}
	SetCurPane(m_CurPane);
	SetFolder(PANE_PROJECTS, theApp.m_DocFolder);
	SetFolder(PANE_PALETTES, theApp.m_PaletteFolder);

	return TRUE;
}

void CFilesBar::OnDestroy() 
{
	m_CurPane = GetCurPane();
	for (int i = 0; i < PANES; i++)
		GetPaneState(i, m_PaneState[i]);
	CFileBrowserControlBar::OnDestroy();
}

void CFilesBar::OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu)
{
	// allow our context menu to display hints in the status bar
	m_Main->SendMessage(WM_SETMESSAGESTRING, nItemID);	// show hint for this menu item
}

void CFilesBar::OnEnterMenuLoop(BOOL bIsTrackPopupMenu)
{
	// prevent main's frame counter from overwriting our menu hints
	m_Main->SendMessage(WM_ENTERMENULOOP, bIsTrackPopupMenu);
}

void CFilesBar::OnExitMenuLoop(BOOL bIsTrackPopupMenu)
{
	// restore main's frame counter behavior
	m_Main->SendMessage(WM_EXITMENULOOP, bIsTrackPopupMenu);
}

void CFilesBar::OnFBOpenItem(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMFBOPENITEM pnmoi = (LPNMFBOPENITEM)pNMHDR;
	LPCTSTR	ItemPath = pnmoi->pszPath;
	if (pnmoi->bIsDir) {
		switch (GetCurPane()) {
		case PANE_PROJECTS:
			theApp.m_DocFolder = ItemPath;
			break;
		case PANE_PALETTES:
			theApp.m_PaletteFolder = ItemPath;
			break;
		default:
			ASSERT(0);
		}
	} else
		OpenFile(ItemPath);
}

void CFilesBar::OnFBRenameItem(NMHDR* pNMHDR, LRESULT* pResult)
{
}

void CFilesBar::OnFBDragMove(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW	pnmlv = (LPNMLISTVIEW)pNMHDR;
	CPoint	pt(pnmlv->ptAction);
	CRect	mr, br;
	m_Main->GetWindowRect(mr);
	GetWindowRect(br);
	// if cursor is within main frame but not within file browser
	if (mr.PtInRect(pt) && !br.PtInRect(pt)) {
		SetCursor(AfxGetApp()->LoadCursor(
			GetCurList()->GetSelectedCount() > 1 ?
			IDC_DRAG_MULTI : IDC_DRAG_SINGLE));
	} else
		SetCursor(LoadCursor(NULL, IDC_NO));
}

void CFilesBar::OnFBDragBegin(NMHDR* pNMHDR, LRESULT* pResult)
{
	m_hMainAccel = m_Main->GetAccelTable();	// save main frame's accelerators
	// during drag, give main frame special modal accelerator table, so escape
	// key cancels drag even if file browser list control doesn't have focus
	m_Main->SetAccelTable(LoadAccelerators(theApp.m_hInstance, 
		MAKEINTRESOURCE(IDR_MODAL)));
}

void CFilesBar::OnFBDragEnd(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW	pnmlv = (LPNMLISTVIEW)pNMHDR;
	m_Main->SetAccelTable(m_hMainAccel);	// restore main frame's accelerators
	CRect	fbr, mfr;
	GetWindowRect(fbr);	// get files bar window rect
	m_Main->GetWindowRect(mfr);	// get main frame window rect
	CPoint	DropPt = pnmlv->ptAction;	// drop point in screen coords
	if (mfr.PtInRect(DropPt) && !fbr.PtInRect(DropPt)) {
		CStringArray	PathList;
		GetCurList()->GetSelectedItems(PathList);
		if (PathList.GetSize())
			OpenFile(PathList[0], &DropPt);
	}
}

void CFilesBar::OnFBContextMenu(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW	pnmlv = (LPNMLISTVIEW)pNMHDR;
	int	MenuId;
	if (pnmlv->iItem >= 0)
		MenuId = IDR_FB_CTX_ITEM;
	else
		MenuId = IDR_FB_CTX_BAR;
	CMenu	menu;
	menu.LoadMenu(MenuId);
	CMenu	*mp = menu.GetSubMenu(0);
	CPersistDlg::UpdateMenu(this, &menu);
	CPoint	pt(pnmlv->ptAction);
	GetCurList()->ClientToScreen(&pt);
	mp->TrackPopupMenu(0, pt.x, pt.y, this);
}

void CFilesBar::OnFBViewType(UINT nID)
{
	UINT	idx = nID -= ID_FB_VIEW_ICON;
	ASSERT(idx >= 0 && idx < CFileBrowserListCtrl::VIEW_TYPES);
	GetCurList()->SetViewType(m_ViewTypeMap[idx]);
}

void CFilesBar::OnUpdateFBViewType(CCmdUI* pCmdUI) 
{
	UINT	idx = pCmdUI->m_nID - ID_FB_VIEW_ICON;
	ASSERT(idx >= 0 && idx < CFileBrowserListCtrl::VIEW_TYPES);
	pCmdUI->SetCheck(GetCurList()->GetViewType() == m_ViewTypeMap[idx]);
}

void CFilesBar::OnFBSortType(UINT nID)
{
	UINT	idx = nID -= ID_FB_SORT_DATE;
	ASSERT(idx >= 0 && idx < CDirList::SORT_PROPS);
	GetCurList()->SetSort(m_SortTypeMap[idx], FALSE);
}

void CFilesBar::OnUpdateFBSortType(CCmdUI* pCmdUI) 
{
	UINT	idx = pCmdUI->m_nID - ID_FB_SORT_DATE;
	ASSERT(idx >= 0 && idx < CDirList::SORT_PROPS);
	pCmdUI->SetCheck(GetCurList()->GetSortCol() == m_SortTypeMap[idx]);
}

void CFilesBar::OnFBParentFolder() 
{
	GetCurList()->OpenParentFolder();
}

void CFilesBar::OnUpdateFBParentFolder(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(GetCurList()->HasParentFolder());
}

void CFilesBar::OnFBRename() 
{
	GetCurList()->Rename();
}

void CFilesBar::OnUpdateFBRename(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetCurList()->CanRename());
}

void CFilesBar::OnUpdateFBFileProps(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(FALSE);	// not currently supported
}

void CFilesBar::OnFBFileOpen()
{
	CStringArray	PathList;
	GetCurList()->GetSelectedItems(PathList);
	if (PathList.GetSize())
		OpenFile(PathList[0]);	// support single selection only
}
