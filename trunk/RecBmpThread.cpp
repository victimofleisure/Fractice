// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version

		worker thread to write bitmaps to AVI file 
 
*/

#include "stdafx.h"
#include "Resource.h"
#include "RecBmpThread.h"
#include "Recorder.h"
#include "BmpToAvi.h"
#include "PathStr.h"
#include "Dib.h"

bool CRecBmpThread::Create(HWND hWnd, UINT StartFrame, LPCTSTR BitmapFolder, CBmpToAvi *RecAvi)
{
	m_hWnd = hWnd;
	m_NextFrame = StartFrame;
	m_WriteCount = 0;
	m_BitmapFolder = BitmapFolder;
	m_RecAvi = RecAvi;
	return(Launch(ThreadFunc, this, THREAD_PRIORITY_BELOW_NORMAL) != NULL);
}

void CRecBmpThread::Abort(WPARAM wParam, LPARAM lParam)
{
	m_Running = FALSE;
	InterlockedExchange(&m_WriteCount, 0);
	PostMessage(m_hWnd, UWM_ABORTRECORD, wParam, lParam);
}

bool CRecBmpThread::MainLoop()
{
	while (WaitForWork()) {
		int	writes = m_WriteCount;
		for (int i = 0; i < writes; i++) {
			if (m_Canceled)
				break;
			CString	name;
			name.Format(RECORDER_BITMAP_NAME_FORMAT, m_NextFrame);
			CPathStr	path(m_BitmapFolder);
			path.Append(name);
			CDib	dib;
			if (!dib.Read(path)) {
				Abort(IDS_CANT_READ_BITMAP, 0);
				return(FALSE);
			}
			if (!m_RecAvi->AddFrame(dib)) {
				Abort(IDS_CANT_ADD_TO_AVI, 0);
				return(FALSE);
			}
			DeleteFile(path);
			m_NextFrame++;
		}
		InterlockedExchangeAdd(&m_WriteCount, -writes);
	}
	return(TRUE);
}

UINT CRecBmpThread::ThreadFunc(LPVOID pParam)
{
	CRecBmpThread	*pThread = (CRecBmpThread *)pParam;
	pThread->MainLoop();
	return(0);
}
