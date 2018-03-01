// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		05jan09	add CanRender
		02		13mar09	add get/set project
		03		14mar09	add slide show manager
		04		11jul09	allow unlimited record points
		05		03dec09	highlight drop position
		06		06jan10	W64: make OnTimer 64-bit compatible
		
		history view
 
*/

#if !defined(AFX_HISTORYVIEW_H__DB1C34FD_AA48_4773_A0DA_4D60B1D3A675__INCLUDED_)
#define AFX_HISTORYVIEW_H__DB1C34FD_AA48_4773_A0DA_4D60B1D3A675__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// HistoryView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CHistoryView view

#include "HistoryItem.h"
#include "Clipboard.h"
#include "WndTimer.h"
#include "SlideShowMgr.h"

class CFracticeView;
class CUndoState;
class CProject;

class CHistoryView : public CScrollView
{
protected:
	CHistoryView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CHistoryView)

public:
// Constants
	enum {
		THUMB_MARGIN = 2,
	};
	enum {	// location codes for Goto
		LC_TOP,
		LC_BOTTOM,
		LC_UP,
		LC_DOWN,
		LC_PAGE_UP,
		LC_PAGE_DOWN,
	};
	enum {	// render flags
		RF_UNDOABLE			= 0x01,	// render creates an undo state
		RF_ALLOW_REMAP		= 0x02,	// skip render if only color changed
	};

// Attributes
	void	SetParentView(CFracticeView *View);
	CFracticeView	*GetParentView();
	void	GetProject(CProject& Proj) const;
	void	SetProject(const CProject& Proj);
	void	GetHistory(CHistory& History) const;
	void	SetHistory(const CHistory& History);
	int		GetItemCount() const;
	CHistoryItem&	GetItem(int ItemIdx);
	void	SetItem(int ItemIdx, const CHistoryItem& Item);
	int		GetViewPos() const;
	void	SetViewPos(int ItemIdx, bool AllowRemap = FALSE);
	bool	IsSelected(int ItemIdx) const;
	bool	HaveSelection() const;
	int		GetSelectedCount() const;
	void	GetSelection(CDWordArray& SelIdx) const;
	void	SetSelection(const CDWordArray& SelIdx);
	void	SetSelectionMark(int ItemIdx);
	int		GetSelectionMark() const;
	int		GetCurPos() const;
	int		GetContextMenuPos() const;
	bool	CanPaste() const;
	bool	CanRender() const;
	int		GetCountPerPage() const;
	bool	IsDetailVisible(int ItemIdx) const;
	void	ShowDetail(int ItemIdx, bool Enable);
	CSlideShowMgr&	GetSlideShowMgr();

// Operations
	void	UpdateLayout(int VisibleItem = -1);
	void	EnsureVisible(int ItemIdx, int Count = 1);
	void	RedrawItem(int ItemIdx);
	int		FindItem(CPoint Point) const;
	void	RemoveAll();
	void	AddItem(CHistoryItem& Item, bool DeferUpdate = FALSE);
	void	InsertItem(int ItemIdx, CHistoryItem& Item, bool DeferUpdate = FALSE);
	void	RemoveItem(int ItemIdx, bool DeferUpdate = FALSE);
	void	RenderItem(int ItemIdx, UINT Flags = 0);
	void	SelectItem(int ItemIdx, bool Selected, bool DeferUpdate = FALSE);
	void	ToggleSelect(int ItemIdx);
	void	SelectSpan(int ItemIdx);
	void	SelectAll();
	void	Deselect();
	void	DeleteSelectedItems();
	void	MoveSelectedItems(int ItemIdx);
	bool	Copy();
	bool	Cut();
	bool	Paste();
	bool	Delete();
	void	Render();
	void	AddRecordPoints(int InsPos, bool Replace);
	void	Goto(int LocationCode);
	void	SaveUndoState(CUndoState& State);
	void	RestoreUndoState(const CUndoState& State);
	void	DumpState() const;
	void	DumpHistory() const;

// Nested classes
	class CSelectIter : public CObject {	// selection iterator
	public:
		CSelectIter(CHistoryView& Hist);
		CSnapshot&	operator*() const;
		CSnapshot*	operator->() const;
		bool	More();
		void	operator++();
		int		GetIdx();

	protected:
		CHistoryView&	m_Hist;	// reference to history view
		int		m_CurPos;		// current position in history array
	};

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHistoryView)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnInitialUpdate();     // first time after construct
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CHistoryView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CHistoryView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnTimer(W64UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Types
	class CHistoryItemEx : public CHistoryItem {
	public:
		int		m_OrigPos;		// item's original position in history
	};
	typedef CArrayEx<CHistoryItemEx, CHistoryItemEx&> CHistoryEx;
	class CClipboardUndoState : public CHistoryEx, public CRefObj {
	public:
	};
	class CMoveUndoState : public CRefObj {
	public:
		CDWordArray	m_SelIdx;	// indices of items to be moved
		int		m_DstPos;		// destination position in list
	};
	class CPaletteUndoState : public CRefObj {
	public:
		DPalette	m_Palette;	// saved palette
	};

// Constants
	enum {
		DRAG_THRESHOLD = 2,	// minimum cursor movement to start drag, in client coords
		SCROLL_DELAY = 50,	// milliseconds
	};
	enum {	// timer IDs
		TID_SCROLL = 1000,
		TID_SLIDE_SHOW,
	};
	enum {	// drag states
		DTS_NONE,		// left button is up
		DTS_TRACK,		// left button is down, but motion hasn't reached drag threshold
		DTS_DRAG,		// left button is down, and drag is in progress
	};
	enum {	// drop targets, returned by drop hit test
		DT_NONE,		// can't drop here
		DT_RECORD,		// over record dialog
		DT_HISTORY,		// over history view
		DT_VIEW,		// over main view
	};

// Member data
	CFracticeView	*m_View;	// pointer to view
	CClipboard	m_Clipboard;	// clipboard instance
	CHistory	m_History;	// list of previous view states
	int		m_SelCount;		// number of selected items
	int		m_SelMark;		// selection mark, or -1 if none
	int		m_CurPos;		// current position, or -1 if none
	int		m_ViewPos;		// position of viewed item, or -1 if none
	int		m_Rows;			// number of thumbnail rows
	int		m_Cols;			// number of thumbnail columns
	CSize	m_ThumbSize;	// thumbnail dimensions
	int		m_ContextPos;	// index of context menu thumbnail, or -1 if none
	int		m_DragState;	// drag state; see enum above
	CPoint	m_DragOrigin;	// point where left button was pressed, in client coords
	int		m_DragPos;		// index of thumbnail being dragged
	HICON	m_ViewPosIcon;	// indicates position of viewed item
	CSize	m_ViewPosIconSize;	// dimensions of viewed item icon
	int		m_UndoArg;		// for passing data to SaveUndoState
	int		m_ScrollDelta;		// scroll by this amount per timer tick
	CWndTimer	m_ScrollTimer;	// timer instance for scrolling
	CSlideShowMgr	m_SlideShowMgr;	// slide show manager
	int		m_DropPos;		// drop position, or -1 if none

// Helpers
	CPoint	GetOrigin(int ItemIdx) const;
	void	RestoreClipboardUndoState(const CUndoState& State);
	void	RestoreMoveUndoState(const CUndoState& State);
	void	EndDrag();
	int		DropHitTest(CPoint point);
	int		GetInsertPos(CPoint point);
	void	UpdateDropPos(CPoint point);
	void	EraseDropPos();
	void	DrawDropPos(int Item, bool Enable);
	static	void	MakeArrow(CPoint point, bool left, bool vert, CRgn& rgn);
};

inline CHistoryView::CSelectIter::CSelectIter(CHistoryView& Hist) : m_Hist(Hist)
{
	m_CurPos = 0;
}

inline CSnapshot& CHistoryView::CSelectIter::operator*() const
{
	return(*m_Hist.GetItem(m_CurPos));
}

inline CSnapshot* CHistoryView::CSelectIter::operator->() const
{
	return(m_Hist.GetItem(m_CurPos));
}

inline bool CHistoryView::CSelectIter::More()
{
	while (m_CurPos < m_Hist.GetItemCount() && !m_Hist.IsSelected(m_CurPos))
		m_CurPos++;
	return(m_CurPos < m_Hist.GetItemCount());
}

inline void CHistoryView::CSelectIter::operator++()
{
	m_CurPos++;
}

inline int CHistoryView::CSelectIter::GetIdx()
{
	return(m_CurPos);
}

inline void CHistoryView::SetParentView(CFracticeView *View)
{
	m_View = View;
}

inline CFracticeView *CHistoryView::GetParentView()
{
	return(m_View);
}

inline void CHistoryView::GetHistory(CHistory& History) const
{
	History.Copy(m_History);
}

inline void CHistoryView::SetHistory(const CHistory& History)
{
	m_History.Copy(History);
}

inline int CHistoryView::GetItemCount() const
{
	return(m_History.GetSize());
}

inline CHistoryItem& CHistoryView::GetItem(int ItemIdx)
{
	return(m_History[ItemIdx]);
}

inline void CHistoryView::SetItem(int ItemIdx, const CHistoryItem& Item)
{
	m_History[ItemIdx] = Item;
}

inline int CHistoryView::GetViewPos() const
{
	return(m_ViewPos);
}

inline bool CHistoryView::IsSelected(int ItemIdx) const
{
	return(m_History[ItemIdx].m_Selected);
}

inline bool CHistoryView::HaveSelection() const
{
	return(m_SelCount > 0);
}

inline int CHistoryView::GetSelectedCount() const
{
	return(m_SelCount);
}

inline int CHistoryView::GetCurPos() const
{
	return(m_CurPos);
}

inline bool CHistoryView::CanPaste() const
{
	return(m_Clipboard.HasData());
}

inline int CHistoryView::GetContextMenuPos() const
{
	return(m_ContextPos);
}

inline bool CHistoryView::IsDetailVisible(int ItemIdx) const
{
	return(m_History[ItemIdx].m_ShowDetail);
}

inline CSlideShowMgr& CHistoryView::GetSlideShowMgr()
{
	return(m_SlideShowMgr);
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HISTORYVIEW_H__DB1C34FD_AA48_4773_A0DA_4D60B1D3A675__INCLUDED_)
