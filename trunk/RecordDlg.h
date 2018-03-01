// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		11jan09	make queue job checkbox persistent
		02		11feb09	add color cycling
		03		11jul09	allow unlimited record points
		04		03dec09	highlight drop position, add scrolling
		05		17dec09	add select images dialog
		06		06jan10	W64: make OnTimer 64-bit compatible

        record dialog
 
*/

#if !defined(AFX_RECORDDLG_H__85E6BA84_3E5B_48B8_A21E_C197127B024C__INCLUDED_)
#define AFX_RECORDDLG_H__85E6BA84_3E5B_48B8_A21E_C197127B024C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RecordDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRecordDlg dialog

#include "ToolDlg.h"
#include "BigPoint.h"
#include "NumEdit.h"
#include "Snapshot.h"
#include "RecordInfo.h"
#include "ParamsCtrl.h"
#include "CtrlResize.h"
#include "WndTimer.h"

class CRecordDlg : public CToolDlg
{
	DECLARE_DYNAMIC(CRecordDlg);
// Construction
public:
	CRecordDlg(CWnd* pParent = NULL);   // standard constructor
	~CRecordDlg();

// Attributes
	void	SetView(CFracticeView *View);
	void	GetInfo(CRecordInfo& Info);
	void	SetInfo(const CRecordInfo& Info);
	float	GetFrameRate() const;
	int		GetFrameCount() const;
	int		GetSnapCount() const;
	void	GetSnap(int SnapIdx, CSnapshot& Snap) const;
	bool	GetQueueJob() const;
	void	SetSnaps(CSnapshotArray& SnapList);

// Operations
	void	Show();
	void	EnableCtrls(bool Enable);
	int		SnapHitTest(CPoint point) const;
	void	InsertSnap(int SnapIdx, CSnapshot& Snap);
	void	DeleteSnap(int SnapIdx);
	void	InsertSnaps(int SnapIdx, CSnapshotArray& SnapList);
	bool	InsertSnaps();
	void	RemoveAllSnaps();
	void	DragMove(CPoint point);
	void	EndDrag();
	static	void	SecsToTime(int Secs, CString& Time);
	static	int		TimeToSecs(LPCTSTR Time);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRecordDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CRecordDlg)
	enum { IDD = IDD_RECORD };
	CButton	m_DeleteBtn;
	CListCtrl	m_SnapList;
	CNumEdit	m_CyclingRateEdit;
	CButton	m_CycleColorsChk;
	CParamsCtrl	m_ParamsCtrl;
	CButton	m_ToBitmapsChk;
	CNumEdit	m_StartFrameEdit;
	CNumEdit	m_EndFrameEdit;
	CNumEdit	m_FrameRateEdit;
	CEdit	m_DurationEdit;
	int		m_DurationUnit;
	int		m_FrameRange;
	BOOL	m_QueueJob;
	CEdit	m_XEdit;
	CEdit	m_ZEdit;
	CEdit	m_YEdit;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CRecordDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnDurationUnit();
	afx_msg void OnKillfocusDuration();
	afx_msg void OnChangedFrameRate(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnCancel();
	afx_msg void OnFrameRange();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnSnapInsert();
	afx_msg void OnSnapDelete();
	afx_msg void OnSnapProperties();
	afx_msg void OnSelectAll();
	afx_msg void OnUpdateSnapInsert(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSnapDelete(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSelectAll(CCmdUI* pCmdUI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnGetdispinfoSnapList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemchangedSnapList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBegindragSnapList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnTimer(W64UINT nIDEvent);
	//}}AFX_MSG
	afx_msg LRESULT	OnEnableCtrls(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

// Constants
	enum {
		DEF_FRAME_RATE = 30,	// default frame rate
		DEF_DURATION = 30,		// default duration
		DEF_FRAME_COUNT = DEF_DURATION * DEF_FRAME_RATE,	// default frame count
		MIN_POINTS = 2,			// minimum number of record points
		MIN_FRAMES = 2,			// minimum number of frames
		SCROLL_TIMER = 1,		// scroll timer ID
		SCROLL_DELAY = 50		// scroll delay in milliseconds
	};
	enum {	// duration units
		DU_SECONDS,
		DU_FRAMES,
		DURATION_UNITS
	};
	static const CCtrlResize::CTRL_LIST	m_CtrlList[];

// Data members
	CFracticeView	*m_View;	// pointer to active view
	CRecordInfo	m_Info;			// recording parameters
	int		m_Duration;			// recording length in seconds
	int		m_FrameCount;		// recording length in frames
	float	m_FrameRate;		// frame rate in frames per second
	int		m_DefDuration;		// default recording length in seconds
	int		m_DefFrameCount;	// default recording length in frames
	float	m_DefFrameRate;		// default frame rate in frames per second
	int		m_CtxMenuSnapIdx;	// context menu snapshot index 
	bool	m_CtrlsEnabled;		// true if child controls are enabled
	CCtrlResize	m_Resize;		// control resizer
	CRect	m_InitRect;			// initial rectangle in screen coords
	CImageList	m_ImgList;		// image list of snapshot thumbnails
	CDWordArray	m_SnapImgIdx;	// maps snapshots to image list indices
	bool	m_Dragging;			// if true, we're dragging snapshots
	bool	m_QuietListUpdate;	// if true, suppress list update side effects
	CSize	m_ThumbSize;		// list control thumbnail size
	int		m_DropPos;			// current drop position, or -1 if none
	int		m_ScrollDelta;		// scroll by this amount per timer tick
	CWndTimer	m_ScrollTimer;	// timer instance for scrolling

// Helpers
	void	ReadRegVars();
	void	WriteRegVars();
	void	GetDefaults(CRecordInfo& Info) const;
	void	SetDefaults(const CRecordInfo& Info);
	void	ReadCtrls();
	void	InitCtrls();
	bool	UpdateUI();
	int		SecsToFrames(int Secs) const;
	int		FramesToSecs(int Frames) const;
	void	InitSnapList();
	void	SetNum(CWnd& Wnd, const BigNum& Num);
	void	SetCoords(const CSnapshot *pSnap);
	void	Select(int SnapIdx, bool Enable);
	void	SelectAll(bool Enable);
	void	GetSelection(CDWordArray& Selection);
	void	DeleteSelectedItems();
	void	MoveSelectedItems(int DstPos);
	void	UpdateDragCursor(CPoint Point);
	void	UpdateDropPos(CPoint point);
	void	EraseDropPos();
	static	void	CopyParams(CSnapshot& Dst, const CSnapshot& Src);
};

inline int CRecordDlg::GetFrameCount() const
{
	return(m_Info.m_FrameCount);
}

inline float CRecordDlg::GetFrameRate() const
{
	return(m_Info.m_FrameRate);
}

inline int CRecordDlg::SecsToFrames(int Secs) const
{
	return(round(Secs * m_FrameRate));
}

inline int CRecordDlg::FramesToSecs(int Frames) const
{
	return(m_FrameRate > 0 ? round(Frames / m_FrameRate) : 0);
}

inline void CRecordDlg::GetSnap(int SnapIdx, CSnapshot& Snap) const
{
	Snap = m_Info.m_Snap[SnapIdx];
}

inline int CRecordDlg::GetSnapCount() const
{
	return(m_Info.m_Snap.GetSize());
}

inline bool CRecordDlg::GetQueueJob() const
{
	return(m_QueueJob != 0);
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RECORDDLG_H__85E6BA84_3E5B_48B8_A21E_C197127B024C__INCLUDED_)
