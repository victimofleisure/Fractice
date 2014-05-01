// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		11jan09	move batch processing to job control dialog
		02		11feb09	add color cycling
		03		11jul09	allow unlimited record points
		04		25dec09	add GetCurJobID
		05		27dec09	overload GetRecordInfo to return info
		06		06jan10	W64: in GetRedoCount, cast redo array size to 32-bit

        recording subsystem
 
*/

#ifndef CRECORDER_INCLUDED
#define CRECORDER_INCLUDED

#include "BmpToAvi.h"
#include "RecordStatusDlg.h"
#include "ServersDlg.h"
#include "RecBmpThread.h"
#include "JobControlDlg.h"
#include "SortArray.h"
#include "RecordLeg.h"

#define RECORDER_BITMAP_NAME_FORMAT		_T("%08d.bmp")
#define RECORDER_BITMAP_FOLDER_SUFFIX	_T(" {bitmaps}")

class CMainFrame;
class CFracticeView;
class CNetEngine;

class CRecorder : public WObject {
public:
// Construction
	CRecorder();
	bool	Create();
	void	Destroy();

// Attributes
	CFracticeView	*GetView();
	CServersDlg&	GetServersDlg();
	CRecordStatusDlg&	GetRecordStatusDlg();
	CJobControlDlg&	GetJobControlDlg();
	bool	IsRecording() const;
	UINT	GetFramesDone() const;
	void	GetRecordInfo(CRecordInfo& Info) const;
	const CRecordInfo&	GetRecordInfo() const;
	void	SetRecordInfo(const CRecordInfo& Info);
	bool	GetBatchMode() const;
	void	GetJobInfo(CJobInfo& Info) const;
	void	SetJobInfo(const CJobInfo& Info);
	bool	IsListeningServers() const;
	bool	IsDistributed() const;
	double	GetTotalRenderTime() const;
	CNetEngine	*GetEngine() const;
	void	AttachEngine(CNetEngine *Engine);
	void	DetachEngine(CNetEngine *Engine);
	UINT	GetCurJobID() const;
	int		GetRedoCount() const;

// Operations
	bool	Record(const CRecordInfo& Info);
	bool	Record(const CRecordInfo& Info, CFracticeView *View, LPCTSTR Path, bool AddToJobCtrl = FALSE);
	bool	Stop();
	bool	StopCheck();
	void	Abort(LPCTSTR Msg);
	void	Abort(UINT MsgID);
	bool	ListenServers(bool Enable);

// Message handlers
	void	OnRenderDone();
	void	OnRcvPacket(GENERIC_PACKET *pPacket, int ServerIdx);
	void	OnTcpConnect(BOOL Connected, int ServerIdx);
	void	OnTcpAccept(WTcp *Tcp, LPSTR ClientAddress);

protected:
// Constants
	enum {
		BITMAP_RESOLUTION = 72,	// dots per inch
		PRIME_FRAMES = 2		// enough to keep pipeline full
	};

// Types
	typedef CSortArray<UINT, UINT&> CSortedFrame;

// Data members
	CMainFrame	*m_Main;		// pointer to main frame window
	CFracticeView	*m_View;	// pointer to view being recorded
	WTcp	*m_Listener;		// listens for server connections
	CRecordStatusDlg	m_RecStatDlg;	// record status dialog
	CServersDlg	m_ServersDlg;	// server dialog
	CBmpToAvi	m_RecAvi;		// records bitmaps to an AVI file
	CVideoComprState	m_ComprState;	// video compressor state
	CRecordInfo	m_RecInfo;		// recording info
	CJobControlDlg	m_JobCtrlDlg;	// job control dialog
	bool	m_Recording;		// true if recording
	UINT	m_CurFrame;			// index of next frame to be rendered
	UINT	m_LocalFrame;		// index of frame being rendered locally
	UINT	m_FramesDone;		// number of frames recorded
	UINT	m_CurJobID;			// uniquely identifies current job
	double	m_TotalRenderTime;	// total rendering time used, in seconds
	CString	m_DestPath;			// path of destination file or folder
	CDWordArray	m_RedoFrame;	// frames that failed to render and must be redone
	CString	m_BmpFolder;		// path of folder for bitmaps
	CRecBmpThread	m_BmpThread;	// worker thread to add bitmaps to AVI file
	bool	m_BmpThrLaunched;	// true if bitmap worker thread was launched
	CSortedFrame	m_PendingBmp;	// sorted list of bitmaps that can't be added
								// to AVI file yet, due to gaps in sequence
	UINT	m_MaxSeqBmp;		// highest bitmap index in uninterrupted sequence
	CFracticeView	*m_BatchView;	// view for running batch jobs
	CNetEngine	*m_Engine;		// attached engine for running strip job
	CRecordLegArray	m_Leg;		// array of recording legs, in start frame order
	int		m_CurLeg;			// index of current leg

// Helpers
	void	SetRecInfo();
	CString	GetBitmapPath(UINT FrameIdx) const;
	bool	CreateAvi(LPCTSTR Path);
	void	SetFrame(UINT FrameIdx);
	bool	RequestNextFrame(int ServerIdx);
	bool	PrimeServer(int ServerIdx);
	void	PrimeServers();
	bool	OutputFrame(UINT FrameIdx, CDib& Frame);
	bool	BitmapSetup();
	void	BitmapCleanup();
	static	void	ReadFrameNumbers(LPCTSTR Folder, LPCTSTR Ext, CSortedFrame& Frame);
	bool	BitmapRecoveryCheck(LPCTSTR Folder);
	bool	QueueJob(LPCTSTR Path);
	void	ListenHandler(bool Connected, LPCSTR ClientAddress);
	static	void	ListenHandler(bool Connected, LPCSTR ClientAddress, void *UserData);
	bool	BuildLegs(const CSnapshotArray& Snap, UINT TotalFrames);
};

inline CFracticeView *CRecorder::GetView()
{
	return(m_View);
}

inline CServersDlg& CRecorder::GetServersDlg()
{
	return(m_ServersDlg);
}

inline CRecordStatusDlg& CRecorder::GetRecordStatusDlg()
{
	return(m_RecStatDlg);
}

inline CJobControlDlg& CRecorder::GetJobControlDlg()
{
	return(m_JobCtrlDlg);
}

inline bool CRecorder::IsRecording() const
{
	return(m_Recording);
}

inline bool CRecorder::GetBatchMode() const
{
	return(m_JobCtrlDlg.GetBatchMode());
}

inline void CRecorder::GetRecordInfo(CRecordInfo& Info) const
{
	Info = m_RecInfo;
}

inline const CRecordInfo& CRecorder::GetRecordInfo() const
{
	return(m_RecInfo);
}

inline UINT CRecorder::GetFramesDone() const
{
	return(m_FramesDone);
}

inline bool CRecorder::IsListeningServers() const
{
	return(m_Listener != NULL);
}

inline bool CRecorder::IsDistributed() const
{
	return(m_ServersDlg.GetConnectionCount() > 0);
}

inline double CRecorder::GetTotalRenderTime() const
{
	return(m_TotalRenderTime);
}

inline CNetEngine *CRecorder::GetEngine() const
{
	return(m_Engine);
}

inline UINT CRecorder::GetCurJobID() const
{
	return(m_CurJobID);
}

inline int CRecorder::GetRedoCount() const
{
	return(INT64TO32(m_RedoFrame.GetSize()));
}

#endif
