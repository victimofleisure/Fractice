// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		05jan09	add SetImage
		02		06jan09	add CacheImage
		03		10jan09	move GetBounds to CSnapshot
		04		11jan09	add OnFilePoster
		05		19jan09	enable cancel only while rendering
		06		01feb09	add color offset
		07		03feb09	add coalesce in SetCycleLength and SetColorOffset
		08		06feb09	add color cycling
		09		07feb09	add get/set escape times
		10		13feb09	allow child snapshot to share parent's escape times
		11		15feb09	add exclusive mode
		12		21feb09	use tweening engine
		13		24feb09	delete escape times from selected items
		14		26feb09	uncache image from selected items
		15		01mar09	remove history navigation handlers
		16		11mar09	move OnImgCancel to main frame
		17		13mar09	add history get/set project
		18		11jul09	allow unlimited record points
		19		12dec09	add GetFrameUID
		20		06jan10	W64: make OnTimer 64-bit compatible

        fractal viewer
 
*/

// FracticeView.h : interface of the CFracticeView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_FRACTICEVIEW_H__73C9C098_36A2_4769_97F5_3155C91D25BF__INCLUDED_)
#define AFX_FRACTICEVIEW_H__73C9C098_36A2_4769_97F5_3155C91D25BF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "HistoryView.h"
#include "TweenEngine.h"
#include "OptionsInfo.h"
#include "Snapshot.h"
#include "Benchmark.h"
#include "DRect.h"
#include "Palette.h"
#include "UndoManager.h"
#include "WndTimer.h"
#include "Recorder.h"

class CMainFrame;
class CPaletteDlg;

class CFracticeView : public CScrollView, public CUndoable, protected CSnapshot
{
// Construction
protected: // create from serialization only
	CFracticeView();
	DECLARE_DYNCREATE(CFracticeView)

// Constants
	enum {	// image size presets; must match m_ImageSizePreset
		ISP_CUSTOM = -1,
		ISP_160_120,
		ISP_320_240,
		ISP_640_480,
		ISP_800_600,
		ISP_1024_768,
		ISP_PRESETS
	};
	enum {	// rendering quality presets; must match m_QualityPreset
		RQP_CUSTOM = -1,
		RQP_DRAFT,
		RQP_NORMAL,
		RQP_HIGH,
		RQP_PRESETS
	};
	enum {	// antialiasing presets; must match m_AntialiasPreset
		AAP_CUSTOM = -1,
		AAP_NONE,
		AAP_2X,
		AAP_3X,
		AAP_4X,
		AAP_PRESETS
	};
	enum {	// undo codes: must match m_UndoTitleID
		UCODE_NONE,
		UCODE_ZOOM,
		UCODE_DRAG,
		UCODE_SCROLL,
		UCODE_FRACTAL_TYPE,
		UCODE_IMAGE_SIZE,
		UCODE_QUALITY,
		UCODE_ANTIALIAS,
		UCODE_COORDS,
		UCODE_BOUNDS,
		UCODE_OPEN_PALETTE,
		UCODE_EDIT_PALETTE,
		UCODE_APPLY_PALETTE,
		UCODE_INVERT_COLOR,
		UCODE_ROTATE_HUE,
		UCODE_HISTORY_RENDER,
		UCODE_HISTORY_DELETE,
		UCODE_HISTORY_CUT,
		UCODE_HISTORY_PASTE,
		UCODE_HISTORY_MOVE,
		UCODE_CYCLE_LEN,
		UCODE_COLOR_OFFSET,
		UCODE_MATH_PREC,
		UCODE_DEEP_ZOOM,
		UCODE_EXPONENT,
		UNDO_CODES,
		UCODE_IMAGE_WIDTH = UCODE_IMAGE_SIZE,
		UCODE_IMAGE_HEIGHT = UCODE_IMAGE_SIZE,
	};
	enum {	// undo flags: combine using bitwise OR
		UFLAG_UNDO		= 0x01,		// direction is undo
		UFLAG_REDO		= 0x02,		// direction is redo
		UFLAG_DIR		= UFLAG_UNDO | UFLAG_REDO	// direction mask
	};
	enum {	// update view hints
		UVH_DEFAULT,
		UVH_EDITOPTIONS,
		UVH_ENABLEWND,
		UVH_EXCLUSIVE,
	};
	enum {	// tool types; must match resource ID order and m_ToolCursorID
		TOOL_HAND,
		TOOL_MARQUEE,
		TOOL_ZOOM,
		TOOLS
	};
	enum {	// timer IDs
		TID_PROGRESS = 1,	// progress timer ID
		TID_CONT_ZOOM,		// continuous zoom timer ID
		TID_COLOR_CYCLE,	// color cycling timer ID
	};
	enum {	// edit flags
		EF_ADD_SNAPSHOT	= 0x01,	// add a snapshot to history
		EF_COALESCE		= 0x02,	// coalesce edit if possible
	};
	enum {	// engine states
		ES_IDLE				= 0,	// engine is idle
		ES_RENDERING		= 1,	// engine is rendering
		ES_COLOR_MAPPING	= -1	// engine is mapping color
	};
	enum {
		COORDS_PRECISION = 6,	// floating-point precision for displaying coordinates
	};

// Types
	class CDeferRender {
	public:
		CDeferRender(CFracticeView& View, bool Render = TRUE);
		~CDeferRender();
		void	SetRender(bool Enable);
		bool	GetRender() const;
	protected:
		CFracticeView&	m_View;
		bool	m_Defer;
		bool	m_Render;
	};

// Attributes
public:
	CFracticeDoc* GetDocument();
	CUndoManager&	GetUndoManager();
	void	GetDefaults(CSnapshot& Snap);
	void	GetZoom(BigNum& Zoom) const;
	void	SetZoom(const BigNum& Zoom);
	void	GetOrigin(BigPoint& Origin) const;
	void	SetOrigin(const BigPoint& Origin);
	void	GetCoords(BigPoint& Origin, BigNum& Zoom) const;
	void	SetCoords(const BigPoint& Origin, const BigNum& Zoom);
	void	SetCoordsPreview(const BigPoint& Origin, const BigNum& Zoom);
	void	GetBounds(BigRect& Bounds) const;
	void	SetBounds(const BigRect& Bounds);
	void	GetSnapshot(CSnapshot& Snap) const;
	const	CSnapshot&	GetSnapshot() const;
	void	SetSnapshot(const CSnapshot& Snap);
	void	SetSnapshotPreview(CSnapshot& Snap, bool AllowRemap = FALSE);
	void	SetSnapshotCached(CSnapshot& Snap, bool AllowRemap = FALSE);
	void	GetProject(CProject& Proj) const;
	void	SetProject(const CProject& Proj);
	CHistoryView	*GetHistoryView() const;
	int		GetHistorySize() const;
	int		GetHistoryPos() const;
	void	SetHistoryPos(int HistIdx, int RenderFlags = 0);
	CHistoryItem&	GetHistoryItem(int HistIdx) const;
	bool	GetAddToHistory() const;
	void	SetAddToHistory(bool Enable);
	UINT	GetFractalType() const;
	void	SetFractalType(UINT Type);
	CSize	GetImageSize() const;
	bool	SetImageSize(CSize Size, UINT Antialias = 0);
	int		GetImageSizePreset() const;
	void	SetImageSizePreset(int PresetIdx);
	UINT	GetQuality() const;
	void	SetQuality(UINT	Quality);
	static	UINT	GetQuality(int PresetIdx);
	int		GetQualityPreset() const;
	void	SetQualityPreset(int PresetIdx);
	int		GetAntialiasPreset() const;
	void	SetAntialiasPreset(int PresetIdx);
	UINT	GetAntialias() const;
	void	SetAntialias(UINT Antialias);
	bool	GetDeepZoom() const;
	void	SetDeepZoom(bool Enable);
	UINT	GetMathPrec() const;
	void	SetMathPrec(UINT Bits);
	CSize	GetThumbSize() const;
	bool	GetFitInWindow() const;
	void	SetFitInWindow(bool Enable);
	int		GetEngineState() const;
	bool	IsIdle() const;
	bool	IsRendering() const;
	bool	IsColorMapping() const;
	UINT	GetFrameUID() const;
	bool	HaveEscapeTimes() const;
	void	GetRecordInfo(CRecordInfo& Info) const;
	void	SetRecordInfo(const CRecordInfo& Info);
	CTweenEngine&	GetEngine();
	WORD	GetColorDepth() const;
	double	GetLastRenderTime() const;
	void	SetLastRenderTime(double Elapsed);
	bool	GetHistogram(CDWordArray& Sample, UINT& MaxVal);
	void	GetPalette(DPalette& Palette) const;
	DPalette&	GetPalette();
	void	SetPalette(const DPalette& Palette, bool AddSnapshot = TRUE);
	UINT	GetCycleLength() const;
	void	SetCycleLength(UINT Length, UINT Flags = EF_ADD_SNAPSHOT);
	double	GetColorOffset() const;
	void	SetColorOffset(double Offset, UINT Flags = EF_ADD_SNAPSHOT);
	bool	GetCycleColors() const;
	void	SetCycleColors(bool Enable);
	double	GetColorCycleRate() const;
	void	SetColorCycleRate(double Rate);
	CDib&	GetImage();
	void	SetImage(CDib& Dib);
	void	GetEscapeTimes(CEscTimeArray& EscTimes) const;
	void	SetEscapeTimes(const CEscTimeArray& EscTimes);
	CUNDOSTATE_VAL(	UValFlags,		WORD,	p.x.w.lo)
	CUNDOSTATE_VAL(	UValViewPos,	int,	p.y.i)

// Operations
public:
	void	OffsetOrigin(const DPoint& Offset, double ZoomScale = 1);
	void	ZoomPoint(CPoint Point, double ZoomScale);
	void	ZoomCenter(double ZoomScale);
	void	Scroll(const DPoint& Offset);
	void	RenderFrame();
	void	CancelRender(bool WaitForIdle = FALSE);
	bool	CreateThumb(CBitmap& Thumb, CRect& Rect);
	bool	ExportBitmap(LPCTSTR Path);
	void	ApplyPalette(UINT Flags = EF_ADD_SNAPSHOT, WORD UndoCode = UCODE_APPLY_PALETTE);
	bool	OpenPalette(LPCTSTR Path, bool Import = FALSE, bool AddSnapshot = TRUE);
	bool	SavePalette(LPCTSTR Path);
	bool	ChoosePaletteFile(bool Import = FALSE);
	void	OffsetColor(double Offset);
	static	void	ShowProperties(const CSnapshot& Snap);
	void	AddHistory();
	void	SetUndoCode(WORD Code, UINT Flags = 0);
	void	CycleColorExclusive();
	void	UpdateColorTimer();
	void	ApplyOptions();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFracticeView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CFracticeView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CFracticeView)
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnDestroy();
	afx_msg void OnEditBounds();
	afx_msg void OnEditCoords();
	afx_msg void OnEditCopy();
	afx_msg void OnEditCut();
	afx_msg void OnEditDelete();
	afx_msg void OnEditPaste();
	afx_msg void OnEditRedo();
	afx_msg void OnEditSelectAll();
	afx_msg void OnEditUndo();
	afx_msg void OnFileExport();
	afx_msg void OnFilePoster();
	afx_msg void OnHistRecord();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnImgCtxZoomIn();
	afx_msg void OnImgCtxZoomOut();
	afx_msg void OnImgCycleColors();
	afx_msg void OnImgDeleteEscTimes();
	afx_msg void OnImgFitInWindow();
	afx_msg void OnImgHistoryExport();
	afx_msg void OnImgPaletteExport();
	afx_msg void OnImgPaletteImport();
	afx_msg void OnImgPaletteOpen();
	afx_msg void OnImgPaletteSave();
	afx_msg void OnImgPalInvertColor();
	afx_msg void OnImgPalRotateHue();
	afx_msg void OnImgRender();
	afx_msg void OnImgUncacheImage();
	afx_msg void OnImgZoomIn();
	afx_msg void OnImgZoomOut();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSnapProperties();
	afx_msg void OnTimer(W64UINT nIDEvent);
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditCut(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditDelete(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditRedo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFilePoster(CCmdUI* pCmdUI);
	afx_msg void OnUpdateImgCycleColors(CCmdUI* pCmdUI);
	afx_msg void OnUpdateImgDeleteEscTimes(CCmdUI* pCmdUI);
	afx_msg void OnUpdateImgFitInWindow(CCmdUI* pCmdUI);
	afx_msg void OnUpdateImgHistoryExport(CCmdUI* pCmdUI);
	afx_msg void OnUpdateImgRender(CCmdUI* pCmdUI);
	afx_msg void OnUpdateImgUncacheImage(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSnapProperties(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewGrid(CCmdUI* pCmdUI);
	afx_msg void OnViewGrid();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnImgTransform();
	afx_msg void OnUpdateImgTransform(CCmdUI* pCmdUI);
	//}}AFX_MSG
	afx_msg void OnImgFractalType(UINT nID);
	afx_msg void OnUpdateImgFractalType(CCmdUI* pCmdUI);
	afx_msg void OnImgImageSize(UINT nID);
	afx_msg void OnUpdateImgImageSize(CCmdUI* pCmdUI);
	afx_msg void OnImgQuality(UINT nID);
	afx_msg void OnUpdateImgQuality(CCmdUI* pCmdUI);
	afx_msg void OnImgAntialias(UINT nID);
	afx_msg void OnUpdateImgAntialias(CCmdUI* pCmdUI);
	afx_msg void OnImgMRUPalette(UINT nID);
	afx_msg void OnUpdateImgMRUPalette(CCmdUI* pCmdUI);
	afx_msg void OnImgMRUImageSize(UINT nID);
	afx_msg void OnUpdateImgMRUImageSize(CCmdUI* pCmdUI);
	afx_msg void OnImgMRUQuality(UINT nID);
	afx_msg void OnUpdateImgMRUQuality(CCmdUI* pCmdUI);
	afx_msg void OnImgMRUAntialias(UINT nID);
	afx_msg void OnUpdateImgMRUAntialias(CCmdUI* pCmdUI);
	afx_msg LRESULT	OnRenderDone(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnStripDone(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnMultiFileSel(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnToolChange(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

// Constants
	static const SIZE m_ImageSizePreset[ISP_PRESETS];	// image size presets
	static const UINT m_QualityPreset[RQP_PRESETS];	// quality presets
	static const UINT m_AntialiasPreset[AAP_PRESETS];	// antialias presets
	static const GRADIENT_POINT	m_DefGradient[];	// default gradient
	static const int	m_DefGradPts;	// default gradient point count
	static const COLORREF	m_DefBkColor;	// default background color
	enum {
		PROG_BAR_HEIGHT = 4,	// progress bar height, in client coords
		PROG_BAR_BORDER = 1,	// progress bar border, in client coords
		DEF_PAL_COLOR_MODE = GRADIENT_HLS,	// default palette color mode
		PROGRESS_PERIOD = 500,	// progress period in milliseconds
		CONT_ZOOM_PERIOD = 100,	// continuous zoom period in milliseconds
	};
	enum {	// continuous zoom states
		CONT_ZOOM_NONE	= 0,	// not zooming
		CONT_ZOOM_IN	= 1,	// zooming in
		CONT_ZOOM_OUT	= -1	// zooming out
	};
	static const int m_UndoTitleID[UNDO_CODES];	// undo title string resource IDs
	static const int m_ToolCursorID[TOOLS];	// tool cursor resource IDs

// Member data
	CMainFrame	*m_Main;		// pointer to main frame window
	CHistoryView	*m_HistView;	// pointer to history view
	CTweenEngine	m_Engine;	// fractal rendering engine
	int		m_EngineState;		// our engine state; see enum above
	CDC		m_FrameDC;			// device context for frame bitmap
	HGDIOBJ	m_FrameDefBmp;		// frame DC's default bitmap
	UINT	m_FrameUID;			// unique frame identifier
	bool	m_HaveEscTimes;		// true if frame's escape times are valid
	CDC		m_BackBufDC;		// back buffer device context
	HGDIOBJ	m_BackBufDefBmp;	// back buffer DC's default bitmap
	CBitmap	m_BackBuf;			// back buffer device-dependent bitmap
	CRect	m_BackBufRect;		// back buffer rectangle, in client coords
	bool	m_FitInWindow;		// true if resizing frame to fit view
	int		m_ImgSizePresetIdx;	// index of image size preset, or -1 if none
	int		m_QualityPresetIdx;	// index of quality preset, or -1 if none
	int		m_AAPresetIdx;		// index of antialiasing preset, or -1 if none
	bool	m_AddToHistory;		// true if render should be added to history
	bool	m_DeferRender;		// true if rendering is deferred
	bool	m_WndEnabled;		// true if window is enabled
	CWndTimer	m_ProgressTimer;	// timer instance for updating progress
	CRect	m_ProgBarRect;		// progress bar rectangle
	bool	m_MarqueeTracking;	// true if marquee is being tracked
	bool	m_Dragging;			// true if image is being dragged
	CPoint	m_DragStart;		// cursor position at start of drag
	CSize	m_DragOffset;		// distance image has been dragged
	HACCEL	m_MainAccel;		// backup of main accelerators during drag
	CSize	m_ThumbSize;		// thumbnail dimensions, in pixels
	CPoint	m_ContextPos;		// context menu's initial cursor position
	bool	m_Previewing;		// true if showing preview image
	DRect	m_PreviewRect;		// portion of frame being previewed
	CSnapshotPtr	m_PreviewSnap;	// pointer to preview snapshot
	COptionsInfo	m_Options;	// options data
	CRecordInfo	m_RecInfo;		// recording parameters
	CBenchmark	m_RenderTimer;	// measures rendering throughput
	double	m_LastRenderTime;	// most recent render time in seconds
	CUndoManager	m_UndoMgr;	// undo manager
	WORD	m_UndoCode;			// current edit's undo code
	UINT	m_UndoFlags;		// current edit's undo flags
	int		m_ContZoom;			// continuous zoom state; see enum above
	CWndTimer	m_ContZoomTimer;	// continuous zoom timer
	bool	m_ShowAxes;			// true if showing axes
	HICON	m_ToolCursor[TOOLS];	// cursor for each tool
	HICON	m_HandGrabCursor;	// cursor while dragging image
	HICON	m_ZoomCursor[2];	// cursors while zooming in/out 
	bool	m_CycleColors;		// true if we're cycling colors
	double	m_ColorCycleRate;	// color cycling rate, in cycles per second
	CWndTimer	m_ColorCycleTimer;	// timer for cycling colors

// Overrides
	void	SaveUndoState(CUndoState& State);
	void	RestoreUndoState(const CUndoState& State);
	void	GetUndoTitle(const CUndoState& State, CString& Title);

// Helpers
	BigPoint	ClientToComplex(const DPoint& Point) const;
	DPoint	ComplexToClient(const BigPoint& Point) const;
	static	int		FindImageSizePreset(CSize Size);
	static	int		FindQualityPreset(UINT Quality);
	static	int		FindAntialiasPreset(UINT Antialias);
	void	GetViewRect(CRect& Rect) const;
	void	UpdatePalette();
	bool	CacheImage(CSnapshot& Snap, CSnapshot *ParentSnap = NULL);
	void	FinishRender();
	bool	CreateBackBuf(CSize Size);
	void	UpdateBackBuf();
	void	UpdateHistView(bool ScrollToEnd = FALSE) const;
	void	InitTools();
	void	Line(CDC *pDC, int x1, int y1, int x2, int y2);
	void	DrawAxes(CDC *pDC, CRect& rb);
	void	SetContZoom(int ZoomType);
	bool	CanCoalesceEdit(WORD UndoCode);
	friend	class	CDeferRender;
	friend	class	CLevelsCtrl;
};

inline void CFracticeView::CDeferRender::SetRender(bool Enable)
{
	m_Render = Enable;
}

inline bool CFracticeView::CDeferRender::GetRender() const
{
	return(m_Render);
}

inline CUndoManager& CFracticeView::GetUndoManager()
{
	return(m_UndoMgr);
}

inline void CFracticeView::GetZoom(BigNum& Zoom) const
{
	Zoom = m_Zoom;
}

inline void CFracticeView::GetOrigin(BigPoint& Origin) const
{
	Origin = m_Origin;
}

inline void CFracticeView::GetCoords(BigPoint& Origin, BigNum& Zoom) const
{
	Origin = m_Origin;
	Zoom = m_Zoom;
}

inline void CFracticeView::GetBounds(BigRect& Bounds) const
{
	CSnapshot::GetBounds(Bounds);
}

inline void CFracticeView::GetSnapshot(CSnapshot& Snap) const
{
	Snap = *this;
}

inline const CSnapshot& CFracticeView::GetSnapshot() const
{
	return(*this);
}

inline UINT CFracticeView::GetFractalType() const
{
	return(m_FractalType);
}

inline CSize CFracticeView::GetImageSize() const
{
	return(m_ImageSize);
}

inline int CFracticeView::GetImageSizePreset() const
{
	return(m_ImgSizePresetIdx);
}

inline UINT CFracticeView::GetQuality() const
{
	return(m_Quality);
}

inline UINT CFracticeView::GetQuality(int PresetIdx)
{
	ASSERT(PresetIdx >= 0 && PresetIdx < RQP_PRESETS);
	return(m_QualityPreset[PresetIdx]);
}

inline int CFracticeView::GetQualityPreset() const
{
	return(m_QualityPresetIdx);
}

inline UINT CFracticeView::GetAntialias() const
{
	return(m_Antialias);
}

inline int CFracticeView::GetAntialiasPreset() const
{
	return(m_AAPresetIdx);
}

inline bool CFracticeView::GetDeepZoom() const
{
	return(m_DeepZoom);
}

inline UINT CFracticeView::GetMathPrec() const
{
	return(m_MathPrec);
}

inline CHistoryView *CFracticeView::GetHistoryView() const
{
	return(m_HistView);
}

inline void CFracticeView::UpdateHistView(bool ScrollToEnd) const
{
	m_HistView->UpdateLayout(ScrollToEnd);
}

inline int CFracticeView::GetHistorySize() const
{
	return(m_HistView->GetItemCount());
}

inline int CFracticeView::GetHistoryPos() const
{
	return(m_HistView->GetViewPos());
}

inline void CFracticeView::SetHistoryPos(int HistIdx, int RenderFlags)
{
	m_HistView->RenderItem(HistIdx, RenderFlags);
}

inline CHistoryItem& CFracticeView::GetHistoryItem(int HistIdx) const
{
	return(m_HistView->GetItem(HistIdx));
}

inline bool CFracticeView::GetAddToHistory() const
{
	return(m_AddToHistory);
}

inline void CFracticeView::SetAddToHistory(bool Enable)
{
	m_AddToHistory = Enable;
}

inline CSize CFracticeView::GetThumbSize() const
{
	return(m_ThumbSize);
}

inline bool CFracticeView::GetFitInWindow() const
{
	return(m_FitInWindow);
}

inline int CFracticeView::GetEngineState() const
{
	return(m_EngineState);
}

inline bool CFracticeView::IsIdle() const
{
	return(m_EngineState == 0);
}

inline bool CFracticeView::IsRendering() const
{
	return(m_EngineState > 0);
}

inline bool CFracticeView::IsColorMapping() const
{
	return(m_EngineState < 0);
}

inline UINT	CFracticeView::GetFrameUID() const
{
	return(m_FrameUID);
}

inline bool CFracticeView::HaveEscapeTimes() const
{
	return(m_HaveEscTimes);
}

inline void CFracticeView::GetRecordInfo(CRecordInfo& Info) const
{
	Info = m_RecInfo;
}

inline void CFracticeView::SetRecordInfo(const CRecordInfo& Info)
{
	m_RecInfo = Info;
}

inline CTweenEngine& CFracticeView::GetEngine()
{
	return(m_Engine);
}

inline WORD CFracticeView::GetColorDepth() const
{
	return(m_Engine.GetDibInfo().BitCount);
}

inline double CFracticeView::GetLastRenderTime() const
{
	return(m_LastRenderTime);
}

inline void CFracticeView::SetLastRenderTime(double Elapsed)
{
	m_LastRenderTime = Elapsed;
}

inline void CFracticeView::GetPalette(DPalette& Palette) const
{
	Palette = m_Palette;
}

inline DPalette& CFracticeView::GetPalette()
{
	return(m_Palette);
}

inline UINT CFracticeView::GetCycleLength() const
{
	return(m_CycleLen);
}

inline double CFracticeView::GetColorOffset() const
{
	return(m_ColorOffset);
}

inline bool CFracticeView::GetCycleColors() const
{
	return(m_CycleColors);
}

inline double CFracticeView::GetColorCycleRate() const
{
	return(m_ColorCycleRate);
}

inline void CFracticeView::SetColorCycleRate(double Rate)
{
	m_ColorCycleRate = Rate;
}

inline CDib& CFracticeView::GetImage()
{
	return(m_Engine.GetDib());
}

inline void CFracticeView::GetEscapeTimes(CEscTimeArray& EscTimes) const
{
	m_Engine.GetEscFrame(EscTimes);
}

inline void CFracticeView::SetUndoCode(WORD Code, UINT Flags)
{
	m_UndoCode = Code;
	m_UndoFlags = Flags;
}

inline void CFracticeView::UpdatePalette()
{
	m_Engine.SetPalette(m_Palette, m_Quality, m_CycleLen, m_ColorOffset);
}

#ifndef _DEBUG  // debug version in FracticeView.cpp
inline CFracticeDoc* CFracticeView::GetDocument()
   { return (CFracticeDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FRACTICEVIEW_H__73C9C098_36A2_4769_97F5_3155C91D25BF__INCLUDED_)
