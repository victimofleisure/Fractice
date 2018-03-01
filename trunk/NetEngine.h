// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		11jan09	optionally write rows to file
		02		27feb09	remove strip job flag, it's a base class member
		03		12dec09	override CancelRender to post cancel and bump UID
		04		17dec09	add GetLocalRowsDone
		05		27dec09	add PostRender

        fractal rendering engine with network support
 
*/

#ifndef CNETENGINE_INCLUDED
#define CNETENGINE_INCLUDED

#include "StripEngine.h"
#include "Snapshot.h"
#include "ServersDlg.h"

class CRecorder;

class CNetEngine : public CStripEngine {
public:
// Construction
	CNetEngine();
	~CNetEngine();

// Attributes
	int		GetStripRowsDone() const;
	int		GetLocalRowsDone() const;
	bool	IsRenderCanceled() const;

// Operations
	void	BeginStripJob(const RENDER_INFO& Info, const BigRect& Bounds, const CSnapshot& Snap);
	void	WriteLocalStrip(UINT ThreadIdx);
	void	WriteRemoteStrip(int ServerIdx, UINT ThreadIdx, UINT y1, UINT y2, const ESCTIME *Strip, double RenderTime);
	bool	PrimeServers();
	bool	PrimeServer(int ServerIdx);
	void	DetachServer(int ServerIdx);
	void	DetachAllServers();
	void	CancelRender(bool WaitForIdle = FALSE);
	void	PostRender(double RenderTime);

protected:
// Constants
	enum {
		PRIME_STRIPS = 2		// number of strips to prime server threads with
	};

// Member data
	CRecorder	*m_Recorder;	// pointer to recording object
	CServersDlg	*m_ServersDlg;	// pointer to servers dialog
	CEscTimeArray	m_Strip;	// destination buffer for local threads
	int		m_StripRows;		// number of rows in a strip
	int		m_NextRow;			// next row to be rendered
	int		m_StripSize;		// size of a strip in elements
	int		m_RowsDone;			// number of rows completed
	CSnapshot	m_Snap;			// snapshot of render parameters
	CDWordArray	m_RedoStrip;	// strips that failed to render and must be redone
	bool	m_RenderToFile;		// true if writing rows to file
	CFile	m_RowFile;			// file to write rows to
	CDWordArray	m_RowIndex;		// index of each row within file

// Helpers
	void	NextLocalStrip(UINT ThreadIdx);
	bool	NextRemoteStrip(int ServerIdx, UINT ThreadIdx);
	void	FinishStripRender();
	UINT	GetRedoStrip(int RedoIdx);
	void	WriteStripToFile(UINT y1, UINT y2, const ESCTIME *Strip, int StripBytes);
};

inline int CNetEngine::GetStripRowsDone() const
{
	return(m_RowsDone);
}

inline bool CNetEngine::IsRenderCanceled() const
{
	return(m_CurUID != m_Job.FrameUID);
}

inline UINT CNetEngine::GetRedoStrip(int RedoIdx)
{
	UINT	Row = m_RedoStrip[RedoIdx];
	m_RedoStrip.RemoveAt(RedoIdx);
	return(Row);
}

inline int CNetEngine::GetLocalRowsDone() const
{
	if (IsStripJob() || IsIdle())
		return(m_ServersDlg->GetLocalHost().GetRenderedCount());
	return(GetCurrentRow());
}

inline void CNetEngine::PostRender(double RenderTime)
{
	if (!IsStripJob()) {	// if normal render
		CServer&	srv = m_ServersDlg->GetLocalHost();
		srv.SetRenderedCount(m_Job.FrameSize.cy);
		srv.SetProgress(100);
	}
	m_ServersDlg->SetLocalRenderTime(RenderTime);
}

#endif
