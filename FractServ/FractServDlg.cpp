// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		10jan09	move GetBounds to CSnapshot
		02		01feb09	add color offset
        03      12feb09	in Create, add ServerIP
        04      06jan10	W64: in DoModal, cast message params to 32-bit
		05		09jan10	add extended capabilities

        fractal rendering server dialog
 
*/

// FractServDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FractServ.h"
#include "FractServDlg.h"
#include "RecordInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFractServDlg dialog

// ExitWindowEx flags; must match shutdown actions enum in PacketDefs.h
const UINT CFractServDlg::m_ExitFlags[SSDA_ACTIONS] = {
	0,								// SSDA_EXIT
	EWX_LOGOFF,						// SSDA_LOG_OFF
	EWX_REBOOT,						// SSDA_REBOOT
	EWX_SHUTDOWN | EWX_POWEROFF		// SSDA_POWER_OFF
};

CFractServDlg::CFractServDlg(HWND hWnd)
{
	m_hWnd = hWnd;
	m_FrameUID = 0;
	m_ImageSize = CSize(0, 0);
	m_Antialias = 1;
	m_MathPrec = 0;
	SYSTEM_INFO	si;
	GetSystemInfo(&si);
	m_CPUCount = si.dwNumberOfProcessors;
	m_ThreadCount = theApp.GetThreadCount() > 0 ? 
		theApp.GetThreadCount() : m_CPUCount;
	m_Rendering = FALSE;
	m_FindServer = NULL;
	m_Progress = 0;
	m_StripSize = 0;
}

CFractServDlg::~CFractServDlg()
{
	Destroy();
}

void CFractServDlg::SetMathPrec(UINT Bits)
{
	if (Bits == m_MathPrec)
		return;	// nothing to do
	m_Engine.CancelRender(TRUE);	// wait for engine to be idle
	m_Engine.SetMathPrec(Bits);
	mpf_set_default_prec(Bits);	// set GMP's default math precision
	// set precision of BigNum-derived member vars here
	m_MathPrec = Bits;
}

bool CFractServDlg::SetImageSize(CSize Size, UINT Antialias)
{
	if (Size == m_ImageSize && Antialias == m_Antialias)
		return(TRUE);
	if (!m_Engine.CreateFrame(Size, CRecordInfo::DEF_BIT_COUNT, Antialias))
		return(FALSE);
	Size = m_Engine.GetDibInfo().Size;	// in case width was changed
	m_ImageSize = Size;
	m_Antialias = Antialias;
	return(TRUE);
}

void CFractServDlg::SetSnapshot(CMySnapPtr& Snap)
{
	m_Snap = Snap;
	SetMathPrec(Snap->m_MathPrec);
	if (SetImageSize(Snap->m_ImageSize, Snap->m_Antialias)) {
		m_Engine.SetPalette(m_Snap->m_Palette, m_Snap->m_Quality, 
			m_Snap->m_CycleLen, m_Snap->m_ColorOffset);
		RenderFrame();
	} else
		printf("can't create frame\n");
}

void CFractServDlg::RenderFrame()
{
	BigRect	Bounds(m_MathPrec, 0, 0);
	m_Snap->GetBounds(Bounds);
	RENDER_INFO	ri;
	ri.FrameBuf = m_Engine.GetEscFrame();
	ri.FractalType = m_Snap->m_FractalType;
	ri.FrameSize = m_Engine.GetEscFrameSize();
	ri.FrameUID = ++m_FrameUID;
	ri.Quality = m_Snap->m_Quality;
	ri.DeepZoom = m_Snap->m_DeepZoom;
	ri.Exponent = m_Snap->m_Exponent;
	m_Rendering = TRUE;
	m_RenderTimer.Reset();
	m_Engine.RenderFrame(ri, Bounds);
}

void CFractServDlg::BeginStripJob(CMySnapPtr& Snap, UINT StripRows)
{
	SetMathPrec(Snap->m_MathPrec);
	m_Snap = Snap;
	BigRect	Bounds(m_MathPrec, 0, 0);
	m_Snap->GetBounds(Bounds);
	m_ImageSize = CSize(0, 0);	// force next SetImageSize to create frame
	CSize	StripImgSize(Snap->m_ImageSize.cx * Snap->m_Antialias,
		Snap->m_ImageSize.cy * Snap->m_Antialias);
	RENDER_INFO	ri;
	ri.FrameBuf = NULL;	// destination is m_Strip, passed in via RenderStrip
	ri.FractalType = m_Snap->m_FractalType;
	ri.FrameSize = StripImgSize;
	ri.FrameUID = ++m_FrameUID;
	ri.Quality = Snap->m_Quality;
	ri.DeepZoom = Snap->m_DeepZoom;
	ri.Exponent = m_Snap->m_Exponent;
	m_StripSize = StripRows * StripImgSize.cx;
	m_Strip.SetSize(m_StripSize * m_Engine.GetThreadCount());
	for (UINT i = 0; i < m_ThreadCount; i++) {
		m_ThreadInfo[i].m_StripQueue.RemoveAll();
		m_ThreadInfo[i].m_Rendering = FALSE;
	}
	m_RenderTimer.Reset();
	m_Engine.BeginStripJob(ri, Bounds);
}

void CFractServDlg::RenderStrip(UINT ThreadIdx, UINT y1, UINT y2)
{
	ESCTIME	*pStrip = m_Strip.GetData() + m_StripSize * ThreadIdx;
	m_Engine.RenderStrip(ThreadIdx, y1, y2, pStrip);
	m_ThreadInfo[ThreadIdx].m_Rendering = TRUE;
}

DWORD CFractServDlg::FindServerCallback(LPCSTR ClientAddress, void *UserData, WFindServers::EXT_STATUS& ExtStatus)
{
	CFractServDlg	*This = (CFractServDlg *)UserData;
	ZeroMemory(&ExtStatus, sizeof(ExtStatus));
	ExtStatus.StructSize = sizeof(ExtStatus);
	bool	Connected = This->m_Packets.IsConnected();
	printf("%s found us\n", ClientAddress);
	if (Connected)
		ExtStatus.ClientIPAddress = This->m_Packets.GetRemoteAddr().sin_addr;
	return(Connected);
}

bool CFractServDlg::Create() 
{
	m_ThreadInfo.SetSize(m_ThreadCount);
	if (!m_Engine.LaunchThreads(m_hWnd, m_ThreadCount)) {
		printf("can't launch engine\n");
		return(FALSE);
	}
	m_Engine.SetSSE2(theApp.UseSSE2());
	LPCTSTR	HostName = theApp.GetClientAddress();
	LPCTSTR	ServerIP = theApp.GetServerIP();
	WORD	PortNumber = theApp.GetNetworkPort();
	if (!m_Packets.Create(HostName, PortNumber, ServerIP, m_hWnd, 0)) {
		printf("%s\n", m_Packets.GetLastErrorString());
		return(FALSE);
	}
	if (theApp.WantUdpServer() && HostName == NULL) {
		char	HostName[WFindServers::MAX_SERVER_NAME];
		gethostname(HostName, sizeof(HostName));
		try {
			m_FindServer = new WFindServers(HostName, PortNumber, ServerIP);
			m_FindServer->InstallStatusCallback(FindServerCallback, this);
		}
		catch (const WError& e) {
			printf("%s\n", e.what());
			return(FALSE);
		}
	}
	if (!m_ProgressTimer.Create(m_hWnd, PROGRESS_TIMER, 0)) {
		printf("can't create progress timer\n");
		return(FALSE);
	}
	return(TRUE);
}

bool CFractServDlg::DoModal() 
{
	if (!Create())
		return(FALSE);
	MSG	msg;
	while (1) {
		int	retc = GetMessage(&msg, NULL, 0, 0);
		if (!retc)	// if quit message
			break;
		switch (msg.message) {
		case UWM_RENDERDONE:
			OnRenderDone(UINT64TO32(msg.wParam));
			break;
		case UWM_RCVPACKET:
			OnRcvPacket((GENERIC_PACKET *)msg.wParam, INT64TO32(msg.lParam));
			break;
		case UWM_TCPCONNECT:
			OnTcpConnect(INT64TO32(msg.wParam), INT64TO32(msg.lParam));
			break;
		case UWM_STRIPDONE:
			OnStripDone(INT64TO32(msg.wParam), INT64TO32(msg.lParam));
			break;
		case WM_TIMER:
			OnTimer(INT64TO32(msg.wParam));
			break;
		}
	}
	return(TRUE);
}

void CFractServDlg::Destroy() 
{
	m_Engine.KillThreads();
	m_Engine.DestroyFrame();
	m_Packets.Destroy();
	delete m_FindServer;
	m_ProgressTimer.Destroy();
}

void CFractServDlg::OnRenderDone(UINT FrameUID)
{
	m_Rendering = FALSE;
	if (!m_Packets.IsConnected())	// if disconnected
		return;	// nothing to do
	float	RenderTime = static_cast<float>(m_RenderTimer.Elapsed());
	m_Packets.WriteDib(m_Snap->m_JobID, m_Snap->m_FrameID, RenderTime,
		m_Engine.GetDibBits(), m_Engine.GetDibInfo());
	if (m_SnapQueue.GetSize()) {
		SetSnapshot(m_SnapQueue[0]);
		m_SnapQueue.RemoveAt(0);
	}
}

void CFractServDlg::OnStripDone(UINT FrameUID, UINT ThreadIdx)
{
	CThreadInfo&	ThrInfo = m_ThreadInfo[ThreadIdx]; 
	ThrInfo.m_Rendering = FALSE;	// thread is finished rendering
	if (FrameUID != m_Engine.GetCurUID())	// if rendering a different frame
		return;	// ignore spurious message
	if (!m_Packets.IsConnected())	// if disconnected
		return;	// nothing to do
	float	RenderTime = static_cast<float>(m_RenderTimer.Elapsed());
	m_Packets.WriteStripDone(m_Snap->m_JobID, ThreadIdx, RenderTime, 
		m_Engine.GetY1(ThreadIdx), m_Engine.GetY2(ThreadIdx),
		m_Engine.GetFrameBuf(ThreadIdx), m_StripSize * sizeof(ESCTIME));
	if (ThrInfo.m_StripQueue.GetSize()) {	// if thread has a queued strip
		STRIP_INFO	si;
		si = ThrInfo.m_StripQueue[0];
		RenderStrip(ThreadIdx, si.y1, si.y2);	// start rendering queued strip
		ThrInfo.m_StripQueue.RemoveAt(0);	// remove strip from queue
	}
}

void CFractServDlg::CancelRender()
{
	m_Engine.CancelRender();
	m_Rendering = FALSE;
	m_SnapQueue.RemoveAll();	// purge pipeline
}

void CFractServDlg::OnRcvPacket(GENERIC_PACKET *pPacket, int Cookie)
{
	switch (pPacket->Message) {
	case PMID_CAPS_QUERY:
		{
			FRAP_CAPS_QUERY	*pp = (FRAP_CAPS_QUERY *)pPacket;
			if (pp->Signature == FRACTICE_NET_SIGNATURE
			&& pp->Version == FRACTICE_NET_VERSION) {
				m_Packets.WriteCapsReply(m_CPUCount, m_ThreadCount);
			} else
				m_Packets.Disconnect();
		}
		break;
	case PMID_EX_CAPS_QUERY:
		m_Packets.WriteExCapsReply();
		break;
	case PMID_HOST_NAME_QUERY:
		m_Packets.WriteHostNameReply();
		break;
	case PMID_SNAPSHOT:
		{
			FRAP_SNAPSHOT	*pp = (FRAP_SNAPSHOT *)pPacket;
			CMySnapPtr	Snap;
			Snap.CreateObj();
			{
				CMemFile	mf(pp->Snapshot, pp->DataLen);
				CArchive	ar(&mf, CArchive::load);
				Snap->Serialize(ar);
			}
			Snap->m_JobID = pp->JobID;
			Snap->m_FrameID = pp->FrameID;
			if (m_Rendering)
				m_SnapQueue.Add(Snap);
			else
				SetSnapshot(Snap);
		}
		break;
	case PMID_SHUTDOWN:
		{
			FRAP_MESSAGE	*pp = (FRAP_MESSAGE *)pPacket;
			UINT	Action = pp->wParam;
			ASSERT(Action >= 0 && Action < SSDA_ACTIONS);
			if (Action != SSDA_EXIT)
				theApp.SetShutdown(TRUE, m_ExitFlags[Action]);
			PostMessage(m_hWnd, WM_QUIT, 0, 0);
		}
		break;
	case PMID_CANCEL_RENDER:
		CancelRender();
		break;
	case PMID_PROGRESS_CONFIG:
		{
			FRAP_MESSAGE	*pp = (FRAP_MESSAGE *)pPacket;
			UINT	Period = pp->wParam;
			if (Period) {
				m_ProgressTimer.Set(Period);
				OnTimer(PROGRESS_TIMER);	// report progress immediately
			} else
				m_ProgressTimer.Stop();
		}
		break;
	case PMID_STRIP_SNAPSHOT:
		{
			FRAP_STRIP_SNAPSHOT	*pp = (FRAP_STRIP_SNAPSHOT *)pPacket;
			CMySnapPtr	Snap;
			Snap.CreateObj();
			{
				CMemFile	mf(pp->Snapshot, pp->DataLen);
				CArchive	ar(&mf, CArchive::load);
				Snap->Serialize(ar);
			}
			Snap->m_JobID = pp->JobID;
			BeginStripJob(Snap, pp->StripRows);
		}
		break;
	case PMID_STRIP_REQUEST:
		{
			FRAP_STRIP_REQUEST	*pp = (FRAP_STRIP_REQUEST *)pPacket;
			if (pp->JobID != m_Snap->m_JobID)	// if job ID doesn't match
				break;	// ignore spurious request
			UINT	ThreadIdx = pp->ThreadIdx;
			CThreadInfo&	ThrInfo = m_ThreadInfo[ThreadIdx]; 
			if (ThrInfo.m_Rendering) {	// if thread is busy rendering
				STRIP_INFO	si;
				si.y1 = pp->y1;
				si.y2 = pp->y2;
				ThrInfo.m_StripQueue.Add(si);	// queue strip info
			} else	// thread is idle
				RenderStrip(ThreadIdx, pp->y1, pp->y2);	// start rendering strip
		}
		break;
	}
	delete pPacket;	// recipient is responsible for cleanup
}

void CFractServDlg::OnTcpConnect(BOOL Connected, int Cookie)
{
	printf("%s %s\n", m_Packets.GetClientAddress(), 
		Connected ? "connected" : "disconnected");
	if (!Connected) {	// if disconnected
		CancelRender();
		m_ProgressTimer.Stop();
		if (theApp.GetClientAddress() != NULL)	// if we initiated connection
			PostMessage(m_hWnd, WM_QUIT, 0, 0);	// exit app
	}
}

void CFractServDlg::OnTimer(UINT nIDEvent) 
{
	if (m_Rendering) {
		int	Progress = round(m_Engine.GetCurrentRow() * 100.0 
			/ m_Engine.GetEscFrameSize().cy);	// convert to percentage
		if (Progress != m_Progress) {	// if percentage has changed
			m_Progress = Progress;	// report progress to client
			m_Packets.WriteMessage(PMID_PROGRESS_REPORT, Progress);
		}
	}
}
