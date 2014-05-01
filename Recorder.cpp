// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		10jan09	in SetFrame, set bignum precision explicitly
		02		11jan09	move batch processing to job control dialog
		03		11feb09	add color cycling
		04		12feb09	in ListenServers, add ServerIP
		05		11jul09	allow unlimited record points
		06		12dec09	in Stop, call servers dialog's OnStop
		07		12dec09	move posting cancel to servers into net engine
		08		18dec09	use FastIsVisible
		09		22dec09	add Wrap for reverse color cycling
		10		25dec09	pass Aborted flag to servers dialog's OnStop
		11		27dec09	call OnStop before bitmap cleanup
		12		27dec09	in OnRenderDone, output frame before starting render
		13		06jan10	W64: use GetRedoCount

        recording subsystem
 
*/

#include "stdafx.h"
#include "Fractice.h"
#include <math.h>
#include <float.h>	// for range of double
#include "Recorder.h"
#include "FracticeDoc.h"
#include "FracticeView.h"
#include "MainFrm.h"
#include "PathStr.h"
#include "ProgressDlg.h"
#include "FolderDialog.h"
#include "MultiFileDlg.h"

CRecorder::CRecorder()
{
	m_Main = NULL;
	m_View = NULL;
	m_Listener = FALSE;
	m_Recording = FALSE;
	m_CurFrame = 0;
	m_LocalFrame = 0;
	m_FramesDone = 0;
	m_CurJobID = 0;
	m_TotalRenderTime = 0;
	m_BmpThrLaunched = FALSE;
	m_MaxSeqBmp = 0;
	m_BatchView = NULL;
	m_Engine = NULL;
	m_CurLeg = 0;
}

static inline double Wrap(double Val, double Limit)
{
	double	r = fmod(Val, Limit);
	return(Val < 0 ? r + Limit : r);
}

bool CRecorder::BuildLegs(const CSnapshotArray& Snap, UINT TotalFrames)
{
	int	snaps = m_RecInfo.m_Snap.GetSize();
	CArrayEx<double, double&>	LogDZ;
	LogDZ.SetSize(snaps);
	double	TotLogDZ = 0;
	int	i;
	for (i = 0; i < snaps - 1; i++) {	// compute delta zoom in log space
		const CSnapshot&	s0 = Snap[i];
		const CSnapshot&	s1 = Snap[i + 1];
		BigNum	dz(s1.m_Zoom / s0.m_Zoom);
		if (dz > DBL_MAX || dz < DBL_MIN) {
			AfxMessageBox(IDS_REC_ZOOM_OUT_OF_RANGE);
			return(FALSE);
		}
		double	ldz = fabs(log(dz));
		LogDZ[i] = ldz;
		TotLogDZ += ldz;
	}
	if (TotLogDZ == 0) {	// if no change in zoom
		if (snaps > 2) {	// if multi-leg recording, error
			AfxMessageBox(IDS_REC_NOT_ENOUGH_ZOOM);
			return(FALSE);
		}
		// single-leg recording: zoom change not required
		LogDZ[0] = 1;	// avoid divide by zero in next step
		TotLogDZ = 1;
	}
	m_Leg.RemoveAll();	// remove all legs
	CRecordLeg	leg;
	leg.SetMathPrec(Snap[0].m_MathPrec);
	int	BadLeg = -1;
	UINT	FramesUsed = 0;
	for (i = 0; i < snaps - 1; i++) {
		UINT	frames = round(LogDZ[i] / TotLogDZ * TotalFrames);
		if (frames > 1) {	// if enough frames for a leg
			const CSnapshot&	s0 = Snap[BadLeg >= 0 ? BadLeg : i];
			const CSnapshot&	s1 = Snap[i + 1];
			leg.m_StartFrame = FramesUsed;
			leg.m_Frames = frames;
			leg.m_Origin[0] = s0.m_Origin;
			leg.m_Origin[1] = s1.m_Origin;
			leg.m_Zoom[0] = s0.m_Zoom;
			leg.m_Zoom[1] = s1.m_Zoom;
			m_Leg.Add(leg);	// add a leg
			BadLeg = -1;
			FramesUsed += frames;
		} else {	// too few frames: leg is useless, skip it
			if (BadLeg < 0)	// if we're not already skipping legs
				BadLeg = i;	// save index for start of next good leg
		}
	}
	int	legs = m_Leg.GetSize();
	if (!legs) {
		AfxMessageBox(IDS_REC_NOT_ENOUGH_ZOOM);
		return(FALSE);
	}
	CRecordLeg&	LastLeg = m_Leg[legs - 1];
	if (BadLeg >= 0) {	// if we ended on a bad leg, update final coordinates
		const CSnapshot&	LastSnap = Snap[snaps - 1];
		LastLeg.m_Origin[1] = LastSnap.m_Origin;
		LastLeg.m_Zoom[1] = LastSnap.m_Zoom;
	}
	LastLeg.m_Frames = TotalFrames - LastLeg.m_StartFrame;	// use all frames
	LastLeg.m_Frames--;	// adjust last leg so we reach final snapshot
	for (i = 0; i < legs; i++)
		m_Leg[i].CalcSteps();	// calculate increments for each leg
	LastLeg.m_Frames++;	// restore last leg
	m_CurLeg = 0;
	return(TRUE);
}

void CRecorder::SetRecordInfo(const CRecordInfo& Info)
{
	m_RecInfo = Info;
	m_CurFrame = Info.m_StartFrame;
	m_LocalFrame = Info.m_StartFrame;
	m_FramesDone = Info.m_StartFrame;
	m_CurJobID = GetTickCount();
	m_TotalRenderTime = 0;
	m_RedoFrame.RemoveAll();
	m_BmpFolder.Empty();
	m_BmpThrLaunched = FALSE;
	m_PendingBmp.RemoveAll();
	m_MaxSeqBmp = 0;
}

bool CRecorder::Create()
{
	m_Main = theApp.GetMain();
	if (!m_RecStatDlg.Create(IDD_RECORD_STATUS)
	|| !m_ServersDlg.Create(IDD_SERVERS)
	|| !m_JobCtrlDlg.Create(this, IDD_JOB_CONTROL))
		return(FALSE);
	if (m_Main->GetOptionsDlg().GetListenServers())
		ListenServers(TRUE);
	return(TRUE);
}

void CRecorder::Destroy()
{
	if (m_Recording)
		m_RecAvi.Close();
	delete m_Listener;
	m_Listener = NULL;
}

CString CRecorder::GetBitmapPath(UINT FrameIdx) const
{
	CString	name;
	name.Format(RECORDER_BITMAP_NAME_FORMAT, FrameIdx);
	CPathStr	path(m_BmpFolder);
	path.Append(name);
	return(path);
}

bool CRecorder::CreateAvi(LPCTSTR Path)
{
	static const LPCTSTR COMPR_STATE_FILE_NAME = _T("ComprState.dat");
	BMPTOAVI_PARMS	Parms = {
		m_RecInfo.m_Snap[0].m_ImageSize.cx,
		m_RecInfo.m_Snap[0].m_ImageSize.cy,
		m_View->GetColorDepth(),
		m_RecInfo.m_FrameRate
	};
	CPathStr	StateFolder, StatePath;
	if (GetBatchMode())
		m_RecAvi.SetComprState(m_ComprState);
	else {
		theApp.GetAppDataFolder(StateFolder);
		StatePath = StateFolder;
		StatePath.Append(COMPR_STATE_FILE_NAME);
		if (PathFileExists(StatePath) && m_ComprState.Read(StatePath))
			m_RecAvi.SetComprState(m_ComprState);
	}
	if (!m_RecAvi.Open(Parms, Path, !GetBatchMode())) {
		if (m_RecAvi.GetLastError()) {	// if user didn't cancel
			CString	msg, Err, DSErr;
			m_RecAvi.GetLastErrorString(Err, DSErr);
			AfxFormatString2(msg, IDS_REC_CANT_CREATE_FILTER, Err, DSErr);
			Abort(msg);
		}
		return(FALSE);
	}
	if (!GetBatchMode()) {
		m_RecAvi.GetComprState(m_ComprState);
		if (!PathFileExists(StatePath))	// test using shlwapi, it's faster
			theApp.CreateFolder(StateFolder);
		m_ComprState.Write(StatePath);	// write compressor state
	}
	return(TRUE);
}

void CRecorder::SetFrame(UINT FrameIdx)
{
	if (!m_Leg[m_CurLeg].SpansFrame(FrameIdx)) {	// if wrong leg
		int	legs = m_Leg.GetSize();
		int	i;
		for (i = 0; i < legs; i++) {	// for each leg
			if (m_Leg[i].SpansFrame(FrameIdx)) {	// if leg spans frame
				m_CurLeg = i;	// switch legs
				break;
			}
		}
		ASSERT(i < legs);	// right leg was found
	}
	const CRecordLeg&	leg = m_Leg[m_CurLeg];
	if (m_RecInfo.m_CycleColors) {
		double	delta = m_RecInfo.m_ColorCycleRate / m_RecInfo.m_FrameRate;
		double	offset = Wrap(m_RecInfo.m_Snap[0].m_ColorOffset 
			+ delta * FrameIdx, 1);
		m_View->OffsetColor(offset);	// doesn't render, only updates palette
	}
	UINT	LegFrame = FrameIdx - leg.m_StartFrame;
	BigPoint	Origin(leg.m_Origin[0] + leg.m_OriginStep * LegFrame);
	// GMP's pow function doesn't support fractional exponents, so we must use
	// standard library pow instead; this limits movie zoom to range of double
	BigNum	z(leg.m_MathPrec, pow(leg.m_ZoomStep, double(LegFrame)), 0);
	BigNum	Zoom(leg.m_Zoom[0] * z);
	int	idx;
	if (leg.m_ZoomingOut) {
		z *= leg.m_ZoomRange;
		idx = 0;
	} else
		idx = 1;
	// compensate origin for zoom so velocity in x/y plane remains constant
	Origin += (leg.m_Origin[idx] - Origin) * ((z - UINT(1)) / z);
	m_View->SetCoords(Origin, Zoom);	// no preview
}

bool CRecorder::QueueJob(LPCTSTR Path)
{
	if (!m_RecInfo.m_ToBitmaps) {
		CString	TempFilePath;
		theApp.GetTempFileName(TempFilePath);
		bool	retc = CreateAvi(TempFilePath);
		if (!retc) {
			DeleteFile(TempFilePath);
			return(FALSE);
		}
		m_RecAvi.Close();
		DeleteFile(TempFilePath);
	}
	CJobInfo	Info;
	GetJobInfo(Info);
	Info.m_RecordPath = Path;
	Info.m_Source = m_View->GetDocument()->GetTitle();
	Info.m_Dest = PathFindFileName(Path);
	m_JobCtrlDlg.Add(Info);
	return(TRUE);
}

bool CRecorder::Record(const CRecordInfo& Info)
{
	CString	Path;
	if (Info.m_ToBitmaps) {
		CString	Title((LPCTSTR)IDS_REC_BMP_DEST_FOLDER);
		int	flags = BIF_EDITBOX | BIF_NEWDIALOGSTYLE;
		CString	InitDir = theApp.m_RecBmpFolder;
		if (InitDir.IsEmpty())
			InitDir = theApp.m_RecAviFolder;
		if (!CFolderDialog::BrowseFolder(Title, Path, NULL, flags, InitDir))
			return(FALSE);
		theApp.m_RecBmpFolder = Path;
	} else {
		CString	Title((LPCTSTR)IDS_RECORD);
		CMultiFileDlg	fd(FALSE, _T(".avi"), NULL, OFN_OVERWRITEPROMPT,
			LDS(IDS_AVI_FILTER), NULL, Title, &theApp.m_RecAviFolder);
		if (fd.DoModal() != IDOK)
			return(FALSE);
		Path = fd.GetPathName();
	}
	bool	AddToJobCtrl = m_Main->GetRecordDlg().GetQueueJob();
	return(Record(Info, m_Main->GetView(), Path, AddToJobCtrl));
}

bool CRecorder::Record(const CRecordInfo& Info, CFracticeView *View, LPCTSTR Path, bool AddToJobCtrl) 
{
	if (m_Recording)	// if already recording
		return(TRUE);	// nothing to do
	if (!Info.m_FrameCount) {	// if invalid duration
		Abort(IDS_REC_BAD_DURATION);
		return(FALSE);
	}
	if (View == NULL) {
		m_Main->SendMessage(WM_COMMAND, ID_FILE_NEW);	// create batch job view
		View = m_Main->GetView();
		if (View == NULL)
			return(FALSE);
		m_BatchView = View;	// save pointer to batch job view
	}
	m_View = View;
	SetRecordInfo(Info);	// set member vars first
	if (!BuildLegs(Info.m_Snap, Info.m_FrameCount))
		return(FALSE);
	m_DestPath = Path;
	if (AddToJobCtrl)
		return(QueueJob(Path));
	if (m_RecInfo.m_ToBitmaps) {
		if (!BitmapRecoveryCheck(Path))
			return(FALSE);	// bitmap recovery failed
	} else {
		if (!CreateAvi(Path))
			return(FALSE);	// can't create AVI file
	}
	if (!BitmapSetup())
		return(FALSE);
	m_Recording = TRUE;
	m_Main->OnRecord(TRUE);
	m_RecStatDlg.SetView(m_View);
	m_RecStatDlg.OnRecord();
	m_ServersDlg.OnRecord();
	if (!GetBatchMode())
		m_RecStatDlg.ShowWindow(SW_SHOW);
	{
		CFracticeView::CDeferRender	defer(*m_View, FALSE);	// don't render at all
		m_View->SetSnapshot(m_RecInfo.m_Snap[0]);	// set rendering attributes
	}
	PrimeServers();	// start remote renders
	SetFrame(m_LocalFrame);	// start local render
	return(TRUE);
}

bool CRecorder::Stop()
{
	if (!m_Recording)
		return(TRUE);	// nothing to do
	bool	Aborted = m_FramesDone <= m_RecInfo.m_EndFrame;
	if (Aborted)
		m_View->CancelRender();
	m_Recording = FALSE;
	m_ServersDlg.OnStop(Aborted);	// before cleanup to keep progress consistent
	m_RecStatDlg.OnStop();
	BitmapCleanup();	// could take a while, shows a progress bar
	m_Main->OnRecord(FALSE);
	m_Main->SetFocus();
	m_CurJobID = 0;	// so any further received bitmaps flunk ID check in OnRcvPacket
	bool	retc = m_RecAvi.Close();
	m_View = NULL;	// order matters; do before StartNextJob
	if (m_BatchView != NULL) {
		m_BatchView->GetParent()->PostMessage(WM_CLOSE);	// destroy batch job view
		m_BatchView = NULL;
	}
	if (GetBatchMode()) {
		if (Aborted)
			m_JobCtrlDlg.AbortJob();
		else
			m_JobCtrlDlg.FinishJob();
	}
	return(retc);
}

bool CRecorder::StopCheck()
{
	if (!m_Recording)
		return(TRUE);	// nothing to do
	if (AfxMessageBox(IDS_MF_STOP_RECORD, MB_YESNO | MB_DEFBUTTON2) == IDNO)
		return(FALSE);
	if (GetBatchMode())
		return(m_JobCtrlDlg.SetBatchMode(FALSE));
	return(Stop());	// stop recording
}

void CRecorder::Abort(LPCTSTR Msg)
{
	if (GetBatchMode()) {
		m_JobCtrlDlg.FailJob(Msg);
		Stop();
	} else {
		Stop();
		AfxMessageBox(Msg);
	}
}

void CRecorder::Abort(UINT MsgID)
{
	CString	msg;
	msg.LoadString(MsgID);
	Abort(msg);
}

void CRecorder::ReadFrameNumbers(LPCTSTR Folder, LPCTSTR Ext, CSortedFrame& Frame)
{
	CPathStr	path(Folder);
	path.Append(CString("*") + Ext);
	CFileFind	ff;
	BOOL	bWorking = ff.FindFile(path);
	while (bWorking) {
		bWorking = ff.FindNextFile();
		if (!_tcsicmp(PathFindExtension(ff.GetFileName()), Ext)) {
			UINT	frame;
			if (_stscanf(ff.GetFileName(), _T("%u"), &frame) == 1)
				Frame.Add(frame);
		}
	}
	Frame.Sort();
}

bool CRecorder::BitmapRecoveryCheck(LPCTSTR Folder)
{
	if (!m_RecInfo.m_ToBitmaps)
		return(TRUE);	// nothing to do
	CMainFrame::CStatusMsg	status(IDS_REC_CHECKING_BITMAPS);
	CSortedFrame	PrevFrame;
	ReadFrameNumbers(Folder, BITMAP_EXT, PrevFrame);
	if (!PrevFrame.GetSize())
		return(TRUE);	// no previous frames were found
	// destination folder already contains a frame sequence
	if (!GetBatchMode()) {	// if not in batch mode
		// ask user if we're continuing a previously aborted recording
		CString	msg;
		AfxFormatString1(msg, IDS_REC_BMP_CONTINUE, Folder);
		int	retc = AfxMessageBox(msg, MB_YESNOCANCEL);
		if (retc == IDCANCEL)
			return(FALSE);	// user canceled
		if (retc == IDNO) {	// user says we're starting a new recording
			AfxFormatString1(msg, IDS_REC_BMP_OVERWRITE, Folder);	// warn again
			return(AfxMessageBox(msg, MB_YESNO | MB_DEFBUTTON2) == IDYES);
		}
	}
	int	PrevIdx = 0;
	int	prevs = PrevFrame.GetSize();
	// skip over any previous frames below start frame
	while (PrevIdx < prevs && PrevFrame[PrevIdx] < m_RecInfo.m_StartFrame)
		PrevIdx++;
	if (PrevIdx >= prevs)	// if all previous frames are below start frame
		return(TRUE);	// previous frames can be ignored
	UINT	i;
	for (i = m_RecInfo.m_StartFrame; i <= m_RecInfo.m_EndFrame; i++) {
		if (PrevIdx >= prevs)
			break;
		if (PrevFrame[PrevIdx] == i)	// if sequences match
			PrevIdx++;	// keep going
		else	// found gap in previous frame sequence
			m_RedoFrame.Add(i);	// add frame to redo list
	}
	if (i > m_RecInfo.m_EndFrame) {	// if we traversed entire frame range
		if (!GetRedoCount()) {	// if we didn't find any gaps
			Abort(IDS_REC_COMPLETE);
			return(FALSE);	// no need to record anything
		}
		i = m_RecInfo.m_EndFrame;	// minimum frame range is one frame
	}
	m_CurFrame = i;
	m_LocalFrame = i;
	m_FramesDone = i - GetRedoCount();
	return(TRUE);
}

bool CRecorder::RequestNextFrame(int ServerIdx)
{
	CServer&	srv = m_ServersDlg.GetServer(ServerIdx);
	if (!srv.IsConnected())
		return(FALSE);	// server is disconnected
	UINT	NextFrame;
	int	redos = GetRedoCount();
	if (redos)	// if frames need redoing
		NextFrame = m_RedoFrame[redos - 1];	// render is a redo
	else {	// no redos
		if (m_CurFrame >= m_RecInfo.m_EndFrame)
			return(FALSE);	// no more frames to render
		NextFrame = m_CurFrame + 1;	// don't update current frame yet
	}
	CFracticeView::CDeferRender	defer(*m_View, FALSE);	// don't render at all
	SetFrame(NextFrame);
	CSnapshot	snap;
	m_View->GetSnapshot(snap);
	if (!srv.WriteSnapshot(m_CurJobID, NextFrame, snap))
		return(FALSE);	// can't send snapshot to server
	// frame was successfully sent to server, so update our state
	srv.AddPending(NextFrame);	// add frame to server's pending list
	if (redos)	// if render was a redo
		m_RedoFrame.SetSize(redos - 1);	// remove it from redo list
	else	// no redos
		m_CurFrame = NextFrame;	// update current frame
	return(TRUE);
}

bool CRecorder::PrimeServer(int ServerIdx)
{
	CServer&	srv = m_ServersDlg.GetServer(ServerIdx);
	if (!srv.IsConnected())
		return(FALSE);	// server is disconnected
	while (srv.GetPendingCount() < PRIME_FRAMES) {
		if (!RequestNextFrame(ServerIdx))
			return(FALSE);
	}
	return(TRUE);
}

void CRecorder::PrimeServers()
{
	int	servs = m_ServersDlg.GetServerCount();
	for (int i = 0; i < servs; i++)
		PrimeServer(i);
}

bool CRecorder::BitmapSetup()
{
	int	conns = m_ServersDlg.GetConnectionCount();
	if (m_BmpFolder.IsEmpty() && (m_RecInfo.m_ToBitmaps || conns)) {
		CPathStr	path(m_DestPath);
		if (!m_RecInfo.m_ToBitmaps) {
			path.RemoveExtension();
			path += RECORDER_BITMAP_FOLDER_SUFFIX;
		}
		if (!theApp.CreateFolder(path)) {	// create subfolder for bitmaps
			Abort(IDS_CANT_CREATE_BMP_FOLDER);
			return(FALSE);	// can't create folder
		}
		m_BmpFolder = path;
	}
	if (!m_BmpThrLaunched && !m_RecInfo.m_ToBitmaps && conns) {
		if (!m_BmpThread.Create(m_View->m_hWnd, m_CurFrame, m_BmpFolder, &m_RecAvi)) {
			Abort(IDS_CANT_LAUNCH_BMP_THREAD);
			return(FALSE);
		}
		m_MaxSeqBmp = m_CurFrame;
		m_BmpThrLaunched = TRUE;
	}
	return(TRUE);
}

void CRecorder::BitmapCleanup()
{
	if (m_BmpThread.IsRunning()) {
		CProgressDlg	dlg;
		dlg.Create();
		dlg.GetDlgItem(IDCANCEL)->ShowWindow(SW_HIDE);
		CString	cap((LPCTSTR)IDS_REC_FINISHING);
		dlg.SetWindowText(cap);
		dlg.SetRange(m_BmpThread.GetNextFrame(), m_MaxSeqBmp);
		dlg.ShowPercent(TRUE);
		while (m_BmpThread.GetWriteCount() > 0) {	// wait for thread to finish writing
			MsgWaitForMultipleObjects(0, NULL, FALSE, 100, QS_ALLINPUT);
			dlg.SetPos(m_BmpThread.GetNextFrame());	// SetPos pumps messages
		}
		m_BmpThread.Kill();
	}
	if (!m_BmpFolder.IsEmpty() && !m_RecInfo.m_ToBitmaps)
		theApp.DeleteDirectory(m_BmpFolder);
}

bool CRecorder::OutputFrame(UINT FrameIdx, CDib& Frame)
{
	if (m_BmpFolder.IsEmpty()) {
		if (!m_RecAvi.AddFrame(Frame)) {	// write frame to AVI file
			if (m_RecAvi.GetLastError()) {
				CString	Msg, Err, DSErr;
				m_RecAvi.GetLastErrorString(Err, DSErr);
	 			Msg.Format(_T("%s\n%s"), Err, DSErr);
				Abort(Msg);
			} else
				Abort(IDS_CANT_ADD_TO_AVI);
			return(FALSE);
		}
	} else {	// write to bitmap folder
		CString	path(GetBitmapPath(FrameIdx));
		CFileException	e;
		if (!Frame.Write(path, BITMAP_RESOLUTION, &e)) {
			if (e.m_cause != CFileException::none) {
				TCHAR	msg[256];
				e.GetErrorMessage(msg, sizeof(msg));
				Abort(msg);
			} else
				Abort(IDS_CANT_WRITE_BITMAP);
			return(FALSE);
		}
		if (m_BmpThrLaunched) {	// if bitmap thread was launched
			if (FrameIdx == m_MaxSeqBmp) {	// if frame is next in sequence
				m_MaxSeqBmp++;	// expand sequence to include current frame
				// if any pending bitmaps follow sequentially, output them too
				// pending list is assumed to be in ascending order
				int	pends = m_PendingBmp.GetSize();
				for (int i = 0; i < pends; i++) {	// for each pending bitmap	
					if (m_PendingBmp[i] == m_MaxSeqBmp)	// if it's next in sequence
						m_MaxSeqBmp++;	// expand sequence to include it
					else	// found gap in pending bitmap list
						break;	// remaining bitmaps stay pending for now
				}
				int	writes = m_MaxSeqBmp - FrameIdx;	// total bitmaps to output
				m_BmpThread.WriteFrames(writes);	// start bitmap thread working
				writes--;	// exclude current frame
				if (writes > 0)	// if any pending bitmaps were output
					m_PendingBmp.RemoveAt(0, writes);	// they're no longer pending

			} else if (FrameIdx > m_MaxSeqBmp) {	// if frame isn't a duplicate
				// add frame to pending list, keeping list in ascending order
				m_PendingBmp.SortedInsert(FrameIdx);
			}
		}
	}
	return(TRUE);
}

void CRecorder::OnRenderDone()
{
	// output frame before starting next render to avoid race with engine
	double	RenderTime = m_View->GetLastRenderTime();
	m_TotalRenderTime += RenderTime;
	m_ServersDlg.OnFrameDone(CServersDlg::LOCAL_HOST, 0, RenderTime);
	if (!OutputFrame(m_LocalFrame, m_View->GetImage()))
		return;	// output error already handled
	m_FramesDone++;
	if (m_FramesDone > m_RecInfo.m_EndFrame) {
		Stop();
		return;	// recording is done
	}
	int	redos = GetRedoCount();
	if (redos) {	// if frames need redoing
		m_LocalFrame = m_RedoFrame[redos - 1];	// render is a redo
		m_RedoFrame.SetSize(redos - 1);	// remove frame from redo list
		SetFrame(m_LocalFrame);	// start rendering
	} else {	// no redos
		if (m_CurFrame < m_RecInfo.m_EndFrame) {	// if more frames to render
			m_CurFrame++;
			m_LocalFrame = m_CurFrame;
			SetFrame(m_CurFrame);	// start rendering
		}
	}
}

void CRecorder::OnRcvPacket(GENERIC_PACKET *pPacket, int ServerIdx)
{
	switch (pPacket->Message) {
	case PMID_DIB:
		if (m_Recording) {
			FRAP_DIB	*pp = (FRAP_DIB *)pPacket;
			if (pp->JobID != m_CurJobID)	// if job ID doesn't match
				break;	// spurious frame, just ignore it
			RequestNextFrame(ServerIdx);	// start rendering ASAP
			m_TotalRenderTime += pp->RenderTime;
			m_ServersDlg.OnFrameDone(ServerIdx, pp->FrameID, pp->RenderTime);
			CDib	dib;
			if (dib.Create(pp->Width, pp->Height, pp->BitCount)) {
				memcpy(dib.GetBits(), pp->DibBits, pp->DibLen);
				if (!OutputFrame(pp->FrameID, dib))
					break;	// output error already handled
			}
			m_FramesDone++;
			if (m_FramesDone > m_RecInfo.m_EndFrame)
				Stop();
		}
		break;
	case PMID_PROGRESS_REPORT:
		{
			FRAP_MESSAGE	*pp = (FRAP_MESSAGE *)pPacket;
			CServer&	srv = m_ServersDlg.GetServer(ServerIdx);
			srv.SetProgress(pp->wParam);
			if (m_ServersDlg.FastIsVisible()) {
				m_ServersDlg.RefreshRow(ServerIdx,	// refresh list control row
					CServersDlg::COL_PROGRESS);
			}
		}
		break;
	case PMID_STRIP_DONE:
		if (m_Engine != NULL) {	// if an engine is attached to us
			FRAP_STRIP_DONE	*pp = (FRAP_STRIP_DONE *)pPacket;
			if (pp->JobID == m_Engine->GetCurUID()) {	// if job IDs match
				m_Engine->WriteRemoteStrip(ServerIdx, pp->ThreadIdx,
					pp->y1, pp->y2, pp->Strip, pp->RenderTime);
			}
		}
		break;
	}
	delete pPacket;	// recipient is responsible for cleanup
}

void CRecorder::OnTcpConnect(BOOL Connected, int ServerIdx)
{
	// if authentication fails, server isn't added to server array, but connect
	// and disconnect notifications are posted anyway, so validate server index
	if (Connected) {
		if (ServerIdx < m_ServersDlg.GetServerCount()) {	// validate index
			if (m_Recording) {
				if (BitmapSetup())
					PrimeServer(ServerIdx);	// get server working
				else
					Stop();
			} else {
				if (m_Engine != NULL)	// if an engine is attached to us
					m_Engine->PrimeServer(ServerIdx);	// get server working
			}
		}
	} else {	// disconnected
		if (ServerIdx < m_ServersDlg.GetServerCount()) {	// validate index
			if (m_Recording) {
				// add server's pending frames to redo list
				CServer&	srv = m_ServersDlg.GetServer(ServerIdx);
				int	pending = srv.GetPendingCount();
				for (int i = 0; i < pending; i++)
					m_RedoFrame.Add(srv.GetPendingItem(i));
			} else {
				if (m_Engine != NULL)	// if an engine is attached to us
					m_Engine->DetachServer(ServerIdx);	// detach from server
			}
		}
		m_ServersDlg.UpdateList();	// refresh servers dialog
	}
}

void CRecorder::GetJobInfo(CJobInfo& Info) const
{
	m_Main->GetRecordDlg().GetInfo(Info.m_RecInfo);
	Info.m_ComprState = m_ComprState;
}

void CRecorder::SetJobInfo(const CJobInfo& Info)
{
	m_Main->GetRecordDlg().SetInfo(Info.m_RecInfo);
	m_ComprState = Info.m_ComprState;
}

bool CRecorder::ListenServers(bool Enable)
{
	if (IsListeningServers()) {
		delete m_Listener;
		m_Listener = NULL;
	}
	if (Enable) {
		try {
			COptionsDlg&	OptsDlg = theApp.GetMain()->GetOptionsDlg();
			WORD	PortNumber = OptsDlg.GetNetworkPort();
			LPCTSTR	ServerIP = OptsDlg.GetNicIPAddressOrNull();
#ifdef UNICODE
			USES_CONVERSION;
			m_Listener = new WTcp(NULL, PortNumber, W2CA(ServerIP));
#else
			m_Listener = new WTcp(NULL, PortNumber, ServerIP);
#endif
			m_Listener->InstallConnectionHandler(ListenHandler, this);
		}
		catch (const WError& e) {
			CString	s((LPCTSTR)IDS_REC_CANT_LISTEN_SERVERS);
			s += CString("\n") + e.what();
			AfxMessageBox(s);
			return(FALSE);
		}
	}
	return(TRUE);
}

void CRecorder::ListenHandler(bool Connected, LPCSTR ClientAddress)
{
	if (Connected) {
		WTcp	*Tcp;
		if (m_Listener->GetConnection(Tcp)) {	// get TCP object from listener
			// recipient is responsible for deleting both TCP and ClientAddress
			PostMessage(m_Main->m_hWnd, UWM_TCPACCEPT, 
				(WPARAM)Tcp, (LPARAM)_strdup(ClientAddress));
		}
	}
}

void CRecorder::ListenHandler(bool Connected, LPCSTR ClientAddress, void *UserData)
{
	CRecorder	*This = (CRecorder *)UserData;
	This->ListenHandler(Connected, ClientAddress);
}

void CRecorder::OnTcpAccept(WTcp *Tcp, LPSTR ClientAddress)
{
#ifdef UNICODE
	USES_CONVERSION;
	m_ServersDlg.AddServer(A2CW(ClientAddress), NULL, NULL, FALSE, Tcp);
#else
	m_ServersDlg.AddServer(ClientAddress, NULL, NULL, FALSE, Tcp);
#endif
	// AddServer takes care of deleting TCP object
	free(ClientAddress);	// we take care of deleting ClientAddress
}

void CRecorder::AttachEngine(CNetEngine *Engine)
{
	ASSERT(Engine != NULL);
	if (Engine != m_Engine) {
		if (m_Engine != NULL)	// if an engine is already attached to us
			m_Engine->DetachAllServers();	// detach it from all servers
		m_Engine = Engine;	// attach new engine
	}
}

void CRecorder::DetachEngine(CNetEngine *Engine)
{
	ASSERT(Engine != NULL);
	if (Engine == m_Engine)	// if this engine is attached to us
		m_Engine = NULL;	// detach it
}
