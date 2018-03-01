// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda

		revision history:
		rev		date	comments
        00      14feb08	initial version
        01      07jan09	call OnBeginStripJob unconditionally
		02		11jan09	optionally write rows to file
		03		16jan09	in WriteStripToFile, catch file errors
		04		26feb09	use PostMessage instead of SendMessage
		05		27feb09	map color in boss thread
		06		12dec09	override CancelRender to post cancel and bump UID
		07		15dec09	in FinishStripRender, don't reset frame size
		08		16dec09	set and reset m_BusyCount
		09		17dec09	replace strip job bool with strip mode
		10		17dec09	add GetLocalRowsDone
		11		17dec09	init pointers in ctor instead of BeginStripJob
		12		27dec09	inline GetLocalRowsDone

        fractal rendering engine with network support

*/

#include "stdafx.h"
#include "Fractice.h"
#include "NetEngine.h"
#include "MainFrm.h"
#include "Recorder.h"

CNetEngine::CNetEngine()
{
	CMainFrame	*pMain = theApp.GetMain();
	ASSERT(pMain != NULL);	// assume we're not a member of CMainFrame
	m_Recorder = &pMain->GetRecorder();	
	m_ServersDlg = &m_Recorder->GetServersDlg();
	m_StripRows = 0;
	m_NextRow = 0;
	m_StripSize = 0;
	m_RowsDone = 0;
	m_RenderToFile = FALSE;
}

CNetEngine::~CNetEngine()
{
	m_Recorder->DetachEngine(this);	// detach from servers
}

void CNetEngine::BeginStripJob(const RENDER_INFO& Info, const BigRect& Bounds, const CSnapshot& Snap)
{
	CStripEngine::BeginStripJob(Info, Bounds);
	if (!m_RenderToFile) {
		// if file render, derived engine must initialize these members
		m_NextRow = 0;
		m_RowsDone = 0;
		m_RedoStrip.RemoveAll();
	}
	m_StripRows = 1;	// tradeoff is granularity versus overhead
	m_StripSize = m_EscFrameSize.cx * m_StripRows;
	int	threads = GetThreadCount();
	m_Strip.SetSize(m_StripSize * threads);
	m_ServersDlg->OnBeginStripJob();	// reset all render counts
	for (int i = 0; i < threads; i++)
		NextLocalStrip(i);
	m_BusyCount = m_RenderCount;	// this engine is busy
	if (m_Recorder->IsDistributed()) {
		m_Recorder->AttachEngine(this);	// attach to servers
		m_Snap = Snap;	// copy rendering parameters
		PrimeServers();
	}
}

bool CNetEngine::PrimeServer(int ServerIdx)
{
	CServer&	srv = m_ServersDlg->GetServer(ServerIdx);
	srv.RemoveAllPending();
	if (!srv.WriteStripSnapshot(m_CurUID, m_StripRows, m_Snap))
		return(FALSE);
	UINT	threads = srv.GetThreadCount();
	for (UINT i = 0; i < threads; i++) {
		if (m_NextRow < m_EscFrameSize.cy) {
			for (int j = 0; j < PRIME_STRIPS; j++) {
				if (!NextRemoteStrip(ServerIdx, i))
					return(FALSE);	// send failed
			}
		} else	// no more strips available
			return(FALSE);
	}
	return(TRUE);
}

bool CNetEngine::PrimeServers()
{
	int	servs = m_ServersDlg->GetServerCount();
	for (int i = 0; i < servs; i++) {
		if (m_ServersDlg->GetServer(i).IsConnected()) {
			if (!PrimeServer(i))
				return(FALSE);
		}
	}
	return(TRUE);
}

void CNetEngine::DetachServer(int ServerIdx)
{
	// add server's pending strips to redo list
	CServer&	srv = m_ServersDlg->GetServer(ServerIdx);
	int	pending = srv.GetPendingCount();
	for (int i = 0; i < pending; i++)
		m_RedoStrip.Add(srv.GetPendingItem(i));
}

void CNetEngine::DetachAllServers()
{
	int	servs = m_ServersDlg->GetServerCount();
	for (int i = 0; i < servs; i++) {
		if (m_ServersDlg->GetServer(i).IsConnected())
			DetachServer(i);
	}
}

void CNetEngine::NextLocalStrip(UINT ThreadIdx)
{
	ESCTIME	*pSrc = m_Strip.GetData() + ThreadIdx * m_StripSize;
	int	y2 = m_NextRow + m_StripRows;
	y2 = min(y2, m_EscFrameSize.cy);	// last strip can have less rows
	RenderStrip(ThreadIdx, m_NextRow, y2, pSrc);
	m_NextRow = y2;
}

bool CNetEngine::NextRemoteStrip(int ServerIdx, UINT ThreadIdx)
{	
	int	y2 = m_NextRow + m_StripRows;
	y2 = min(y2, m_EscFrameSize.cy);	// last strip can have less rows
	CServer&	srv = m_ServersDlg->GetServer(ServerIdx);
	if (!srv.WriteStripRequest(m_CurUID, ThreadIdx, m_NextRow, y2))
		return(FALSE);
	srv.AddPending(m_NextRow);	// add strip to server's pending list
	m_NextRow = y2;
	return(TRUE);
}

void CNetEngine::WriteLocalStrip(UINT ThreadIdx)
{
	int	y1 = m_Render[ThreadIdx].m_y1;
	int	y2 = m_Render[ThreadIdx].m_y2;
	int	rows = y2 - y1;
	int	StripBytes = m_EscFrameSize.cx * rows * sizeof(ESCTIME);
	const ESCTIME	*pSrc = m_Strip.GetData() + ThreadIdx * m_StripSize;
	if (m_RenderToFile) {
		WriteStripToFile(y1, y2, pSrc, StripBytes);
	} else {	// render to memory
		ESCTIME	*pDst = GetEscFrame() + y1 * m_EscFrameSize.cx;
		memcpy(pDst, pSrc, StripBytes);
	}
	m_RowsDone += rows;
	m_ServersDlg->GetLocalHost().AddToRenderedCount(rows);
	if (m_NextRow < m_EscFrameSize.cy)	// if more rows to render
		NextLocalStrip(ThreadIdx);
	else {	// no more rows
		if (m_RedoStrip.GetSize()) {	// if rows need redoing
			m_NextRow = GetRedoStrip(0);
			NextLocalStrip(ThreadIdx);	// redo a row locally
			m_NextRow = m_EscFrameSize.cy;	// restore next row
		} else {	// no redos
			m_BusyCount = 0;	// this engine is idle
			if (m_RowsDone == m_EscFrameSize.cy)	// if all rows completed
				FinishStripRender();
		}
	}
}

void CNetEngine::WriteRemoteStrip(int ServerIdx, UINT ThreadIdx, UINT y1, UINT y2, const ESCTIME *Strip, double RenderTime)
{
	int	rows = y2 - y1;
	int	StripBytes = m_EscFrameSize.cx * rows * sizeof(ESCTIME);
	if (m_RenderToFile) {
		WriteStripToFile(y1, y2, Strip, StripBytes);
	} else {	// render to memory
		ESCTIME	*pDst = GetEscFrame() + y1 * m_EscFrameSize.cx;
		memcpy(pDst, Strip, StripBytes);
	}
	m_RowsDone += rows;
	CServer&	srv = m_ServersDlg->GetServer(ServerIdx);
	srv.AddToRenderedCount(rows);
	srv.SetRenderTime(RenderTime);
	srv.RemovePending(y1);	// remove strip from server's pending list
	if (m_NextRow < m_EscFrameSize.cy)	// if more rows to render
		NextRemoteStrip(ServerIdx, ThreadIdx);
	else {	// no more rows
		if (m_RedoStrip.GetSize()) {	// if rows need redoing
			m_NextRow = GetRedoStrip(0);
			NextRemoteStrip(ServerIdx, ThreadIdx);	// redo a row remotely
			m_NextRow = m_EscFrameSize.cy;	// restore next row
		} else {	// no redos
			if (m_RowsDone == m_EscFrameSize.cy)	// if all rows completed
				FinishStripRender();
		}
	}
}

void CNetEngine::FinishStripRender()
{
	m_ServersDlg->OnEndStripJob();
	m_Recorder->DetachEngine(this);	// detach from servers
	if (m_RenderToFile)
		PostMessage(m_hWnd, UWM_RENDERDONE, m_CurUID, 0);	// notify parent window
	else {	// render to memory
		m_Job.FractalType = FT_NO_RENDER;	// color mapping only
		m_StripMode = SM_STRIP_COLORMAP;
		m_Boss.StartWork();	// map color in boss thread
	}
}

void CNetEngine::WriteStripToFile(UINT y1, UINT y2, const ESCTIME *Strip, int StripBytes)
{
	int	done = m_RowsDone;
	for (UINT y = y1; y < y2; y++)
		m_RowIndex[y] = done++;
	TRY {
		m_RowFile.Write(Strip, StripBytes);
	}
	CATCH(CFileException, e)
	{
		enum {
			MAX_ERROR_MSG = 256
		};
		// message recipient is responsible for deleting this error string
		LPTSTR	pMsg = new TCHAR[MAX_ERROR_MSG];
		e->GetErrorMessage(pMsg, MAX_ERROR_MSG);
		PostMessage(m_hWnd, UWM_ABORTRENDER, m_CurUID, (LPARAM)pMsg);
	}
	END_CATCH
}

void CNetEngine::CancelRender(bool WaitForIdle)
{
	CStripEngine::CancelRender(WaitForIdle);
	if (m_StripMode == SM_STRIP_RENDER)
		m_ServersDlg->PostMsgToAllServers(PMID_CANCEL_RENDER);
	// Following a cancel, the app may resize our escape frame. If the new
	// frame is smaller, receiving any further strips for the canceled job
	// could cause an access violation in WriteRemoteStrip. Posting cancel
	// to the servers doesn't necessarily prevent this, because the strips
	// could already be in transit. The solution is to increment our UID,
	// so that any further strips received for this job will flunk the UID
	// check in CRecorder::OnRcvPacket. We use a big increment to avoid
	// collisions with the normal UID sequence.
	m_CurUID += SHRT_MAX;
}
