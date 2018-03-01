// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		05jan09	add SetImage, call in CancelRender
		02		06jan09	in OnRenderDone, if history item, cache image
		03		10jan09	move GetBounds to CSnapshot
		04		11jan09	add OnFilePoster
		05		19jan09	enable cancel only while rendering
		06		01feb09	add color offset
		07		03feb09	in SetSnapshotPreview, add color offset
		08		03feb09	add coalesce in SetCycleLen and SetColorOffset
		09		06feb09	add color cycling
		10		07feb09	in OnInitialUpdate, restore escape times
		11		13feb09	allow child snapshot to share parent's escape times
		12		15feb09	add exclusive mode
		13		18feb09	include imported palettes in recent palettes list
		14		20feb09	don't let palette shrink during exclusive render
		15		21feb09	in ShowProperties, use single-threaded modeless message box
		16		24feb09	delete escape times from selected items
		17		26feb09	uncache image from selected items
		18		01mar09	remove history navigation handlers
		19		11mar09	move OnImgCancel to main frame
		20		13mar09	add history get/set project
		21		14mar09	in OnUpdate, if disabling, stop slide show
		22		15mar09	in OnTimer, if cycling color and no escape times, render
		23		16mar09	in OnDraw, if previewing empty thumbnail, paint it black
		24		11jul09	allow unlimited record points
		25		23jul09	add ID for last fractal type
		26		02dec09	merge multi-leg recording into trunk
		27		04dec09	in OffsetOrigin, convert to bignum before divide by zoom
		28		22dec09	add Wrap for reverse color cycling
		29		27dec09	in OnRenderDone, add PostRender
		30		06jan10	W64: make OnTimer 64-bit compatible
		31		06jan10	W64: in OnStripDone, cast thread index to 32-bit

        fractal viewer
 
*/

// FracticeView.cpp : implementation of the CFracticeView class
//

#include "stdafx.h"
#include "Fractice.h"

#include "FracticeDoc.h"
#include "FracticeView.h"

#include <math.h>
#include <float.h>	// for _isnan
#include "MainFrm.h"
#include "CoordsDlg.h"
#include "BoundsDlg.h"
#include "ImageSizeDlg.h"
#include "QualityDlg.h"
#include "AntialiasDlg.h"
#include "PathStr.h"
#include "PaletteDlg.h"
#include "ParamsBar.h"
#include "MultiFileDlg.h"
#include "MsgBoxDlg.h"
#include "PosterDlg.h"
#include "RectTrackerEx.h"
#include "RenderingDlg.h"
#include "FolderDialog.h"
#include "TransformDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFracticeView

IMPLEMENT_DYNCREATE(CFracticeView, CScrollView)

const SIZE CFracticeView::m_ImageSizePreset[ISP_PRESETS] = {	// must match enum
	{320,	240},
	{640,	480},
	{800,	600},
	{1024,	768},
};

const UINT CFracticeView::m_QualityPreset[RQP_PRESETS] = {	// must match enum
	128,	// draft
	1024,	// normal
	4096,	// high
};

const UINT CFracticeView::m_AntialiasPreset[AAP_PRESETS] = {	// must match enum
	1,		// none
	2,		// 2x
	3,		// 3x
	4,		// 4x
};

#define MRU_IMAGE_SIZE_FMT	_T("%d x %d")
#define MRU_QUALITY_FMT		_T("%d")
#define MRU_ANTIALIAS_FMT	_T("%dx")

const GRADIENT_POINT CFracticeView::m_DefGradient[] = {
	{0.0,	{2.0 / 3,	0.5,	1.0}},	// blue at 240 degrees
	{1.0,	{-1.0 / 3,	0.5,	1.0}},	// blue at -120 (360 counter-clockwise)
};
const int CFracticeView::m_DefGradPts = sizeof(m_DefGradient) / sizeof(GRADIENT_POINT);
const COLORREF CFracticeView::m_DefBkColor = RGB(0, 0, 0);

#define SHOW_CROSSHAIR	0

#define RK_FIT_ON_SCREEN	_T("FitInWindow")
#define RK_SHOW_AXES		_T("ShowAxes")

#define ID_LAST_FRACTAL_TYPE	(ID_IMG_FRACTAL_TYPE_1 + FRACTAL_TYPES - 1)

const int CFracticeView::m_UndoTitleID[UNDO_CODES] = {	// must match enum
	0,	// UCODE_NONE
	IDS_UC_ZOOM,
	IDS_UC_DRAG,
	IDS_UC_SCROLL,
	IDS_PARM_FRACTAL_TYPE,
	IDS_UC_IMAGE_SIZE,
	IDS_PARM_QUALITY,
	IDS_PARM_ANTIALIAS,
	IDS_UC_COORDS,
	IDS_UC_BOUNDS,
	IDS_UC_OPEN_PALETTE,
	IDS_UC_EDIT_PALETTE,
	IDS_UC_APPLY_PALETTE,
	IDS_UC_INVERT_COLOR,
	IDS_UC_ROTATE_HUE,
	IDS_UC_HISTORY_RENDER,
	IDS_UC_HISTORY_DELETE,
	IDS_UC_HISTORY_CUT,
	IDS_UC_HISTORY_PASTE,
	IDS_UC_HISTORY_MOVE,
	IDS_PARM_CYCLE_LEN,
	IDS_PARM_COLOR_OFFSET,
	IDS_PARM_MATH_PREC,
	IDS_PARM_DEEP_ZOOM,
	IDS_PARM_EXPONENT,
};

const int CFracticeView::m_ToolCursorID[TOOLS] = {	// must match enum
	{IDC_TOOL_HAND_OPEN},	// hand
	{0},					// marquee (default arrow)
	{IDC_TOOL_ZOOM},		// zoom
};

/////////////////////////////////////////////////////////////////////////////
// CFracticeView construction/destruction

CFracticeView::CFracticeView() :
	m_UndoMgr(NULL)
{
	m_Main = NULL;
	m_HistView = NULL;
	m_EngineState = ES_IDLE;
	m_FrameDefBmp = NULL;
	m_FrameUID = 0;
	m_HaveEscTimes = FALSE;
	m_BackBufDefBmp = NULL;
	m_BackBufRect.SetRectEmpty();
	m_FitInWindow = theApp.RdRegBool(RK_FIT_ON_SCREEN, TRUE);
	m_ImgSizePresetIdx = 0;
	m_QualityPresetIdx = 0;
	m_AAPresetIdx = AAP_NONE;
	m_AddToHistory = FALSE;
	m_DeferRender = FALSE;
	m_WndEnabled = TRUE;
	m_ProgBarRect.SetRectEmpty();
	m_MarqueeTracking = FALSE;
	m_Dragging = FALSE;
	m_DragStart = CPoint(0, 0);
	m_DragOffset = CSize(0, 0);
	m_MainAccel = NULL;
	m_ThumbSize = CSize(0, 0);
	m_ContextPos = CPoint(0, 0);
	m_Previewing = FALSE;
	m_PreviewRect.SetRectEmpty();
	m_LastRenderTime = 0;
	m_UndoMgr.SetRoot(this);
	m_UndoCode = 0;
	m_UndoFlags = 0;
	m_ContZoom = 0;
	m_ShowAxes = theApp.RdRegBool(RK_SHOW_AXES);
	ZeroMemory(m_ToolCursor, sizeof(m_ToolCursor));
	m_HandGrabCursor = NULL;
	ZeroMemory(m_ZoomCursor, sizeof(m_ZoomCursor));
	m_CycleColors = FALSE;
	m_ColorCycleRate = m_RecInfo.m_ColorCycleRate;
}

CFracticeView::~CFracticeView()
{
	theApp.WrRegBool(RK_FIT_ON_SCREEN, m_FitInWindow);
	theApp.WrRegBool(RK_SHOW_AXES, m_ShowAxes);
}

static inline double Wrap(double Val, double Limit)
{
	double	r = fmod(Val, Limit);
	return(Val < 0 ? r + Limit : r);
}

CFracticeView::CDeferRender::CDeferRender(CFracticeView& View, bool Render) :
	m_View(View)
{
	m_Defer = m_View.m_DeferRender;
	m_Render = Render;
	m_View.m_DeferRender = TRUE;
}

CFracticeView::CDeferRender::~CDeferRender()
{
	m_View.m_DeferRender = m_Defer;
	if (!m_Defer && m_Render)
		m_View.RenderFrame();
}

void CFracticeView::GetDefaults(CSnapshot& Snap)
{
	CString	DefSnap = m_Options.m_DefSnapshot;
	if (!DefSnap.IsEmpty() && Snap.Read(DefSnap))
		return;
	Snap.SetMathPrec(DEF_MATH_PREC);	// order matters
	Snap.m_Origin.x = 0;
	Snap.m_Origin.y = 0;
	Snap.m_Zoom = 1.0;
	Snap.m_FractalType = DEF_FRACTAL_TYPE;
	Snap.m_ImageSize = CSize(DEF_IMAGE_WIDTH, DEF_IMAGE_HEIGHT);
	Snap.m_Quality = DEF_QUALITY;
	Snap.m_Antialias = DEF_ANTIALIAS;
	Snap.m_CycleLen = DEF_CYCLE_LEN;
	Snap.m_DeepZoom = FALSE;
	Snap.m_Exponent = DEF_EXPONENT;
	Snap.m_Palette.SetSize(m_DefGradPts);
	for (int i = 0; i < m_DefGradPts; i++)
		Snap.m_Palette[i] = m_DefGradient[i];
	Snap.m_Palette.m_BkColor = m_DefBkColor;
	Snap.m_Palette.m_ColorMode = DEF_PAL_COLOR_MODE;
}

void CFracticeView::SetZoom(const BigNum& Zoom)
{
	m_Zoom = Zoom;
	RenderFrame();
}

void CFracticeView::SetOrigin(const BigPoint& Origin)
{
	m_Origin = Origin;
	RenderFrame();
}

void CFracticeView::SetSnapshot(const CSnapshot& Snap)
{
	CDeferRender	defer(*this);	// defer render until out of scope
	SetMathPrec(Snap.m_MathPrec);	// order matters; do before setting bignums
	m_Origin = Snap.m_Origin;
	m_Zoom = Snap.m_Zoom;
	m_FractalType = Snap.m_FractalType;
	m_Palette = Snap.m_Palette;
	m_CycleLen = Snap.m_CycleLen;
	m_ColorOffset = Snap.m_ColorOffset;
	m_DeepZoom = Snap.m_DeepZoom;
	m_Exponent = Snap.m_Exponent;
	SetImageSize(Snap.m_ImageSize, Snap.m_Antialias);
	SetAntialias(Snap.m_Antialias);
	if (Snap.m_Quality != m_Quality)	// if quality changed
		SetQuality(Snap.m_Quality);		// updates palette as a side effect
	else
		UpdatePalette();
	m_Main->GetPaletteDlg().InitCtrls();
}

void CFracticeView::SetSnapshotPreview(CSnapshot& Snap, bool AllowRemap)
{
	if (AllowRemap && m_HaveEscTimes && !IsRendering()) {
		CSnapshot	CurSnap;
		GetSnapshot(CurSnap);
		// if new snapshot matches current state except for colors
		if (Snap.EqualIgnoreColor(CurSnap)) {
			m_CycleLen = Snap.m_CycleLen;
			m_ColorOffset = Snap.m_ColorOffset;
			m_Palette.RemoveAll();	// spoof no-op test in SetPalette
			SetPalette(Snap.m_Palette, FALSE);	// remap colors
			if (IsColorMapping())
				m_PreviewSnap = &Snap;	// so OnRenderDone can cache image
			return;
		}
	}
	SetSnapshot(Snap);
	m_PreviewSnap = &Snap;
	m_Previewing = FALSE;	// discard previous transformation if any
	m_AddToHistory = FALSE;
}

void CFracticeView::SetSnapshotCached(CSnapshot& Snap, bool AllowRemap)
{
	if (!Snap.HasImage()) {	// if cached image not available
		if (Snap.HasEscTimes()) {	// if escape times were saved
			{
				CDeferRender	defer(*this, FALSE);	// don't render at all
				SetSnapshotPreview(Snap);	// restore parameters
			}
			SetEscapeTimes(Snap.m_EscTimes);	// restore escape times
			ApplyPalette(0);	// recreate image from escape times
		} else	// no escape times either
			SetSnapshotPreview(Snap, AllowRemap);	// re-render from scratch
	} else {	// image is cached in history, no need to re-render it
		if (m_Main->IsExclusive()) {
			// in exclusive mode, if snapshot doesn't have escape times, we must
			// re-render even though image is cached, else display won't update
			if (!Snap.HasEscTimes()) {	// if escape times weren't saved
				SetSnapshotPreview(Snap, AllowRemap);	// re-render
				return;	// early out
			}
			// not rendering, so force engine to rebuild palette
			m_Engine.ResetPalette();
		}
		CDeferRender	defer(*this, FALSE);	// don't render at all
		SetSnapshot(Snap);	// restore parameters
		m_Main->OnParamChange();	// update UI
		SetImage(Snap.m_Image);	// restore cached image
		if (Snap.HasEscTimes())	// if escape times were saved
			SetEscapeTimes(Snap.m_EscTimes);	// restore them too
		else	// no escape times
			m_Main->GetLevelsBar().SetEmpty();	// reset histogram
	}
}

void CFracticeView::GetProject(CProject& Proj) const
{
	Proj.m_ThumbSize = m_ThumbSize;
	Proj.m_CycleColors = m_CycleColors;
	Proj.m_ColorCycleRate = m_ColorCycleRate;
	m_Main->GetRecordDlg().GetInfo(Proj.m_RecInfo);
	m_HistView->GetProject(Proj);
}

void CFracticeView::SetProject(const CProject& Proj)
{
	m_ThumbSize = Proj.m_ThumbSize;	// set thumbnail size
	m_CycleColors = Proj.m_CycleColors;	// order matters; do before rendering
	m_ColorCycleRate = Proj.m_ColorCycleRate;
	m_Main->GetRecordDlg().SetInfo(Proj.m_RecInfo);
	m_HistView->SetProject(Proj);
}

int CFracticeView::FindImageSizePreset(CSize Size)
{
	for (int i = 0; i < ISP_PRESETS; i++) {
		if (Size == m_ImageSizePreset[i])
			return(i);
	}
	return(ISP_CUSTOM);
}

void CFracticeView::SetFractalType(UINT Type)
{
	ASSERT(Type < FRACTAL_TYPES);
	if (Type == m_FractalType)
		return;	// nothing to do
	m_FractalType = Type;
	RenderFrame();
}

bool CFracticeView::SetImageSize(CSize Size, UINT Antialias)
{
	if (Size == m_ImageSize)
		return(TRUE);	// nothing to do
	if (m_Engine.GetSSE2() && (Size.cx & 1))	// if SSE2 and odd width
		AfxMessageBox(IDS_SSE2_ODD_WIDTH);	// warn that width will be changed
	if (!Antialias)	// if antialiasing not specified
		Antialias = m_Antialias;	// use current value
	if (!m_Engine.CreateFrame(Size, CRecordInfo::DEF_BIT_COUNT, Antialias)) {
		AfxMessageBox(IDS_CANT_CREATE_FRAME);
		return(FALSE);
	}
	int	HistItems = GetHistorySize();
	if (!IsRendering() && !m_DeferRender && HistItems > 0)
		m_PreviewSnap = GetHistoryItem(HistItems - 1);	// easy preview case
	Size = m_Engine.GetDibInfo().Size;	// in case width was changed
	m_ImageSize = Size;
	m_ImgSizePresetIdx = FindImageSizePreset(Size);
	if (m_ImgSizePresetIdx < 0) {	// if custom value
		CString	s;
		s.Format(MRU_IMAGE_SIZE_FMT, Size.cx, Size.cy);
		m_Main->GetMRUImgSize().Add(s);	// add to most recently used
	}
	SelectObject(m_FrameDC, m_Engine.GetDib());
	if (m_FitInWindow)
		CreateBackBuf(Size);
	else
		SetScrollSizes(MM_TEXT, Size);
	RenderFrame();
	return(TRUE);
}

void CFracticeView::SetImageSizePreset(int PresetIdx)
{
	ASSERT(PresetIdx >= 0 && PresetIdx < ISP_PRESETS);
	if (PresetIdx == m_ImgSizePresetIdx)
		return;	// nothing to do
	SetImageSize(m_ImageSizePreset[PresetIdx]);
}

int CFracticeView::FindQualityPreset(UINT Quality)
{
	for (int i = 0; i < RQP_PRESETS; i++) {
		if (Quality == m_QualityPreset[i])
			return(i);
	}
	return(RQP_CUSTOM);
}

void CFracticeView::SetQuality(UINT Quality)
{
	ASSERT(Quality > 0 && Quality <= MAX_QUALITY);
	if (Quality == m_Quality)
		return;	// nothing to do
	m_Quality = Quality;
	m_QualityPresetIdx = FindQualityPreset(Quality);
	if (m_QualityPresetIdx < 0) {	// if custom value
		CString	s;
		s.Format(MRU_QUALITY_FMT, Quality);
		m_Main->GetMRUQuality().Add(s);	// add to most recently used
	}
	UpdatePalette();
	RenderFrame();
}

void CFracticeView::SetQualityPreset(int PresetIdx)
{
	ASSERT(PresetIdx >= 0 && PresetIdx < RQP_PRESETS);
	if (PresetIdx == m_QualityPresetIdx)
		return;	// nothing to do
	SetQuality(m_QualityPreset[PresetIdx]);
}

int CFracticeView::FindAntialiasPreset(UINT Antialias)
{
	for (int i = 0; i < AAP_PRESETS; i++) {
		if (Antialias == m_AntialiasPreset[i])
			return(i);
	}
	return(AAP_CUSTOM);
}

void CFracticeView::SetAntialias(UINT Antialias)
{
	ASSERT(Antialias > 0 && Antialias <= MAX_ANTIALIAS);
	if (Antialias == m_Antialias)
		return;	// nothing to do
	m_Antialias = Antialias;
	m_AAPresetIdx = FindAntialiasPreset(Antialias);
	if (m_AAPresetIdx < 0) {	// if custom value
		CString	s;
		s.Format(MRU_ANTIALIAS_FMT, Antialias);
		m_Main->GetMRUAntialias().Add(s);	// add to most recently used
	}
	m_Engine.SetAntialias(Antialias);
	RenderFrame();
}

void CFracticeView::SetAntialiasPreset(int PresetIdx)
{
	ASSERT(PresetIdx >= 0 && PresetIdx < AAP_PRESETS);
	if (PresetIdx == m_AAPresetIdx)
		return;	// nothing to do
	SetAntialias(PresetIdx + 1);
}

void CFracticeView::SetDeepZoom(bool Enable)
{
	if (Enable == m_DeepZoom)
		return;	// nothing to do
	m_DeepZoom = Enable;
	RenderFrame();
}

void CFracticeView::SetMathPrec(UINT Bits)
{
	if (Bits == m_MathPrec)
		return;	// nothing to do
	m_Engine.CancelRender(TRUE);	// wait for engine to be idle
	m_Engine.SetMathPrec(Bits);	// set engine's math precision
	mpf_set_default_prec(Bits);	// set GMP's default math precision
	// set math precision of all BigNum-derived member vars here
	m_Origin.SetPrec(Bits);
	m_Zoom.SetPrec(Bits);
	m_MathPrec = Bits;
	RenderFrame();
}

void CFracticeView::SetFitInWindow(bool Enable)
{
	if (Enable == m_FitInWindow)
		return;	// nothing to do
	m_FitInWindow = Enable;
	if (Enable) {
		CRect	rc;
		GetClientRect(rc);
		HBITMAP	PrevBmp = m_BackBuf;	// save bitmap handle
		CreateBackBuf(rc.Size());
		// create does nothing unless frame size has changed
		if (m_BackBuf == PrevBmp)	// if create was a no-op
			UpdateBackBuf();			// update back buffer
		Invalidate();	// order matters
		SetScrollSizes(MM_TEXT, rc.Size());
	} else {
		Invalidate();
		SetScrollSizes(MM_TEXT, m_ImageSize);
	}
}

void CFracticeView::SetPalette(const DPalette& Palette, bool AddSnapshot)
{
	if (Palette == m_Palette)
		return;	// nothing to do
	m_Palette = Palette;
	ApplyPalette(AddSnapshot ? EF_ADD_SNAPSHOT : 0);
	m_Main->GetPaletteDlg().InitCtrls();
}

void CFracticeView::SetCycleLength(UINT Length, UINT Flags)
{
	if (Length == m_CycleLen)
		return;	// nothing to do
	m_CycleLen = Length;
	ApplyPalette(Flags, UCODE_CYCLE_LEN);
}

void CFracticeView::SetColorOffset(double Offset, UINT Flags)
{
	if (Offset == m_ColorOffset)
		return;	// nothing to do
	m_ColorOffset = Offset;
	ApplyPalette(Flags, UCODE_COLOR_OFFSET);
}

void CFracticeView::OffsetColor(double Offset)
{
	m_ColorOffset = Offset;
	if (!m_DeferRender)
		m_Engine.UpdatePalette(m_Palette, m_CycleLen, m_ColorOffset);
}

bool CFracticeView::CanCoalesceEdit(WORD UndoCode)
{
	if (m_DeferRender || IsRendering() || !m_HaveEscTimes)
		return(FALSE);
	int	undos = m_UndoMgr.GetSize();
	return(undos > 0 && m_UndoMgr.GetPos() == undos 
		&& m_UndoMgr.GetState(undos - 1).GetCode() == UndoCode);
}

void CFracticeView::ApplyPalette(UINT Flags, WORD UndoCode)
{
	UpdatePalette();
	if (m_DeferRender)
		return;
	if (!IsRendering()) {
		if (m_HaveEscTimes) {
			static const BigRect	EmptyBounds;
			RENDER_INFO	ri;
			ZeroMemory(&ri, sizeof(ri));
			ri.FractalType = FT_NO_RENDER;	// color mapping only
			ri.FrameUID = ++m_FrameUID;
			m_Engine.RenderFrame(ri, EmptyBounds);
			m_EngineState = ES_COLOR_MAPPING;
			m_RenderTimer.Reset();
			m_Main->OnParamChange();	// so params bar snapshot has new palette
		} else
			RenderFrame();
	} 
	m_AddToHistory = (Flags & EF_ADD_SNAPSHOT);
	m_UndoCode = UndoCode;
	if ((Flags & EF_COALESCE) && CanCoalesceEdit(UndoCode))
		m_UndoFlags = UE_COALESCE;
	else
		m_UndoFlags = 0;
}

BigPoint CFracticeView::ClientToComplex(const DPoint& Point) const
{
	DPoint	pt(Point);
	DPoint	FrameSize(m_ImageSize);
	if (m_FitInWindow) {
		DRect	bbr(m_BackBufRect);
		pt -= bbr.CenterPoint();
		pt *= m_ImageSize.cx / bbr.Width();
	} else {
		pt += GetScrollPosition();
		pt -= FrameSize / 2;
	}
	DPoint	scale(GetFrameAspect() * GetNominalSize(), GetNominalSize());
	pt *= scale / FrameSize;
	BigPoint	cp(m_MathPrec, 0, 0);
	cp.x = pt.x;
	cp.y = pt.y;
	cp /= m_Zoom;
	cp.x += m_Origin.x;
	cp.y = m_Origin.y - cp.y;	// flip y-axis
	return(cp);
}

DPoint CFracticeView::ComplexToClient(const BigPoint& Point) const
{
	BigPoint	cp(m_MathPrec, 0, 0);
	cp = Point;
	cp.x -= m_Origin.x;
	cp.y = m_Origin.y - cp.y;	// flip y-axis
	cp *= m_Zoom;
	DPoint	pt(cp.x, cp.y);
	DPoint	FrameSize(m_ImageSize);
	DPoint	scale(GetFrameAspect() * GetNominalSize(), GetNominalSize());
	pt /= scale / FrameSize;
	if (m_FitInWindow) {
		DRect	bbr(m_BackBufRect);
		pt /= m_ImageSize.cx / bbr.Width();
		pt += bbr.CenterPoint();
	} else {
		pt += FrameSize / 2;
		pt -= GetScrollPosition();
	}
	return(pt);
}

void CFracticeView::GetViewRect(CRect& Rect) const
{
	if (m_FitInWindow)
		Rect = m_BackBufRect;
	else	// frame is in client coords 
		Rect = CRect(-GetScrollPosition(), m_ImageSize);
}

void CFracticeView::OffsetOrigin(const DPoint& Offset, double ZoomScale)
{
	DPoint	FrameOfs(Offset);
	if (m_FitInWindow)
		FrameOfs /= m_BackBufRect.Width() / double(m_ImageSize.cx);
	// calculate preview rectangle
	DRect	rp(m_ImageSize);
	double	PrevScale = 1;
	if (m_Previewing) {
		rp = m_PreviewRect;
		PrevScale = m_PreviewRect.Width() / double(m_ImageSize.cx);
	}
	rp += FrameOfs * PrevScale;
	rp.Scale(rp.CenterPoint(), 1 / ZoomScale);
	m_PreviewRect = rp;
	m_Previewing = TRUE;
	// offset fractal origin and scale zoom
	DPoint	OriginOfs = FrameOfs / m_ImageSize * GetNominalSize();
	OriginOfs.x *= GetFrameAspect();
	BigPoint	BigOrgOfs(m_MathPrec, OriginOfs.x, OriginOfs.y, 0);
	m_Origin.x += BigOrgOfs.x / m_Zoom;
	m_Origin.y -= BigOrgOfs.y / m_Zoom;	// flip y-axis
	m_Zoom *= ZoomScale;
	RenderFrame();
}

void CFracticeView::ZoomPoint(CPoint Point, double ZoomScale)
{
	CRect	rv;
	GetViewRect(rv);
	DPoint	ofs(Point - rv.CenterPoint());
	double	r;
	if (ZoomScale > 1)
		r = (ZoomScale - 1) / ZoomScale;
	else
		r = 1 - 1 / ZoomScale;
	ofs *= r;
	OffsetOrigin(ofs, ZoomScale);
}

void CFracticeView::ZoomCenter(double ZoomScale)
{
	if (m_FitInWindow)
		OffsetOrigin(DPoint(0, 0), ZoomScale);	// origin is center of view
	else {
		CRect	rc;
		GetClientRect(rc);
		// if frame is smaller than client, use frame center
		if (rc.Width() > m_ImageSize.cx)
			rc.right = m_ImageSize.cx;
		if (rc.Height() > m_ImageSize.cy)
			rc.bottom = m_ImageSize.cy;
		ZoomPoint(rc.CenterPoint(), ZoomScale);
	}
}

void CFracticeView::Scroll(const DPoint& Offset)
{
	SetUndoCode(UCODE_SCROLL, CanCoalesceEdit(UCODE_SCROLL) ? UE_COALESCE : 0);
	OffsetOrigin(Offset);
}

void CFracticeView::SetCoords(const BigPoint& Origin, const BigNum& Zoom)
{
	m_Origin = Origin;
	m_Zoom = Zoom;
	RenderFrame();
}

void CFracticeView::SetCoordsPreview(const BigPoint& Origin, const BigNum& Zoom)
{
	{
		double	ZoomScale(Zoom / m_Zoom);
		BigPoint	delta((Origin - m_Origin) * m_Zoom);
		DPoint	pt(delta.x, -delta.y);	// flip y-axis
		DPoint	scale(GetFrameAspect() * GetNominalSize(), GetNominalSize());
		pt /= scale / m_ImageSize;
		if (m_FitInWindow) {
			DRect	bbr(m_BackBufRect);
			pt /= m_ImageSize.cx / bbr.Width();
		}
		CDeferRender	defer(*this, FALSE);	// don't render at all
		OffsetOrigin(pt, ZoomScale);	// set preview rectangle
	}
	m_Origin = Origin;
	m_Zoom = Zoom;
	RenderFrame();
}

void CFracticeView::SetBounds(const BigRect& Bounds)
{
	BigPoint	delta(Bounds.Size());
	BigPoint	Origin(Bounds.p1() + delta / 2);
	BigNum	Zoom(GetNominalSize() / delta.x * GetFrameAspectBig());
	SetCoordsPreview(Origin, Zoom);
}

void CFracticeView::RenderFrame()
{
	if (m_DeferRender)
		return;
	BigRect	Bounds(m_MathPrec, 0, 0);
	GetBounds(Bounds);
	RENDER_INFO	ri;
	ri.FrameBuf = m_Engine.GetEscFrame();
	ri.FractalType = m_FractalType;
	ri.FrameSize = m_Engine.GetEscFrameSize();
	ri.FrameUID = ++m_FrameUID;
	ri.Quality = m_Quality;
	ri.DeepZoom = m_DeepZoom;
	ri.Exponent = m_Exponent;
	if (m_Main->IsDistributed() && !m_Main->IsRecording())
		m_Engine.BeginStripJob(ri, Bounds, *this);
	else
		m_Engine.RenderFrame(ri, Bounds);
	m_RenderTimer.Reset();
	m_EngineState = ES_RENDERING;
	m_HaveEscTimes = FALSE;
	m_AddToHistory = TRUE;
	m_Main->OnParamChange();
	Invalidate();
	if (!(m_Main->IsRecording() || m_ContZoom))
		m_ProgressTimer.Run(TRUE);
}

void CFracticeView::FinishRender()
{
	m_EngineState = ES_IDLE;
	m_PreviewSnap.SetEmpty();
	m_Previewing = FALSE;
	m_ProgressTimer.Run(FALSE);
}

void CFracticeView::CancelRender(bool WaitForIdle)
{
	if (IsIdle())
		return;
	m_Engine.CancelRender(WaitForIdle);
	m_FrameUID++;	// ignore frame already queued
	FinishRender();
	Invalidate();
	int	items = GetHistorySize();
	if (items > 0) {
		CDeferRender	defer(*this, FALSE);	// don't render at all
		int	HistPos = GetHistoryPos();
		if (HistPos < 0)	// if no current position
			HistPos = items - 1;	// default to last item
		CHistoryItem&	item = GetHistoryItem(HistPos);
		SetSnapshot(item);	// restore current history item
		m_Main->OnParamChange();	// update UI
		if (item->HasImage()) {	// if image bitmap is cached
			SetImage(item->m_Image);	// restore image bitmap
			if (item->HasEscTimes())	// if escape times were saved
				SetEscapeTimes(item->m_EscTimes);	// restore them too
		}
	}
}

bool CFracticeView::CreateThumb(CBitmap& Thumb, CRect& Rect)
{
	CClientDC	dc(this);
	if (!Thumb.CreateCompatibleBitmap(&dc, m_ThumbSize.cx, m_ThumbSize.cy))
		return(FALSE);
	HGDIOBJ	PrevBmp = m_BackBufDC.SelectObject(Thumb);	// borrow a DC
	CRect	r(CPoint(0, 0), m_ThumbSize);
	CDib::Letterbox(r, m_ImageSize, r);
	m_BackBufDC.StretchBlt(r.left, r.top, r.Width(), r.Height(),
		&m_FrameDC, 0, 0, m_ImageSize.cx, m_ImageSize.cy, SRCCOPY);
	m_BackBufDC.SelectObject(PrevBmp);	// restore DC's previous bitmap
	Rect = r;
	return(TRUE);
}

void CFracticeView::SetImage(CDib& Dib)
{
	FinishRender();
	m_HaveEscTimes = FALSE;
	m_Engine.SetDib(Dib);
	if (m_FitInWindow)
		UpdateBackBuf();
	Invalidate();
}

void CFracticeView::SetEscapeTimes(const CEscTimeArray& EscTimes)
{
	m_Engine.SetEscFrame(EscTimes);
	m_HaveEscTimes = TRUE;
	m_Main->GetLevelsBar().CalcLevels();	// recalculate histogram
}

bool CFracticeView::CacheImage(CSnapshot& Snap, CSnapshot *ParentSnap)
{
	if (!Snap.HasImage()) {	// if image not already cached
		Snap.m_Image.CreateObj();
		Snap.m_Image->Copy(m_Engine.GetDib());
		if (!Snap.HasImage())	// if image creation failed
			return(FALSE);
	}
	// if saving escape times, and they're not already saved
	if (m_Options.m_SaveEscTimes && !Snap.HasEscTimes()) {
		if (ParentSnap != NULL && ParentSnap->HasEscTimes()) {
			// assume child shares parent's escape times; instead of storing
			// redundant data, add a reference to parent's escape time array
			Snap.m_EscTimes = ParentSnap->m_EscTimes;
		} else {	// no parent snapshot, or parent didn't have escape times
			Snap.m_EscTimes.CreateObj();	// allocate escape time array
			m_Engine.GetEscFrame(Snap.m_EscTimes);	// store escape times
		}
	}
	return(TRUE);
}

bool CFracticeView::CreateBackBuf(CSize Size)
{
	CSize	PrevSz = m_BackBufRect.Size();
	CRect	rc;
	GetClientRect(rc);
	CDib::Letterbox(rc, m_ImageSize, m_BackBufRect);
	if (m_BackBufRect.Size() != PrevSz) {
		CClientDC	dc(this);
		m_BackBuf.DeleteObject();
		if (!m_BackBuf.CreateCompatibleBitmap(&dc, 
		m_BackBufRect.Width(), m_BackBufRect.Height())) {
			AfxMessageBox(IDS_CANT_CREATE_FRAME);
			return(FALSE);
		}
		UpdateBackBuf();
	}
	return(TRUE);
}

void CFracticeView::UpdateBackBuf()
{
	m_BackBufDC.SelectObject(m_BackBuf);
	m_BackBufDC.StretchBlt(0, 0, m_BackBufRect.Width(), m_BackBufRect.Height(), 
		&m_FrameDC, 0, 0, m_ImageSize.cx, m_ImageSize.cy, SRCCOPY);
}

bool CFracticeView::ExportBitmap(LPCTSTR Path)
{
	static const int	Resolution = 72;	// dots per inch
	CFileException	e;
	if (!m_Engine.GetDib().Write(Path, Resolution, &e)) {
		if (e.m_cause != CFileException::none)
			e.ReportError();
		else
			AfxMessageBox(IDS_CANT_CREATE_BITMAP);
		return(FALSE);
	}
	return(TRUE);
}

void CFracticeView::UpdateColorTimer()
{
	m_ColorCycleTimer.Run(m_CycleColors && !m_Main->IsExclusive());
}

void CFracticeView::CycleColorExclusive()
{
	double	delta = m_ColorCycleRate / m_Options.m_FrameRate;
	m_ColorOffset = Wrap(m_ColorOffset + delta, 1);
	m_Engine.UpdateTweenPalette(m_Palette, m_CycleLen, m_ColorOffset);
}

bool CFracticeView::OpenPalette(LPCTSTR Path, bool Import, bool AddSnapshot)
{
	if (Import) {
		UINT	Flags = m_Main->GetOptionsDlg().GetPalImpExpFlags();
		if (!m_Palette.Import(Path, Flags))
			return(FALSE);	// error is already handled
	} else {
		DPalette	pal;
		if (!pal.Read(Path))
			return(FALSE);	// error is already handled
		m_Palette = pal;
	}
	m_Palette.m_Name = theApp.GetTitleFromPath(Path);	// name palette after file
	ApplyPalette(AddSnapshot ? EF_ADD_SNAPSHOT : 0, UCODE_OPEN_PALETTE);
	m_Main->GetPaletteDlg().InitCtrls();
	if (AddSnapshot)
		m_Main->GetMRUPalette().Add(Path);
	return(TRUE);
}

bool CFracticeView::SavePalette(LPCTSTR Path)
{
	DPalette	pal(m_Palette);
	pal.m_Name.Empty();	// don't store name in file, it would be redundant
	if (!pal.Write(Path))
		return(FALSE);
	m_Palette.m_Name = theApp.GetTitleFromPath(Path);	// name palette after file
	m_Main->GetPaletteDlg().SetPaletteName(m_Palette.m_Name);
	m_Main->GetMRUPalette().Add(Path);
	return(TRUE);
}

bool CFracticeView::ChoosePaletteFile(bool Import)
{
	bool	retc = FALSE;
	DPalette	PrevPal(m_Palette);
	CString	Path;
	bool	Preview;
	if (CPaletteDlg::ChooseFile(Path, Preview, this, Import)) {	// if user pressed OK
		if (Preview) {	// if previewing, palette was already loaded
			SetUndoCode(UCODE_OPEN_PALETTE);
			AddHistory();	// don't re-render, just add to history
			m_Main->GetMRUPalette().Add(Path);	// update recent file list
		} else	// not previewing palette files
			retc = OpenPalette(Path, Import);
		m_Main->SetFolder(CFilesBar::PANE_PALETTES, Path);
	} else {	// user canceled dialog
		if (m_Palette != PrevPal) {	// if palette changed
			SetPalette(PrevPal, FALSE);	// restore it, no snapshot
		}
	}
	return(retc);
}

void CFracticeView::SetCycleColors(bool Enable)
{
	if (Enable == m_CycleColors)
		return;
	if (Enable && !m_HaveEscTimes && !IsRendering()) {
		RenderFrame();
		m_AddToHistory = FALSE;
	}
	m_CycleColors = Enable;
	UpdateColorTimer();
	m_Main->OnParamChange();	// update UI
}

bool CFracticeView::GetHistogram(CDWordArray& Sample, UINT& MaxSamp)
{
	if (!m_HaveEscTimes)
		return(FALSE);
	int	samps = m_Quality + 1;	// one extra for background value
	Sample.RemoveAll();
	Sample.SetSize(samps);
	const ESCTIME	*pEscFrame = m_Engine.GetEscFrame();
	SIZE	EscFrameSize = m_Engine.GetEscFrameSize();
	int	pixels = EscFrameSize.cx * EscFrameSize.cy;
	int	i;
	for (i = 0; i < pixels; i++) {
		UINT	idx = *pEscFrame++;
		Sample[idx]++;
	}
	Sample.SetSize(--samps);	// exclude background value
	UINT	hival = 0;
	for (i = 0; i < samps; i++) {	// find biggest sample
		if (Sample[i] > hival)
			hival = Sample[i];
	}
	MaxSamp = hival;
	return(TRUE);
}

void CFracticeView::SetContZoom(int ZoomType)
{
	if (ZoomType == m_ContZoom)
		return;	// nothing to do
	if (ZoomType) {
		if (m_ContZoom)	// if already continuous zooming
			m_ContZoom = ZoomType;	// just change direction
		else {
			SetCapture();
			SetUndoCode(UCODE_ZOOM);
			m_ContZoom = ZoomType;
			m_ContZoomTimer.Run(TRUE);
			OnTimer(TID_CONT_ZOOM);
		}
		SetCursor(m_ZoomCursor[ZoomType == CONT_ZOOM_OUT]);
	} else {
		ReleaseCapture();
		m_ContZoom = 0;
		m_ContZoomTimer.Run(FALSE);
		if (!IsRendering()) {
			m_PreviewRect.SetRectEmpty();	// prevent thumbnail detail rectangle
			AddHistory();
		}
	}
}

void CFracticeView::ShowProperties(const CSnapshot& Snap)
{
	CString	msg, sx, sy, sz;
	Snap.m_Origin.x.Format(COORDS_PRECISION, sx);
	Snap.m_Origin.y.Format(COORDS_PRECISION, sy);
	Snap.m_Zoom.Format(COORDS_PRECISION, sz);
	ASSERT(Snap.m_FractalType < FRACTAL_TYPES);
	CString	PaletteName(Snap.m_Palette.m_Name);
	if (PaletteName.IsEmpty())
		PaletteName.LoadString(IDS_MF_UNTITLED);
	CString	FractalName = CParamsCtrl::GetComboString(
		PARM_FRACTAL_TYPE, Snap.m_FractalType);
	CString	DeepZoomStr = CParamsCtrl::GetComboString(
		PARM_DEEP_ZOOM, Snap.m_DeepZoom);
	msg.Format(IDS_SNAP_PROPERTIES, FractalName, sx, sy, sz,
		Snap.m_ImageSize.cx, 
		Snap.m_ImageSize.cy, 
		Snap.m_Quality, 
		Snap.m_Antialias, 
		Snap.m_CycleLen,
		Snap.m_ColorOffset,
		Snap.m_MathPrec,
		DeepZoomStr,
		Snap.m_Exponent,
		PaletteName,
		LDS((Snap.HasImage() ? IDS_PARM_YES : IDS_PARM_NO)),
		LDS((Snap.HasEscTimes() ? IDS_PARM_YES : IDS_PARM_NO))
	);
	CMsgBoxDlg::Do(msg, LDS(IDS_SNAP_PROPS_CAP), MB_ICONINFORMATION);
}

void CFracticeView::AddHistory()
{
	CHistoryItem	item;
	item.CreateObj();
	GetSnapshot(*item);
	item->m_Thumb.CreateObj();
	CRect	ThumbFrame;
	if (CreateThumb(item->m_Thumb, ThumbFrame)) {
		item->m_ThumbFrame = ThumbFrame;
		int	hsz = GetHistorySize();
		if (m_Previewing && hsz > 0 && GetHistoryPos() == hsz - 1) {
			CHistoryItem&	src = GetHistoryItem(hsz - 1);
			if (src->m_ThumbDetail.IsRectNull() || !src.m_ShowDetail) {
				DPoint	scale(ThumbFrame.Size());
				scale /= m_ImageSize;
				DPoint	p1(m_PreviewRect.TopLeft());
				DPoint	p2(m_PreviewRect.BottomRight());
				CRect	r(p1 * scale, p2 * scale);
				r += ThumbFrame.TopLeft();
				src->m_ThumbDetail = r;
				src.m_ShowDetail = TRUE;
			}
		}
		if (m_Options.m_CacheImages) {	// if caching full-size images
			CSnapshot	*ParentSnap = NULL;
			if (m_Options.m_SaveEscTimes) {	// if saving escape times
				bool	remap;
				// list of edits that only affect color
				switch (m_UndoCode) {
				case UCODE_OPEN_PALETTE:
				case UCODE_EDIT_PALETTE:
				case UCODE_APPLY_PALETTE:
				case UCODE_INVERT_COLOR:
				case UCODE_ROTATE_HUE:
				case UCODE_CYCLE_LEN:
				case UCODE_COLOR_OFFSET:
					remap = TRUE;	// color remap
					break;
				default:
					remap = FALSE;	// regular render
				}
				if (remap) {	// if color remap, escape times don't change
					int	HistPos = GetHistoryPos();
					if (HistPos >= 0)	// if we have a parent snapshot
						ParentSnap = GetHistoryItem(HistPos);	// share its times
				}
			}
			if (!CacheImage(item, ParentSnap))
				AfxMessageBox(IDS_CANT_CREATE_BITMAP);
		}
		if (m_UndoCode) {
			NotifyUndoableEdit(m_UndoCode, m_UndoFlags);
			if ((m_UndoFlags & UE_COALESCE) && GetHistorySize() > 0) {
				int	last = GetHistorySize() - 1;
				m_HistView->SetItem(last, item);
				m_HistView->RedrawItem(last);
			} else	// not coalescing
				m_HistView->AddItem(item);
			m_UndoCode = 0;
			m_UndoFlags = 0;
		} else	// not undoable
			m_HistView->AddItem(item);
	} else 	// create thumb failed
		AfxMessageBox(IDS_CANT_CREATE_THUMB);
}

void CFracticeView::ApplyOptions()
{
	COptionsInfo	Opts;
	m_Main->GetOptionsDlg().GetInfo(Opts);
	bool	WasRendering = IsRendering();
	bool	PrevAddToHistory = m_AddToHistory;
	{
		CDeferRender	defer(*this, FALSE);	// assume no rendering needed
		int	ThreadCount = Opts.m_ThreadCount;
		if (ThreadCount != m_Engine.GetThreadCount()) {	// if thread count changed
			if (IsRendering())	// relaunching engine cancels current render
				defer.SetRender(TRUE);	// restart render when we leave scope
			if (!m_Engine.LaunchThreads(m_hWnd, ThreadCount))
				AfxMessageBox(IDS_CANT_LAUNCH_ENGINE);
		}
		int	UndoLevels = Opts.m_UndoLevels;
		if (UndoLevels != m_UndoMgr.GetLevels())	// if undo levels changed
			m_UndoMgr.SetLevels(UndoLevels);
		m_Engine.SetSSE2(Opts.m_UseSSE2 != 0);
		if (m_Engine.GetSSE2() && (m_ImageSize.cx & 1)) {	// if SSE2 and odd width
			AfxMessageBox(IDS_SSE2_ODD_WIDTH);
			CSize	sz(m_ImageSize.cx & ~1, m_ImageSize.cy);	// force even width
			SetImageSize(sz);
			defer.SetRender(TRUE);	// render when we leave scope
		}
		if (Opts.m_GridColor != m_Options.m_GridColor)
			Invalidate();
		m_ColorCycleTimer.SetPeriod(round(1000 / Opts.m_FrameRate));
	}	// do deferred render if any
	if (IsRendering() && WasRendering)
		m_AddToHistory = PrevAddToHistory;
	m_Options = Opts;	// copy options to member
}

void CFracticeView::SaveUndoState(CUndoState& State)
{
	m_HistView->SaveUndoState(State);	// delegate to history view
	if (!UValFlags(State))	// if undo state is being initialized
		UValFlags(State) = UFLAG_UNDO;	// initial direction is undo
	else	// undo or redo is in progress
		UValFlags(State) ^= UFLAG_DIR;	// reverse direction
	GetDocument()->SetModifiedFlag();	// mark document modified
}

void CFracticeView::RestoreUndoState(const CUndoState& State)
{
	m_Previewing = FALSE;	// reset preview state
	m_HistView->RestoreUndoState(State);	// delegate to history view
}

void CFracticeView::GetUndoTitle(const CUndoState& State, CString& Title)
{
	Title.LoadString(m_UndoTitleID[State.GetCode()]);
}

/////////////////////////////////////////////////////////////////////////////
// CFracticeView drawing

inline void CFracticeView::Line(CDC *pDC, int x1, int y1, int x2, int y2)
{
	pDC->MoveTo(x1, y1);
	pDC->LineTo(x2, y2);
}

void CFracticeView::DrawAxes(CDC *pDC, CRect& rb)
{
	static const double Epsilon = 1e-6;	// threshold for detecting origin
	static const double MinTickDelta = 60;	// minimum tick spacing in client coords
	static const LPCTSTR NumFmt = _T("%.4g");	// number format
	static const int	TextOfs = 5;	// text offset in client coords
	if (rb.IsRectEmpty())
		return;	// avoid real trouble
	DPoint	org = ComplexToClient(BigPoint(0, 0));	// origin
	DPoint	upt = ComplexToClient(BigPoint(1, 1));	// unit point
	double	uv = upt.x - org.x;	// unit vector in client coords
	double	td = 0;	// tick delta
	if (!_isnan(uv) && uv > 0) {	// if unit vector is a positive number
		int	uvexp;
		frexp(uv, &uvexp);	// get unit vector's exponent
		if (abs(uvexp) < DBL_MAX_EXP - 64) {	// if exponent isn't too big
			td = uv;	// iterate to find optimal tick spacing
			if (td > MinTickDelta) {
				double	r;
				while ((r = td / 2) > MinTickDelta)
					td = r;
			} else {
				while (td < MinTickDelta)
					td *= 2;
			}
		}
	}
	if (!td)
		return;	// tick delta underflow
	if (!m_FitInWindow)
		org += GetScrollPosition();
	if (m_Dragging)
		org -= m_DragOffset;
	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextAlign(TA_TOP | TA_LEFT);
	pDC->SetTextColor(m_Options.m_GridColor);
	HGDIOBJ	PrevFont = pDC->SelectObject(GetStockObject(DEFAULT_GUI_FONT));
	HGDIOBJ	PrevPen = pDC->SelectObject(GetStockObject(DC_PEN));
	SetDCPenColor(pDC->m_hDC, m_Options.m_GridColor);
	CRect	TextRect;
	if (m_FitInWindow)
		TextRect = m_BackBufRect;
	else
		GetClientRect(TextRect);
	if (m_Dragging)
		TextRect -= m_DragOffset;
	DPoint	start(fmod(org.x, td), fmod(org.y, td));
	int	i = round(ceil((rb.left - start.x) / td));
	int	iorg = round(floor(org.x / td));	// for detecting zero crossing
	CString	s;
	while (1) {	// draw x-axis numbers and verticals
		double	x = start.x + td * i;
		if (x >= rb.right)
			break;
		double	n = (i == iorg) ? 0 : (x - org.x) / uv;
		s.Format(NumFmt, n);
		int	ix = round(x);
		pDC->TextOut(ix + TextOfs, TextRect.top + TextOfs, s);
		Line(pDC, ix, rb.top, ix, rb.bottom);
		i++;
	}
	i = round(ceil((rb.top - start.y) / td));
	iorg = round(floor(org.y / td));
	while (1) {	// draw y-axis numbers and horizontals
		double	y = start.y + td * i;
		if (y >= rb.bottom)
			break;
		double	n = (i == iorg) ? 0 : (org.y - y) / uv;	// flip y-axis
		s.Format(NumFmt, n);	
		int	iy = round(y);
		pDC->TextOut(TextRect.left + TextOfs, iy + TextOfs, s);
		Line(pDC, rb.left, iy, rb.right, iy);
		i++;
	}
	pDC->SelectObject(PrevFont);
	pDC->SelectObject(PrevPen);
}

void CFracticeView::OnDraw(CDC* pDC)
{
	static const double MIN_SHRINK = 2e-3;	// excessive shrinking makes StretchBlt slow
	CRect	rcb;
	pDC->GetClipBox(rcb);
	CRect	rb;
	if (m_FitInWindow)
		rb = m_BackBufRect;
	else
		rb = CRect(CPoint(0, 0), m_ImageSize);
	if (m_Dragging)
		rb -= m_DragOffset;
	if (!m_PreviewSnap.IsEmpty()) {	// if previewing a snapshot
		CRect	r(m_PreviewSnap->m_ThumbFrame);
		if (m_Previewing) {	// if preview has been offset and/or zoomed
			// translate preview rect from frame coords to snapshot coords
			DRect	rs(m_PreviewRect);
			rs.Scale(r.Width() / double(m_ImageSize.cx));
			rs += r.TopLeft();	// in case snapshot is letterboxed
			r = rs;
		}
		HBITMAP	ThumbBmp = *m_PreviewSnap->m_Thumb;
		if (ThumbBmp && rb.Width() / double(r.Width()) > MIN_SHRINK) {
			HGDIOBJ	PrevBmp = m_BackBufDC.SelectObject(ThumbBmp);	// borrow a DC
			pDC->SetStretchBltMode(HALFTONE);
			pDC->StretchBlt(rb.left, rb.top, rb.Width(), rb.Height(), 
				&m_BackBufDC, r.left, r.top, r.Width(), r.Height(), SRCCOPY);
			m_BackBufDC.SelectObject(PrevBmp);	// restore DC's previous bitmap
		} else	// excessive shrink
			pDC->FillSolidRect(rb, RGB(0, 0, 0));	// paint it black
	} else {
		if (m_Previewing) {	// if previewing frame buffer
			CRect	r(m_PreviewRect);
			if (rb.Width() / double(r.Width()) > MIN_SHRINK) {
				pDC->SetStretchBltMode(HALFTONE);
				pDC->StretchBlt(rb.left, rb.top, rb.Width(), rb.Height(), 
					&m_FrameDC, r.left, r.top, r.Width(), r.Height(), SRCCOPY);
			} else	// excessive shrink
				pDC->FillSolidRect(rb, RGB(0, 0, 0));	// paint it black
		} else {	// blit back buffer or frame buffer
			pDC->BitBlt(rb.left, rb.top, rb.Width(), rb.Height(), 
				m_FitInWindow ? &m_BackBufDC : &m_FrameDC, 0, 0, SRCCOPY);
		}
	}
	if (m_ShowAxes)
		DrawAxes(pDC, rb);
	pDC->ExcludeClipRect(rb);
	COLORREF	color = GetSysColor(COLOR_3DFACE);
	pDC->FillSolidRect(rcb, color);
#if SHOW_CROSSHAIR
	CClientDC	dc(this);
	CRect	rc;
	GetClientRect(rc);
	dc.SelectObject(GetStockObject(WHITE_PEN));
	Line(dc, rc.left, rc.Height() / 2, rc.right, rc.Height() / 2);
	Line(dc, rc.Width() / 2, rc.top, rc.Width() / 2, rc.bottom);
#endif
}

/////////////////////////////////////////////////////////////////////////////
// CFracticeView printing

BOOL CFracticeView::OnPreparePrinting(CPrintInfo* pInfo)
{
	pInfo->SetMaxPage(1);	// default to one page
	return DoPreparePrinting(pInfo);
}

void CFracticeView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo) 
{
	CScrollView::OnPrepareDC(pDC, pInfo);
}

void CFracticeView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}

void CFracticeView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}

void CFracticeView::OnPrint(CDC* pDC, CPrintInfo* pInfo) 
{
	CRect	rb;
	if (m_FitInWindow) {
		CSize	psz(pDC->GetDeviceCaps(HORZRES), pDC->GetDeviceCaps(VERTRES));
		CRect	rp(CPoint(0, 0), psz);
		CDib::Letterbox(rp, m_ImageSize, rb);
	} else {
		rb = CRect(CPoint(0, 0), m_ImageSize);
	}
	BITMAPINFO	bi;
	ZeroMemory(&bi, sizeof(bi));
	bi.bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth         = m_ImageSize.cx;
	bi.bmiHeader.biHeight        = m_ImageSize.cy;
	bi.bmiHeader.biPlanes        = 1;
	bi.bmiHeader.biBitCount      = m_Engine.GetDibInfo().BitCount;
	bi.bmiHeader.biCompression   = BI_RGB;
	bi.bmiHeader.biSizeImage     = m_Engine.GetDibInfo().Length;
	SetStretchBltMode(pDC->m_hDC, HALFTONE);
	StretchDIBits(pDC->m_hDC, rb.left, rb.top, rb.Width(), rb.Height(),
		0, 0, m_ImageSize.cx, m_ImageSize.cy,
		m_Engine.GetDibBits(), &bi, DIB_RGB_COLORS, SRCCOPY);
}

/////////////////////////////////////////////////////////////////////////////
// CFracticeView diagnostics

#ifdef _DEBUG
void CFracticeView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CFracticeView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

CFracticeDoc* CFracticeView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CFracticeDoc)));
	return (CFracticeDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFracticeView message map

BEGIN_MESSAGE_MAP(CFracticeView, CScrollView)
	//{{AFX_MSG_MAP(CFracticeView)
	ON_WM_MOUSEWHEEL()
	ON_WM_SETCURSOR()
	ON_WM_CREATE()
	ON_WM_CONTEXTMENU()
	ON_WM_DESTROY()
	ON_COMMAND(ID_EDIT_BOUNDS, OnEditBounds)
	ON_COMMAND(ID_EDIT_COORDS, OnEditCoords)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_DELETE, OnEditDelete)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
	ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_COMMAND(ID_FILE_EXPORT, OnFileExport)
	ON_COMMAND(ID_FILE_POSTER, OnFilePoster)
	ON_COMMAND(ID_HIST_RECORD, OnHistRecord)
	ON_WM_HSCROLL()
	ON_COMMAND(ID_IMG_CTX_ZOOM_IN, OnImgCtxZoomIn)
	ON_COMMAND(ID_IMG_CTX_ZOOM_OUT, OnImgCtxZoomOut)
	ON_COMMAND(ID_IMG_CYCLE_COLORS, OnImgCycleColors)
	ON_COMMAND(ID_IMG_DELETE_ESC_TIMES, OnImgDeleteEscTimes)
	ON_COMMAND(ID_IMG_FIT_IN_WINDOW, OnImgFitInWindow)
	ON_COMMAND(ID_IMG_HISTORY_EXPORT, OnImgHistoryExport)
	ON_COMMAND(ID_IMG_PALETTE_EXPORT, OnImgPaletteExport)
	ON_COMMAND(ID_IMG_PALETTE_IMPORT, OnImgPaletteImport)
	ON_COMMAND(ID_IMG_PALETTE_OPEN, OnImgPaletteOpen)
	ON_COMMAND(ID_IMG_PALETTE_SAVE, OnImgPaletteSave)
	ON_COMMAND(ID_IMG_PAL_INVERT_COLOR, OnImgPalInvertColor)
	ON_COMMAND(ID_IMG_PAL_ROTATE_HUE, OnImgPalRotateHue)
	ON_COMMAND(ID_IMG_RENDER, OnImgRender)
	ON_COMMAND(ID_IMG_UNCACHE_IMAGE, OnImgUncacheImage)
	ON_COMMAND(ID_IMG_ZOOM_IN, OnImgZoomIn)
	ON_COMMAND(ID_IMG_ZOOM_OUT, OnImgZoomOut)
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_SIZE()
	ON_COMMAND(ID_SNAP_PROPERTIES, OnSnapProperties)
	ON_WM_TIMER()
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_DELETE, OnUpdateEditDelete)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateEditRedo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_UPDATE_COMMAND_UI(ID_FILE_POSTER, OnUpdateFilePoster)
	ON_UPDATE_COMMAND_UI(ID_IMG_CYCLE_COLORS, OnUpdateImgCycleColors)
	ON_UPDATE_COMMAND_UI(ID_IMG_DELETE_ESC_TIMES, OnUpdateImgDeleteEscTimes)
	ON_UPDATE_COMMAND_UI(ID_IMG_FIT_IN_WINDOW, OnUpdateImgFitInWindow)
	ON_UPDATE_COMMAND_UI(ID_IMG_HISTORY_EXPORT, OnUpdateImgHistoryExport)
	ON_UPDATE_COMMAND_UI(ID_IMG_RENDER, OnUpdateImgRender)
	ON_UPDATE_COMMAND_UI(ID_IMG_UNCACHE_IMAGE, OnUpdateImgUncacheImage)
	ON_UPDATE_COMMAND_UI(ID_SNAP_PROPERTIES, OnUpdateSnapProperties)
	ON_UPDATE_COMMAND_UI(ID_VIEW_GRID, OnUpdateViewGrid)
	ON_COMMAND(ID_VIEW_GRID, OnViewGrid)
	ON_WM_VSCROLL()
	ON_UPDATE_COMMAND_UI(ID_HIST_RECORD, OnUpdateSnapProperties)
	ON_COMMAND(ID_IMG_TRANSFORM, OnImgTransform)
	ON_UPDATE_COMMAND_UI(ID_IMG_TRANSFORM, OnUpdateImgTransform)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CScrollView::OnFilePrintPreview)
	ON_MESSAGE(UWM_RENDERDONE, OnRenderDone)
	ON_MESSAGE(UWM_STRIPDONE, OnStripDone)
	ON_COMMAND_RANGE(ID_IMG_FRACTAL_TYPE_1, ID_LAST_FRACTAL_TYPE, OnImgFractalType)
	ON_UPDATE_COMMAND_UI_RANGE(ID_IMG_FRACTAL_TYPE_1, ID_LAST_FRACTAL_TYPE, OnUpdateImgFractalType)
	ON_COMMAND_RANGE(ID_IMG_IMAGE_SIZE_0, ID_IMG_IMAGE_SIZE_4, OnImgImageSize)
	ON_UPDATE_COMMAND_UI_RANGE(ID_IMG_IMAGE_SIZE_1, ID_IMG_IMAGE_SIZE_4, OnUpdateImgImageSize)
	ON_COMMAND_RANGE(ID_IMG_QUALITY_0, ID_IMG_QUALITY_3, OnImgQuality)
	ON_UPDATE_COMMAND_UI_RANGE(ID_IMG_QUALITY_1, ID_IMG_QUALITY_3, OnUpdateImgQuality)
	ON_COMMAND_RANGE(ID_IMG_ANTIALIAS_0, ID_IMG_ANTIALIAS_4, OnImgAntialias)
	ON_UPDATE_COMMAND_UI_RANGE(ID_IMG_ANTIALIAS_1, ID_IMG_ANTIALIAS_4, OnUpdateImgAntialias)
	ON_COMMAND_RANGE(ID_IMG_MRU_PALETTE1, ID_IMG_MRU_PALETTE9, OnImgMRUPalette)
	ON_UPDATE_COMMAND_UI_RANGE(ID_IMG_MRU_PALETTE1, ID_IMG_MRU_PALETTE9, OnUpdateImgMRUPalette)
	ON_COMMAND_RANGE(ID_IMG_MRU_IMAGE_SIZE1, ID_IMG_MRU_IMAGE_SIZE9, OnImgMRUImageSize)
	ON_UPDATE_COMMAND_UI_RANGE(ID_IMG_MRU_IMAGE_SIZE1, ID_IMG_MRU_IMAGE_SIZE9, OnUpdateImgMRUImageSize)
	ON_COMMAND_RANGE(ID_IMG_MRU_QUALITY1, ID_IMG_MRU_QUALITY9, OnImgMRUQuality)
	ON_UPDATE_COMMAND_UI_RANGE(ID_IMG_MRU_QUALITY1, ID_IMG_MRU_QUALITY9, OnUpdateImgMRUQuality)
	ON_COMMAND_RANGE(ID_IMG_MRU_ANTIALIAS1, ID_IMG_MRU_ANTIALIAS9, OnImgMRUAntialias)
	ON_UPDATE_COMMAND_UI_RANGE(ID_IMG_MRU_ANTIALIAS1, ID_IMG_MRU_ANTIALIAS9, OnUpdateImgMRUAntialias)
	ON_MESSAGE(UWM_MULTIFILESEL, OnMultiFileSel)
	ON_MESSAGE(UWM_TOOLCHANGE, OnToolChange)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFracticeView message handlers

BOOL CFracticeView::PreCreateWindow(CREATESTRUCT& cs)
{
	// override default window class styles CS_HREDRAW and CS_VREDRAW
	// otherwise resizing frame redraws entire view, causing flicker
	cs.lpszClass = AfxRegisterWndClass(	// create our own window class
		CS_DBLCLKS,						// request double-clicks
		theApp.LoadStandardCursor(IDC_ARROW),	// standard cursor
		NULL,									// no background brush
		theApp.LoadIcon(IDR_MAINFRAME));		// app's icon
    ASSERT(cs.lpszClass);
	return CScrollView::PreCreateWindow(cs);
}

int CFracticeView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CScrollView::OnCreate(lpCreateStruct) == -1)
		return -1;
	m_Main = theApp.GetMain();
	m_Main->GetOptionsDlg().GetInfo(m_Options);
	m_ThumbSize = m_Options.m_ThumbSize;
	CRuntimeClass	*pFactory = RUNTIME_CLASS(CHistoryView);
	m_HistView = (CHistoryView *)pFactory->CreateObject();
	m_HistView->SetParentView(this);
	DWORD	dwStyle = AFX_WS_DEFAULT_VIEW;
    CRect r(0, 0, 0, 0);	// arbitrary size
	CWnd	*pHistBar = &m_Main->GetHistoryBar();
    if (!m_HistView->Create(NULL, NULL, dwStyle, r, pHistBar, ID_HISTORY_VIEW, NULL))
		return -1;
	if (!(m_FrameDC.CreateCompatibleDC(NULL) 
	&& m_BackBufDC.CreateCompatibleDC(NULL))) {
		AfxMessageBox(IDS_CANT_CREATE_DC);
		return -1;
	}
	m_FrameDefBmp = m_FrameDC.GetCurrentBitmap();	// save DC's default bitmap
	m_BackBufDefBmp = m_BackBufDC.GetCurrentBitmap();
	m_BackBufDC.SetStretchBltMode(HALFTONE);	// back buffer uses halftoning
	if (!m_Engine.LaunchThreads(m_hWnd, m_Options.m_ThreadCount)) {
		AfxMessageBox(IDS_CANT_LAUNCH_ENGINE);
		return -1;
	}
	m_Engine.SetSSE2(m_Options.m_UseSSE2 != 0);
	CDeferRender	defer(*this, FALSE);	// don't render until OnInitialUpdate
	SetUndoManager(&m_UndoMgr);
	m_UndoMgr.SetLevels(m_Options.m_UndoLevels);
	m_ProgressTimer.Create(m_hWnd, TID_PROGRESS, PROGRESS_PERIOD, FALSE);
	m_ContZoomTimer.Create(m_hWnd, TID_CONT_ZOOM, CONT_ZOOM_PERIOD, FALSE);
	int	ColorCyclePeriod = round(1000 / m_Options.m_FrameRate);
	m_ColorCycleTimer.Create(m_hWnd, TID_COLOR_CYCLE, ColorCyclePeriod, FALSE);
	for (int i = 0; i < TOOLS; i++) {
		int	tcid = m_ToolCursorID[i];
		if (tcid)
			m_ToolCursor[i] = theApp.LoadCursor(tcid);
		else
			m_ToolCursor[i] = LoadCursor(NULL, IDC_ARROW);
	}
	m_HandGrabCursor = theApp.LoadCursor(IDC_TOOL_HAND_CLOSED);
	m_ZoomCursor[0] = theApp.LoadCursor(IDC_TOOL_ZOOM_IN);
	m_ZoomCursor[1] = theApp.LoadCursor(IDC_TOOL_ZOOM_OUT);

	return 0;
}

void CFracticeView::OnDestroy() 
{
	m_FrameDC.SelectObject(m_FrameDefBmp);	// restore default bitmap
	m_BackBufDC.SelectObject(m_BackBufDefBmp);
	m_Engine.KillThreads();
	m_Engine.DestroyFrame();
	m_ProgressTimer.Destroy();
	m_ContZoomTimer.Destroy();
	m_HistView->DestroyWindow();
	CScrollView::OnDestroy();
}

void CFracticeView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();
	SetScrollSizes(MM_TEXT, CSize(0, 0));
	m_Main->SetView(this);	// order matters
	CFracticeDoc	*pDoc = GetDocument();
	if (pDoc->IsSnapshot()) {	// if document is a snapshot
		m_ThumbSize = m_Options.m_ThumbSize;	// set thumbnail size
		CSnapshot	Snap = GetDocument()->m_Snap;	// copy snapshot
		if (Snap.IsEmpty())	// if snapshot is empty
			GetDefaults(Snap);	// get default values
		m_HistView->UpdateLayout();	// update history view layout
		SetSnapshot(Snap);	// render document
		m_Main->GetRecordDlg().SetInfo(m_RecInfo);
	} else {	// document is a project
		// if project has a snapshot, and snapshot has an image
		if (pDoc->HasSnapshot() && pDoc->m_Snapshot->HasImage()) {
			CDeferRender	defer(*this, FALSE);	// don't render at all
			SetProject(*pDoc);	// initialize our state from project
			PVOID	pBits;	// image can be big, so avoid making duplicates
			CSnapshot&	snap = pDoc->m_Snapshot;
			HBITMAP	hDib = snap.m_Image->Detach(pBits);	// detach image bitmap
			m_Engine.AttachDib(hDib, pBits);	// attach image to engine
			SelectObject(m_FrameDC, m_Engine.GetDib());	// select image into DC
			m_PreviewSnap = NULL;	// not rendering, so preview not needed
			if (snap.HasEscTimes())	// if project snapshot has escape times
				SetEscapeTimes(snap.m_EscTimes);	// transfer them to engine
			else	// no escape times in project
				m_Main->GetLevelsBar().SetEmpty();	// erase histogram
			m_Main->OnParamChange();	// normally called by RenderFrame
		} else	// project didn't have a snapshot image
			SetProject(*pDoc);	// just initialize our state from project
		pDoc->m_History.RemoveAll();	// remove document's history references
	}
	UpdateColorTimer();
	if (m_FitInWindow)
		UpdateBackBuf();
}

void CFracticeView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	switch (lHint) {
	case UVH_EDITOPTIONS:
		ApplyOptions();
		break;
	case UVH_ENABLEWND:
		m_WndEnabled = pHint != NULL;
		EnableWindow(m_WndEnabled);
		m_HistView->EnableWindow(m_WndEnabled);
		if (!m_WndEnabled)	// if disabled
			m_HistView->GetSlideShowMgr().Stop();	// stop slide show
		break;
	case UVH_EXCLUSIVE:
		UpdateColorTimer();
		break;
	}
}

void CFracticeView::OnSize(UINT nType, int cx, int cy) 
{
	if (m_Engine.GetDib() != NULL) {	// make sure frame exists
		if (m_FitInWindow) {
			CreateBackBuf(CSize(cx, cy));	// order matters
			Invalidate();
			SetScrollSizes(MM_TEXT, CSize(cx, cy));
			CScrollView::OnSize(nType, cx, cy);
		} else {
			if (IsRendering())
				InvalidateRect(m_ProgBarRect);	// erase stale progress bar
		}
	}
#if SHOW_CROSSHAIR
	Invalidate();
#endif
	CScrollView::OnSize(nType, cx, cy);
}

void CFracticeView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if (IsRendering() && !m_FitInWindow)
		InvalidateRect(m_ProgBarRect);	// erase stale progress bar
#if SHOW_CROSSHAIR
	Invalidate();
#endif
	CScrollView::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CFracticeView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if (IsRendering() && !m_FitInWindow)
		InvalidateRect(m_ProgBarRect);	// erase stale progress bar
#if SHOW_CROSSHAIR
	Invalidate();
#endif
	CScrollView::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CFracticeView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	UINT	Tool = m_Main->GetCurTool();
	if (nFlags & MK_CONTROL)
		Tool = TOOL_HAND;
	switch (Tool) {
	case TOOL_ZOOM:
		SetContZoom(CONT_ZOOM_IN);
		break;
	case TOOL_HAND:
		m_Engine.CancelRender();
		SetCapture();
		m_Dragging = TRUE;
		m_DragStart = point;
		m_MainAccel = m_Main->GetAccelTable();	// save main accelerators
		m_Main->SetAccelTable(NULL);	// disable main accels so we get Esc key
		SetCursor(m_HandGrabCursor);
		break;
	default:
		CRectTrackerEx	tracker;
		m_MarqueeTracking = TRUE;
		if (tracker.TrackRubberBand(this, point)) {
			CRect	rv;
			GetViewRect(rv);
			CRect	rt = tracker.m_rect;
			rt.NormalizeRect();
			double	sx = rv.Width() / double(rt.Width());
			double	sy = rv.Height() / double(rt.Height());
			double	ZoomScale = min(sx, sy);
			SetUndoCode(UCODE_ZOOM);
			OffsetOrigin(rt.CenterPoint() - rv.CenterPoint(), ZoomScale);
		}
		m_MarqueeTracking = FALSE;
	}
	CScrollView::OnLButtonDown(nFlags, point);
}

void CFracticeView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (m_Dragging) {
		ReleaseCapture();
		m_Dragging = FALSE;
		m_Main->SetAccelTable(m_MainAccel);	// restore main accelerators
		if (point != m_DragStart) {
			SetUndoCode(UCODE_DRAG);
			OffsetOrigin(m_DragStart - point);
		}
	}
	if (m_ContZoom) {
		if (!(GetAsyncKeyState(VK_RBUTTON) & GKS_DOWN))	// if other button is up
			SetContZoom(CONT_ZOOM_NONE);	// stop continuous zooming
	}
	CScrollView::OnLButtonUp(nFlags, point);
}

void CFracticeView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	switch (nChar) {
	case VK_ESCAPE:
		if (m_Dragging) {
			ReleaseCapture();
			m_Dragging = FALSE;
			m_Main->SetAccelTable(m_MainAccel);	// restore main accelerators
			Invalidate();
		}
		break;
	case VK_UP:
		Scroll(DPoint(0, -m_Options.m_ScrollDelta));
		break;
	case VK_DOWN:
		Scroll(DPoint(0, m_Options.m_ScrollDelta));
		break;
	case VK_LEFT:
		Scroll(DPoint(-m_Options.m_ScrollDelta, 0));
		break;
	case VK_RIGHT:
		Scroll(DPoint(m_Options.m_ScrollDelta, 0));
		break;
	}
	CScrollView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CFracticeView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	switch (m_Main->GetCurTool()) {
	case TOOL_ZOOM:
		SetContZoom(CONT_ZOOM_OUT);
		break;
	}
	CScrollView::OnRButtonDown(nFlags, point);
}

void CFracticeView::OnRButtonUp(UINT nFlags, CPoint point) 
{
	if (m_ContZoom) {
		if (!(GetAsyncKeyState(VK_LBUTTON) & GKS_DOWN))	// if other button is up
			SetContZoom(CONT_ZOOM_NONE);	// stop continuous zooming
	} else
		CScrollView::OnRButtonUp(nFlags, point);
}

void CFracticeView::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_Dragging) {
		m_DragOffset = m_DragStart - point;
		Invalidate();
	}
	CScrollView::OnMouseMove(nFlags, point);
}

BOOL CFracticeView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	if (nFlags == MK_CONTROL) {
		if (zDelta) {	// avoid divide by zero
			ScreenToClient(&pt);
			double	scale = m_Options.m_ZoomStep * abs(zDelta) / WHEEL_DELTA;
			if (zDelta < 0)	// if wheel rotated back
				scale = 1 / scale;	// zoom out
			SetUndoCode(UCODE_ZOOM);
			ZoomPoint(pt, scale);
			return(0);
		}
	} else {
		if (IsRendering() && !m_FitInWindow)
			InvalidateRect(m_ProgBarRect);	// erase stale progress bar
	}
	return CScrollView::OnMouseWheel(nFlags, zDelta, pt);
}

void CFracticeView::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	CRect	rc;
	GetClientRect(rc);
	CPoint	pt(point);
	ScreenToClient(&pt);
	if (rc.PtInRect(pt)) {
		CMenu	menu, *mp;
		menu.LoadMenu(IDR_VIEW);
		mp = menu.GetSubMenu(0);
		mp->TrackPopupMenu(0, point.x, point.y, GetParent());
		m_ContextPos = point;
	} else	// assume point is within a scroll bar
		CScrollView::OnContextMenu(pWnd, point);	// show scrolling menu
}

LRESULT CFracticeView::OnRenderDone(WPARAM wParam, LPARAM lParam)
{
	// wParam: frame UID, lParam: none
	if (wParam != m_FrameUID)	// if unexpected frame
		return(0);	// ignore message
	m_LastRenderTime = m_RenderTimer.Elapsed();
//printf("elapsed = %f\n", m_LastRenderTime);
	m_HaveEscTimes = TRUE;
	m_Main->GetLevelsBar().CalcLevels();	// recalculate histogram
	if (m_Main->IsRecording()) {	// if recording a movie
		FinishRender();
		m_Main->GetRecorder().OnRenderDone();
		if (m_FitInWindow)
			UpdateBackBuf();
		RedrawWindow();	// force immediate repaint
	} else {
		m_Engine.PostRender(m_LastRenderTime);
		if (m_ContZoom) {	// if continuous zooming
			FinishRender();
			if (m_FitInWindow)
				UpdateBackBuf();
			RedrawWindow();	// force immediate repaint
		} else {
			if (m_AddToHistory)	// if adding history item
				AddHistory();
			else {
				if (m_Options.m_CacheImages && !m_PreviewSnap.IsEmpty())
					CacheImage(m_PreviewSnap);
			}
			FinishRender();
			if (m_FitInWindow)
				UpdateBackBuf();
			Invalidate();
		}
	}
	return(0);
}

LRESULT CFracticeView::OnStripDone(WPARAM wParam, LPARAM lParam)
{
	if (wParam != m_FrameUID)	// if unexpected frame
		return(0);	// ignore message
	m_Engine.WriteLocalStrip(INT64TO32(lParam));
	return(0);
}

void CFracticeView::OnTimer(W64UINT nIDEvent) 
{
	switch (nIDEvent) {
	case TID_PROGRESS:
		{
			CClientDC	dc(this);
			CRect	rb;
			if (m_FitInWindow)
				rb = m_BackBufRect;
			else
				GetClientRect(rb);
			CRect	rpb(rb.left, rb.bottom - PROG_BAR_HEIGHT - PROG_BAR_BORDER * 2,
				rb.right, rb.bottom);
			m_ProgBarRect = rpb;
			CBrush	bkbr(RGB(0, 0, 0));
			dc.FrameRect(rpb, &bkbr);
			rpb.DeflateRect(PROG_BAR_BORDER, PROG_BAR_BORDER);
			int	Progress;
			if (m_Engine.IsStripJob())
				Progress = m_Engine.GetStripRowsDone();
			else
				Progress = m_Engine.GetCurrentRow();
			int	x = rpb.left + round(double(Progress) / 
				m_Engine.GetEscFrameSize().cy * (rpb.Width()));
			int	y = rpb.bottom - PROG_BAR_HEIGHT;
			CRect	DoneRect(rpb.left, y, x, rpb.bottom);
			dc.FillSolidRect(DoneRect, RGB(0, 255, 0));
			CRect	RemainRect(x, y, rpb.right, rpb.bottom);
			dc.FillSolidRect(RemainRect, RGB(0, 0, 0));
		}
		break;
	case TID_CONT_ZOOM:
		// if continuous zooming and a render is already in progress, assume
		// render time exceeds timer period and we've fallen behind; mustn't
		// start another render, else renders never finish and we seem stuck
		if (m_ContZoom && !IsRendering()) {	// if already rendering, discard
			CPoint	pt;
			GetCursorPos(&pt);
			ScreenToClient(&pt);
			double	step = m_Options.m_ContZoomStep;
			ZoomPoint(pt, m_ContZoom > 0 ? step : 1 / step);
			m_Previewing = FALSE;	// don't waste time showing preview
			ValidateRect(NULL);	// don't paint until render is done
		}
		break;
	case TID_COLOR_CYCLE:
		if (!IsRendering() && !m_MarqueeTracking && !m_Dragging && m_WndEnabled) {
			if (m_HaveEscTimes) {	// if we have escape times
				double	delta = m_ColorCycleRate / m_Options.m_FrameRate;
				m_ColorOffset = Wrap(m_ColorOffset + delta, 1);
				m_Engine.UpdatePalette(m_Palette, m_CycleLen, m_ColorOffset);
				m_Engine.MapColor();
				if (m_FitInWindow)
					UpdateBackBuf();
				RedrawWindow();
				m_Main->DoIdleLayout();
			} else	// can't cycle colors without escape times
				RenderFrame();	// so generate escape times
		}
		break;
	}
}

BOOL CFracticeView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	UINT	Tool = m_Main->GetCurTool();
	ASSERT(Tool >= 0 && Tool < TOOLS);
	SetCursor(m_ToolCursor[Tool]);
	return TRUE;
}

LRESULT	CFracticeView::OnToolChange(WPARAM wParam, LPARAM lParam)
{
	CRect	r;
	GetWindowRect(r);
	CPoint	CursorPos;
	GetCursorPos(&CursorPos);
	if (r.PtInRect(CursorPos))	// if cursor within our window
		OnSetCursor(this, HTCLIENT, WM_MOUSEMOVE);	// set cursor
	return(0);
}

void CFracticeView::OnFileExport() 
{
	CString	Title((LPCTSTR)IDS_EXPORT);
	CPathStr	Name(GetDocument()->GetFileName());
	Name.RemoveExtension();
	CMultiFileDlg	fd(FALSE, BITMAP_EXT, Name, OFN_OVERWRITEPROMPT,
		LDS(IDS_BMP_FILTER), NULL, Title, &theApp.m_ExportFolder);
	if (fd.DoModal() == IDOK)
		ExportBitmap(fd.GetPathName());
}

void CFracticeView::OnFilePoster() 
{
	int	HistPos = GetHistoryPos();
	if (HistPos >= 0) {
		CHistoryItem&	item = GetHistoryItem(HistPos);
		CPosterDlg	PosterDlg;
		PosterDlg.SetSnapshot(item);
		PosterDlg.DoModal();
	}
}

void CFracticeView::OnUpdateFilePoster(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(GetHistoryPos() >= 0);
}

void CFracticeView::OnEditUndo() 
{
	m_UndoMgr.Undo();
}

void CFracticeView::OnEditRedo() 
{
	m_UndoMgr.Redo();
}

void CFracticeView::OnUpdateEditUndo(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_UndoMgr.IsUndoable());
	CString	Text;
	Text.Format(LDS(IDS_EDIT_UNDO_FMT), m_UndoMgr.GetUndoTitle());
	pCmdUI->SetText(Text);
}

void CFracticeView::OnUpdateEditRedo(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_UndoMgr.IsRedoable());
	CString	Text;
	Text.Format(LDS(IDS_EDIT_REDO_FMT), m_UndoMgr.GetRedoTitle());
	pCmdUI->SetText(Text);
}

void CFracticeView::OnEditCopy() 
{
	m_HistView->Copy();
}

void CFracticeView::OnEditCut() 
{
	m_HistView->Cut();
}

void CFracticeView::OnEditPaste() 
{
	m_HistView->Paste();
}

void CFracticeView::OnEditDelete() 
{
	m_HistView->Delete();
}

void CFracticeView::OnUpdateEditCopy(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_HistView->HaveSelection());
}

void CFracticeView::OnUpdateEditCut(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_HistView->HaveSelection());
}

void CFracticeView::OnUpdateEditPaste(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_HistView->CanPaste());
}

void CFracticeView::OnUpdateEditDelete(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_HistView->HaveSelection());
}

void CFracticeView::OnEditSelectAll() 
{
	m_HistView->SelectAll();
}

void CFracticeView::OnEditCoords() 
{
	CCoordsDlg	dlg;
	dlg.m_x = m_Origin.x.ToStr();
	dlg.m_y = m_Origin.y.ToStr();
	dlg.m_z = m_Zoom.ToStr();
	if (dlg.DoModal() == IDOK) {
		SetUndoCode(UCODE_COORDS);
		BigPoint	Origin(m_MathPrec, 0, 0);
		BigNum		Zoom(m_MathPrec, 0);
		Origin.x.FromStr(dlg.m_x);
		Origin.y.FromStr(dlg.m_y);
		Zoom.FromStr(dlg.m_z);
		SetCoordsPreview(Origin, Zoom);
	}
}

void CFracticeView::OnEditBounds() 
{
	BigRect	b(m_MathPrec, 0, 0);
	GetBounds(b);
	CBoundsDlg	dlg;
	dlg.m_x1 = b.x1.ToStr();
	dlg.m_y1 = b.y1.ToStr();
	dlg.m_y2 = b.y2.ToStr();
	dlg.m_x2 = b.x2.ToStr();
	if (dlg.DoModal() == IDOK) {
		SetUndoCode(UCODE_BOUNDS);
		b.x1.FromStr(dlg.m_x1);
		b.y1.FromStr(dlg.m_y1);
		b.x2.FromStr(dlg.m_x2);
		b.y2.FromStr(dlg.m_y2);
		SetBounds(b);
	}
}

void CFracticeView::OnImgFractalType(UINT nID) 
{
	UINT	Type = nID;
	Type -= ID_IMG_FRACTAL_TYPE_1;
	SetUndoCode(UCODE_FRACTAL_TYPE);
	SetFractalType(Type);
}

void CFracticeView::OnUpdateImgFractalType(CCmdUI* pCmdUI)
{
	UINT	nID = pCmdUI->m_nID;
	UINT	Type = int(nID) - ID_IMG_FRACTAL_TYPE_1;
	pCmdUI->SetRadio(Type == m_FractalType);
}

void CFracticeView::OnImgImageSize(UINT nID) 
{
	int	PresetIdx = nID;
	PresetIdx -= ID_IMG_IMAGE_SIZE_1;
	if (PresetIdx < 0) {
		CImageSizeDlg	dlg;
		dlg.m_Width = m_ImageSize.cx;
		dlg.m_Height = m_ImageSize.cy;
		if (dlg.DoModal() == IDOK) {
			SetUndoCode(UCODE_IMAGE_SIZE);
			SetImageSize(CSize(dlg.m_Width, dlg.m_Height));
		}
	} else {
		SetUndoCode(UCODE_IMAGE_SIZE);
		SetImageSizePreset(PresetIdx);
	}
}

void CFracticeView::OnUpdateImgImageSize(CCmdUI* pCmdUI)
{
	UINT	nID = pCmdUI->m_nID;
	int	PresetIdx = int(nID) - ID_IMG_IMAGE_SIZE_1;
	pCmdUI->SetRadio(PresetIdx == m_ImgSizePresetIdx);
}

void CFracticeView::OnImgMRUImageSize(UINT nID)
{
	int	MRUIdx = nID - ID_IMG_MRU_IMAGE_SIZE1;
	CSize	sz;
	_stscanf(m_Main->GetMRUImgSize()[MRUIdx], MRU_IMAGE_SIZE_FMT, &sz.cx, &sz.cy);
	SetUndoCode(UCODE_IMAGE_SIZE);
	SetImageSize(sz);
}

void CFracticeView::OnUpdateImgMRUImageSize(CCmdUI* pCmdUI) 
{
	m_Main->GetMRUImgSize().UpdateMenu(pCmdUI);
	if (m_ImgSizePresetIdx < 0)
		m_Main->GetMRUImgSize().SetRadio(pCmdUI, 0, TRUE);
}

void CFracticeView::OnImgQuality(UINT nID)
{
	int	PresetIdx = nID;
	PresetIdx -= ID_IMG_QUALITY_1;
	if (PresetIdx < 0) {
		CQualityDlg	dlg;
		dlg.m_Value = m_Quality;
		if (dlg.DoModal() == IDOK) {
			SetUndoCode(UCODE_QUALITY);
			SetQuality(dlg.m_Value);
		}
	} else {
		SetUndoCode(UCODE_QUALITY);
		SetQualityPreset(PresetIdx);
	}
}

void CFracticeView::OnUpdateImgQuality(CCmdUI* pCmdUI) 
{
	UINT	nID = pCmdUI->m_nID;
	int	PresetIdx = int(nID) - ID_IMG_QUALITY_1;
	pCmdUI->SetRadio(PresetIdx == m_QualityPresetIdx);
}

void CFracticeView::OnImgMRUQuality(UINT nID)
{
	int	MRUIdx = nID - ID_IMG_MRU_QUALITY1;
	UINT	Quality;
	_stscanf(m_Main->GetMRUQuality()[MRUIdx], MRU_QUALITY_FMT, &Quality);
	SetUndoCode(UCODE_QUALITY);
	SetQuality(Quality);
}

void CFracticeView::OnUpdateImgMRUQuality(CCmdUI* pCmdUI) 
{
	m_Main->GetMRUQuality().UpdateMenu(pCmdUI);
	if (m_QualityPresetIdx < 0)
		m_Main->GetMRUQuality().SetRadio(pCmdUI, 0, TRUE);
}

void CFracticeView::OnImgAntialias(UINT nID)
{
	int	PresetIdx = nID;
	PresetIdx -= ID_IMG_ANTIALIAS_1;
	if (PresetIdx < 0) {
		CAntialiasDlg	dlg;
		dlg.m_Value = m_Antialias;
		if (dlg.DoModal() == IDOK) {
			SetUndoCode(UCODE_ANTIALIAS);
			SetAntialias(dlg.m_Value);
		}
	} else {
		SetUndoCode(UCODE_ANTIALIAS);
		SetAntialiasPreset(PresetIdx);
	}
}

void CFracticeView::OnUpdateImgAntialias(CCmdUI* pCmdUI) 
{
	UINT	nID = pCmdUI->m_nID;
	int	PresetIdx = int(nID) - ID_IMG_ANTIALIAS_1;
	pCmdUI->SetRadio(PresetIdx == m_AAPresetIdx);
}

void CFracticeView::OnImgMRUAntialias(UINT nID)
{
	int	MRUIdx = nID - ID_IMG_MRU_ANTIALIAS1;
	UINT	Antialias;
	_stscanf(m_Main->GetMRUAntialias()[MRUIdx], MRU_ANTIALIAS_FMT, &Antialias);
	SetUndoCode(UCODE_ANTIALIAS);
	SetAntialias(Antialias);
}

void CFracticeView::OnUpdateImgMRUAntialias(CCmdUI* pCmdUI) 
{
	m_Main->GetMRUAntialias().UpdateMenu(pCmdUI);
	if (m_AAPresetIdx < 0)
		m_Main->GetMRUAntialias().SetRadio(pCmdUI, 0, TRUE);
}

void CFracticeView::OnImgRender() 
{
	m_HistView->Render();
}

void CFracticeView::OnUpdateImgRender(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_HistView->CanRender());
}

void CFracticeView::OnImgUncacheImage() 
{
	if (AfxMessageBox(IDS_HIST_UNCACHE_IMAGE_WARN, MB_OKCANCEL) == IDOK) {
		for (CHistoryView::CSelectIter it(*m_HistView); it.More(); ++it)
			it->m_Image.SetEmpty();
	}
}

void CFracticeView::OnUpdateImgUncacheImage(CCmdUI* pCmdUI) 
{
	bool	GotItems = FALSE;
	for (CHistoryView::CSelectIter it(*m_HistView); it.More(); ++it) {
		if (it->HasImage()) {	// if selected item has cached image
			GotItems = TRUE;
			break;
		}
	}
	pCmdUI->Enable(GotItems);
}

void CFracticeView::OnImgDeleteEscTimes() 
{
	if (AfxMessageBox(IDS_HIST_DEL_ESC_TIMES_WARN, MB_OKCANCEL) == IDOK) {
		for (CHistoryView::CSelectIter it(*m_HistView); it.More(); ++it)
			it->m_EscTimes.SetEmpty();
	}
}

void CFracticeView::OnUpdateImgDeleteEscTimes(CCmdUI* pCmdUI) 
{
	bool	GotItems = FALSE;
	for (CHistoryView::CSelectIter it(*m_HistView); it.More(); ++it) {
		if (it->HasEscTimes()) {	// if selected item has escape times
			GotItems = TRUE;
			break;
		}
	}
	pCmdUI->Enable(GotItems);
}

void CFracticeView::OnImgHistoryExport() 
{
	CString	Title((LPCTSTR)IDS_REC_BMP_DEST_FOLDER);
	CString	Path;
	int	flags = BIF_EDITBOX | BIF_NEWDIALOGSTYLE;
	CString	InitDir = theApp.m_ExportFolder;
	if (CFolderDialog::BrowseFolder(Title, Path, NULL, flags, InitDir)) {
		theApp.m_ExportFolder = Path;
		CRenderingDlg	dlg(CRenderingDlg::CF_EXPORT_BITMAPS);
		CPathStr	Prefix(GetDocument()->GetTitle());
		Prefix.RemoveExtension();
		dlg.SetExportInfo(Path, Prefix + '_');
		dlg.DoModal();
	}
}

void CFracticeView::OnUpdateImgHistoryExport(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_HistView->HaveSelection());
}

void CFracticeView::OnImgPaletteOpen() 
{
	ChoosePaletteFile();
}

void CFracticeView::OnImgPaletteSave() 
{
	CMultiFileDlg	fd(FALSE, PALETTE_EXT, m_Palette.m_Name, OFN_OVERWRITEPROMPT,
		LDS(IDS_PALETTE_FILTER), NULL, NULL, &theApp.m_PaletteFolder);
	if (fd.DoModal() == IDOK)
		SavePalette(fd.GetPathName());
}

void CFracticeView::OnImgPaletteImport() 
{
	ChoosePaletteFile(TRUE);	// import
}

void CFracticeView::OnImgPaletteExport() 
{
	CMultiFileDlg	fd(FALSE, IMPORT_PAL_EXT, m_Palette.m_Name, OFN_OVERWRITEPROMPT,
		LDS(IDS_IMP_PAL_FILTER), NULL, NULL, &theApp.m_ImportFolder);
	if (fd.DoModal() == IDOK) {
		UINT	Flags = m_Main->GetOptionsDlg().GetPalImpExpFlags();
		UINT	Entries = m_Main->GetOptionsDlg().GetPalExportSize();
		m_Palette.Export(fd.GetPathName(), Entries, Flags);
	}
}

void CFracticeView::OnImgMRUPalette(UINT nID)
{
	int	FileIdx = nID - ID_IMG_MRU_PALETTE1;
	LPCTSTR	Path = m_Main->GetMRUPalette()[FileIdx];
	bool	Import = !DPalette::IsNativeFormat(Path);
	if (!OpenPalette(Path, Import))
		m_Main->GetMRUPalette().Remove(FileIdx);
}

void CFracticeView::OnUpdateImgMRUPalette(CCmdUI* pCmdUI) 
{
	m_Main->GetMRUPalette().UpdateMenu(pCmdUI);
}

void CFracticeView::OnImgPalInvertColor() 
{
	m_Palette.Invert();
	m_Main->GetPaletteDlg().InitCtrls();
	ApplyPalette(EF_ADD_SNAPSHOT | EF_COALESCE, UCODE_INVERT_COLOR);
}

void CFracticeView::OnImgPalRotateHue() 
{
	m_Palette.RotateHue(m_Options.m_HueRotation / 360.0);
	m_Main->GetPaletteDlg().InitCtrls();
	ApplyPalette(EF_ADD_SNAPSHOT | EF_COALESCE, UCODE_ROTATE_HUE);
}

LRESULT	CFracticeView::OnMultiFileSel(WPARAM wParam, LPARAM lParam)
{
	CMultiFileDlg	*fd = (CMultiFileDlg *)wParam;
	if (fd->GetPreview()) {	// if preview is checked
		CString	Path = fd->GetPathName();
		if (!Path.IsEmpty() && PathFileExists(Path)) {
			bool	Import = !_tcscmp(fd->m_ofn.lpstrDefExt, IMPORT_PAL_EXT);
			OpenPalette(fd->GetPathName(), Import, FALSE);	// don't add snapshot
		}
	}
	return(0);
}

void CFracticeView::OnImgZoomIn() 
{
	SetUndoCode(UCODE_ZOOM);
	ZoomCenter(m_Options.m_ZoomStep);
}

void CFracticeView::OnImgZoomOut() 
{
	SetUndoCode(UCODE_ZOOM);
	ZoomCenter(1 / m_Options.m_ZoomStep);
}

void CFracticeView::OnImgCtxZoomIn()
{
	CPoint	org(m_ContextPos);
	ScreenToClient(&org);
	SetUndoCode(UCODE_ZOOM);
	ZoomPoint(org, m_Options.m_ZoomStep);
}

void CFracticeView::OnImgCtxZoomOut()
{
	CPoint	org(m_ContextPos);
	ScreenToClient(&org);
	SetUndoCode(UCODE_ZOOM);
	ZoomPoint(org, 1 / m_Options.m_ZoomStep);
}

void CFracticeView::OnImgFitInWindow() 
{
	SetFitInWindow(!m_FitInWindow);
}

void CFracticeView::OnUpdateImgFitInWindow(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_FitInWindow);
}

void CFracticeView::OnImgCycleColors() 
{
	SetCycleColors(!m_CycleColors);
}

void CFracticeView::OnUpdateImgCycleColors(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_CycleColors);
}

void CFracticeView::OnImgTransform() 
{
	CTransformDlg	dlg(*this);
	dlg.DoModal();
}

void CFracticeView::OnUpdateImgTransform(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_HistView->HaveSelection());
}

void CFracticeView::OnSnapProperties()
{
	for (CHistoryView::CSelectIter iter(*m_HistView); iter.More(); ++iter)
		ShowProperties(*iter);
}

void CFracticeView::OnUpdateSnapProperties(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_HistView->HaveSelection());
}

void CFracticeView::OnHistRecord()
{
	int	pos = m_HistView->GetContextMenuPos();
	if (pos >= 0 && pos < GetHistorySize())
		m_HistView->AddRecordPoints(0, TRUE);	// replace existing script
}

void CFracticeView::OnViewGrid() 
{
	m_ShowAxes ^= 1;
	Invalidate();
}

void CFracticeView::OnUpdateViewGrid(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_ShowAxes);
}
