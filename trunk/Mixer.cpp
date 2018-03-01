// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      15feb09	initial version
		01		21feb09	use tweening engine
        02      09mar09	add crossfading
		03		10mar09	add autofade
		04		16mar09	add display mode info
		05		25mar09	animate mix slider during autofade
		06		06jan10	W64: make OnTimer 64-bit compatible

        mix two views in DirectDraw Exclusive mode
 
*/

// Mixer.cpp : implementation file
//

#include "stdafx.h"
#include "Fractice.h"
#include "Mixer.h"
#include <ddraw.h>
#include "MainFrm.h"
#include "FracticeDoc.h"
#include "FracticeView.h"
#include "MsgBoxDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMixer

CMixer::CMixer()
{
	m_View[CH_A] = NULL;
	m_View[CH_B] = NULL;
	m_FrameDefBmp = NULL;
	m_FrameSize = CSize(0, 0);
	m_IsExclusive = FALSE;
	m_IsDualMonitor = FALSE;
	m_DispSize = CSize(0, 0);
	m_DispRect.SetRectEmpty();
	m_FrameRate = 0;
	m_CurChan = 0;
	m_Chan[CH_A].m_Mixer = this;
	m_Chan[CH_B].m_Mixer = this;
	m_FaderPos = 0;
	m_FaderEnable = FALSE;
	m_Playing = FALSE;
	m_Looping = FALSE;
	m_AutofadeDir = 1;
	m_AutofadeTime = 0;
	m_AutofadeDelta = 0;
	m_AutofadeAnimMod = 0;
	m_AutofadeAnimCnt = 0;
	m_PalTweenTime = 0;
	m_Mirror = FALSE;
}

CMixer::~CMixer()
{
	DestroyBackBuf();
}

inline void CMixer::UpdateFrame(CDib& Frame, CFracticeView *View, int ChanIdx)
{
	if (View->GetCycleColors())
		View->CycleColorExclusive();
	View->GetEngine().MapColorEx(Frame, m_DibInfo, 
		m_BackBuf.IsMirrored(), m_Chan[ChanIdx].m_Origin);
}

inline void CMixer::FastSetFaderPos(double Pos)
{
	m_FaderPos = Pos;
	m_FaderEnable = Pos > 0 && Pos < 1;
	m_CurChan = Pos > .5;
}

bool CMixer::CreateBackBuf(CString& ErrMsg, const DISPLAY_MODE_INFO *ModeInfo)
{
	CRect	rc(0, 0, 100, 100);	// initial size is arbitrary
	LPCTSTR	ClassName = AfxRegisterWndClass(NULL, 0, NULL, 0);
	CString	WndName((LPCTSTR)IDS_EXCLUSIVE_WND_NAME);
	DWORD	dwStyle = WS_POPUP | WS_VISIBLE;
	if (!CreateEx(0, ClassName, WndName, dwStyle, rc, NULL, 0))
		return(FALSE);	// can't create our window
	if (!m_FrameDC.CreateCompatibleDC(NULL))
		return(FALSE);	// can't create frame DC
	m_FrameDefBmp = m_FrameDC.GetCurrentBitmap(); // save default bitmap
	CDDEnumObj	dev;
	CMainFrame	*pMain = theApp.GetMain();
	pMain->GetOptionsDlg().GetDisplayInfo(dev);
	dev.GetScreenCoords(m_DispRect);
	m_DispSize = m_DispRect.Size();
	if (ModeInfo == NULL) {	// if display size wasn't specified as an argument
		// verify that both image sizes match display size
		if ((m_View[CH_A] != NULL && m_View[CH_A]->GetImageSize() != m_DispSize)
		|| (m_View[CH_B] != NULL && m_View[CH_B]->GetImageSize() != m_DispSize)) {
			if (AfxMessageBox(IDS_MIX_BAD_IMAGE_SIZE, MB_OKCANCEL) != IDOK)
				return(FALSE);	// user heeded image size warning
		}
	}
	// if display device is a secondary monitor, we must pass its
	// GUID to DirectDraw to ensure proper hardware acceleration
	GUID	*pGuid = dev.m_hMonitor ? &dev.m_Guid : NULL;
	int	ErrId = 0;
	if (m_BackBuf.Create(m_hWnd, m_hWnd, pGuid, TRUE)) {
		if (ModeInfo != NULL) {
			if (m_BackBuf.GetDD()->SetDisplayMode(ModeInfo->Width,
			ModeInfo->Height, ModeInfo->BPP, 0, 0) != DD_OK) {
				ErrId = IDS_CANT_SET_DISPLAY_MODE;
				goto DDError;
			}
			m_DispSize = CSize(ModeInfo->Width, ModeInfo->Height);
		}
		if (m_BackBuf.CreateSurface(m_DispSize.cx, m_DispSize.cy)) {
			m_FrameTimer.Create(m_hWnd, FRAME_TIMER_ID, 0);
			m_FrameSize = CSize(0, 0);	// ensure frame gets created
			CRect	MainRect;
			HMONITOR	hOurMon, hMainMon;
			hOurMon = CBackBufDD::GetFullScreenRect(m_hWnd, m_DispRect);
			hMainMon = CBackBufDD::GetFullScreenRect(pMain->m_hWnd, MainRect);
			bool	SingleMon = hOurMon == hMainMon || MainRect == m_DispRect;
			m_IsDualMonitor = !SingleMon;
			m_IsExclusive = TRUE;	// success
		} else	// can't create surface
			ErrId = IDS_CANT_CREATE_SURFACE;
	} else	// can't create DirectDraw
		ErrId = IDS_CANT_CREATE_DIRECTDRAW;
DDError:
	if (ErrId) {
		CString	msg((LPCTSTR)ErrId);	// show error message
		msg += CString("\n") + m_BackBuf.GetLastErrorString();
		ErrMsg = msg;
		return(FALSE);	// DirectDraw error
	}
	return(TRUE);
}

void CMixer::DestroyBackBuf()
{
	m_BackBuf.Destroy();
	if (m_FrameDC.m_hDC != NULL) {
		m_FrameDC.SelectObject(m_FrameDefBmp);
		m_FrameDC.DeleteDC();
	}
	m_FrameSize = CSize(0, 0);
	m_FrameTimer.Destroy();
	for (int i = 0; i < CHANS; i++) {
		m_Frame[i].Destroy();
		m_Chan[i].m_Metronome.Kill();
	}
	if (IsWindow(m_hWnd))
		DestroyWindow();
	m_IsExclusive = FALSE;
	m_IsDualMonitor = FALSE;
}

bool CMixer::Exclusive(bool Enable, const DISPLAY_MODE_INFO *ModeInfo)
{
	if (Enable == m_IsExclusive)
		return(TRUE);	// nothing to do
	bool	retc = FALSE;	// assume failure
	if (Enable) {
		if (GetCurView() == NULL)	// if no view
			return(FALSE);	// exclusive mode is useless
		CString	ErrMsg;
		if (CreateBackBuf(ErrMsg, ModeInfo)) {	// create back buffer
			ApplySettings();
			OnTimer(FRAME_TIMER_ID);	// display first frame
			retc = TRUE;	// success
		} else {	// create back buffer failed
			DestroyBackBuf();
			if (ErrMsg.IsEmpty())	// if specific error message wasn't set
				ErrMsg.LoadString(IDS_CANT_SET_EXCLUSIVE);	// use generic message
			AfxMessageBox(ErrMsg);
		}
	} else {	// disable exclusive
		DestroyBackBuf();
		retc = TRUE;	// success
	}
	theApp.UpdateAllViews(NULL, CFracticeView::UVH_EXCLUSIVE, NULL);
	return(retc);
}

void CMixer::OnChannelChange()
{
	CFracticeView	*View = GetCurView();
	if (View != NULL)
		View->GetEngine().SetTweening(FALSE);	// suppress initial tween
}

void CMixer::SetView(int ChanIdx, CFracticeView *View)
{
	ASSERT(ChanIdx >= 0 && ChanIdx < CHANS);
	if (View == m_View[ChanIdx])	// if same view
		return;	// nothing to do
	m_View[ChanIdx] = View;
	if (ChanIdx == m_CurChan)	// if setting current channel
		OnChannelChange();
}

void CMixer::SetCurChan(int ChanIdx)
{
	ASSERT(ChanIdx >= 0 && ChanIdx < CHANS);
	if (ChanIdx == m_CurChan)	// if same input
		return;	// nothing to do
	m_CurChan = ChanIdx;
	if (m_View[CH_A] != m_View[CH_B])	// if channel views differ
		OnChannelChange();
}

void CMixer::SetMirror(bool Enable)
{
	m_Mirror = Enable;
	m_BackBuf.SetMirror(Enable);
}

void CMixer::SetOriginMotion(int ChanIdx, int OMType)
{
	ASSERT(OMType >= 0 && OMType < OM_TYPES);
	GetChan(ChanIdx).m_OrgMotion = OMType;
	if (m_IsExclusive)
		GetChan(ChanIdx).UpdateMetronome();
}

void CMixer::SetTempo(int ChanIdx, double Tempo)
{
	GetChan(ChanIdx).m_Tempo = Tempo;
	if (m_IsExclusive)
		GetChan(ChanIdx).UpdateMetronome();
}

void CMixer::SetTargetOrigin(int ChanIdx, const DPoint& Origin)
{
	GetChan(ChanIdx).m_TargetOrg.x = CLAMP(Origin.x, 0, 1);
	GetChan(ChanIdx).m_TargetOrg.y = CLAMP(Origin.y, 0, 1);
}

void CMixer::SetTargetOriginX(int ChanIdx, double Pos)
{
	GetChan(ChanIdx).m_TargetOrg.x = CLAMP(Pos, 0, 1);
}

void CMixer::SetTargetOriginY(int ChanIdx, double Pos)
{
	GetChan(ChanIdx).m_TargetOrg.y = CLAMP(Pos, 0, 1);
}

void CMixer::ApplySettings()
{
	enum {
		AUTOFADE_ANIM_FREQ = 3	// in Hertz; approximate
	};
	CMainFrame	*pMain = theApp.GetMain();
	m_FrameRate = pMain->GetOptionsDlg().GetFrameRate();
	CMixerInfo	Info;
	pMain->GetMixerDlg().GetInfo(Info);
	SetFaderPos(Info.m_FaderPos);
	m_Playing = Info.m_AutofadePlay != 0;
	m_Looping = Info.m_AutofadeLoop != 0;
	SetAutofadeTime(Info.m_AutofadeTime);
	m_PalTweenTime = Info.m_PalTweenTime;
	m_Mirror = Info.m_Mirror != 0;
	m_Chan[CH_A].SetBaseInfo(Info.m_Chan[CH_A]);
	m_Chan[CH_B].SetBaseInfo(Info.m_Chan[CH_B]);
	m_BackBuf.SetMirror(m_Mirror);
	if (m_IsExclusive) {
		int	period = round(1000.0 / m_FrameRate);
		m_FrameTimer.SetPeriod(period);
		m_Chan[CH_A].UpdateMetronome();
		m_Chan[CH_B].UpdateMetronome();
		m_AutofadeAnimMod = round(m_FrameRate / AUTOFADE_ANIM_FREQ);
	}
}

void CMixer::SetFaderPos(double Pos)
{
	FastSetFaderPos(Pos);
}

void CMixer::SetPlay(bool Enable)
{
	m_Playing = Enable;
}

void CMixer::SetLoop(bool Enable)
{
	m_Looping = Enable;
}

void CMixer::SetAutofadeTime(double Seconds)
{
	m_AutofadeTime = Seconds;
	m_AutofadeDelta = m_AutofadeTime ? 1.0 / m_AutofadeTime / m_FrameRate : 1e10;
}

BOOL WINAPI CMixer::DDEnumCallbackEx(GUID FAR *lpGUID, LPSTR lpDriverDescription, 
	LPSTR lpDriverName, LPVOID lpContext, HMONITOR hm)
{
	CDDEnumObjArray	*pList = (CDDEnumObjArray *)lpContext;
	CDDEnumObj	obj;
	if (lpGUID != NULL)
		memcpy(&obj.m_Guid, lpGUID, sizeof(GUID));
	obj.m_DriverName = lpDriverName;
	obj.m_DriverDescription = lpDriverDescription;
	obj.m_hMonitor = hm;
	pList->Add(obj);
	return(DDENUMRET_OK);	// continue enumerating
}

bool CMixer::EnumDDObjects(CDDEnumObjArray& List)
{
	// DirectDrawEnumerateEx has to be manually imported from the DirectDraw DLL
	HINSTANCE hInst = LoadLibrary(_T("ddraw.dll"));
	if (!hInst)
		return(FALSE);
	LPDIRECTDRAWENUMERATEEXA	lpDDEnumEx;	// specify ANSI version of struct
	lpDDEnumEx = (LPDIRECTDRAWENUMERATEEXA)GetProcAddress(hInst, "DirectDrawEnumerateExA");
	bool	retc = FALSE;
	if (lpDDEnumEx) {	// if the function was imported
		List.RemoveAll();
		lpDDEnumEx(DDEnumCallbackEx, &List, DDENUM_ATTACHEDSECONDARYDEVICES);
		retc = TRUE;
	}
    FreeLibrary(hInst);
	return(retc);
}

BEGIN_MESSAGE_MAP(CMixer, CWnd)
	//{{AFX_MSG_MAP(CMixer)
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMixer message handlers

void CMixer::OnClose() 
{
	theApp.GetMain()->Exclusive(FALSE);
}

void CMixer::OnTimer(W64UINT nIDEvent) 
{
	CFracticeView	*View = GetCurView();
	if (View == NULL || !m_IsExclusive)
		return;	// can't blit
	CSize	SrcSize = View->GetImageSize();
	CSize	DstSize = m_DispSize;
	if (m_BackBuf.IsMirrored()) {
		SrcSize.cx >>= 1;
		SrcSize.cy >>= 1;
		DstSize.cx >>= 1;
		DstSize.cy >>= 1;
	}
	if (SrcSize != m_FrameSize) {	// if image size changed
		// create frame DIB and select it into our device context
		WORD	bpp = View->GetColorDepth();
		if (!m_Frame[CH_A].Create(SrcSize.cx, SrcSize.cy, bpp))
			return;	// error: can't create DIB
		if (!m_Frame[CH_B].Create(SrcSize.cx, SrcSize.cy, bpp))
			return;	// error: can't create DIB
		BITMAP	bmp;
		if (!GetObject(m_Frame[CH_A], sizeof(BITMAP), &bmp))
			return;
		m_DibInfo.Size = CSize(bmp.bmWidth, bmp.bmHeight);
		m_DibInfo.BitCount = bmp.bmBitsPixel;
		m_DibInfo.Stride = CDib::GetStride(bmp);
		m_DibInfo.Length = bmp.bmHeight * m_DibInfo.Stride;
		m_FrameSize = SrcSize;
		m_FrameDC.SelectObject(m_Frame[CH_A]);
	}
	if (m_Playing) {
		double	newpos = m_FaderPos + m_AutofadeDelta * m_AutofadeDir;
		if (newpos < 0 || newpos > 1) {	// if position out of range
			newpos = newpos < 0 ? 0 : 1;
			m_AutofadeDir = -m_AutofadeDir;
			if (!m_Looping) {
				m_Playing = FALSE;
				theApp.GetMain()->GetMixerDlg().UpdateAutofadeState();
			}
		}
		FastSetFaderPos(newpos);
		m_AutofadeAnimCnt++;
		if (m_AutofadeAnimCnt >= m_AutofadeAnimMod) {
			theApp.GetMain()->GetMixerDlg().FastSetFaderPos(
				round(newpos * CMixMainDlg::FADER_RANGE));
			m_AutofadeAnimCnt = 0;
		}
	}
	if (m_FaderEnable) {
		if (m_View[CH_A]->GetImageSize() != m_View[CH_B]->GetImageSize()) {
			SetFaderPos(0);	// set fader to channel A
			m_Playing = FALSE;	// stop automated fading
			theApp.GetMain()->GetMixerDlg().UpdateAutofadeState();
			if (m_IsDualMonitor)
				AfxMessageBox(IDS_MIX_IMAGE_SIZE_MISMATCH);
			else {	// single monitor; use modeless message box
				CMsgBoxDlg::Do(LDS(IDS_MIX_IMAGE_SIZE_MISMATCH));
				SetFocus();	// else keyboard won't work as expected
			}
			return;	// error: images not same size
		}
		m_Chan[CH_A].UpdateOrigin(SrcSize);
		m_Chan[CH_B].UpdateOrigin(SrcSize);
		UpdateFrame(m_Frame[CH_A], m_View[CH_A], CH_A);
		UpdateFrame(m_Frame[CH_B], m_View[CH_B], CH_B);
		CTweenEngine::MixDibs(m_Frame[CH_A], m_Frame[CH_B], m_DibInfo, m_FaderPos);
	} else {	// not fading
		m_Chan[m_CurChan].UpdateOrigin(SrcSize);
		UpdateFrame(m_Frame[CH_A], View, m_CurChan);
	}
	HDC	hDC;
	m_BackBuf.GetDrawBuf()->GetDC(&hDC);
	if (SrcSize == DstSize) {	// if source and destination sizes match
		BitBlt(hDC, 0, 0, DstSize.cx, DstSize.cy, m_FrameDC, 0, 0, SRCCOPY);
	} else {	// source must be stretched
		StretchBlt(hDC, 0, 0, DstSize.cx, DstSize.cy, m_FrameDC, 0, 0, 
			SrcSize.cx, SrcSize.cy, SRCCOPY);
	}
	m_BackBuf.GetDrawBuf()->ReleaseDC(hDC);
	m_BackBuf.Blt();
}

BOOL CMixer::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	SetCursor(NULL);	// hide cursor whenever it's within our window
	return(TRUE);
}
