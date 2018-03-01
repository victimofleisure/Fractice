// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      11jan09	initial version
		01		16jan09	add pause/resume
		02		06jan10	W64: in ResumePosterJob, cast row array size to 32-bit

        fractal rendering engine for posters
 
*/

#include "stdafx.h"
#include "Fractice.h"
#include "PosterEngine.h"

CPosterEngine::CPosterEngine()
{
	m_PosterAntialias = 0;
	m_ResumingJob = FALSE;
}

void CPosterEngine::BeginPosterJob(const RENDER_INFO& Info, const BigRect& Bounds, const CSnapshot& Snap, LPCTSTR RowFilePath)
{
	CFileException	e;
	UINT	flags = CFile::modeReadWrite;
	if (!m_ResumingJob)	// if not resuming a job
		flags |= CFile::modeCreate;	// create new file
	if (!m_RowFile.Open(RowFilePath, flags, &e))
		AfxThrowFileException(e.m_cause, e.m_lOsError, e.m_strFileName);
	// if m_RenderToFile is true, BeginStripJob skips initializing m_NextRow,
	// m_RowsDone, and RedoStrip, thereby allowing us to resume a paused job
	if (m_ResumingJob) {	// if resuming a paused job
		m_RowFile.SeekToEnd();	// position row file at end
	} else {	// starting a new job
		m_NextRow = 0;
		m_RowsDone = 0;
		m_RedoStrip.RemoveAll();
		m_RowIndex.SetSize(Info.FrameSize.cy);
		memset(m_RowIndex.GetData(), -1, Info.FrameSize.cy * sizeof(DWORD));
	}
	m_RowFilePath = RowFilePath;
	m_EscFrameSize = Info.FrameSize;
	m_PosterAntialias = Snap.m_Antialias;
	m_RenderToFile = TRUE;
	BeginStripJob(Info, Bounds, Snap);
}

void CPosterEngine::WriteDibHeader(CFile& File, const BITMAP& Bmp, float Resolution)
{
	DIBSECTION	ds;
	ZeroMemory(&ds, sizeof(DIBSECTION));
	ds.dsBm = Bmp;
	ds.dsBmih.biSize = sizeof(BITMAPINFOHEADER);
	ds.dsBmih.biWidth = Bmp.bmWidth;
	ds.dsBmih.biHeight = Bmp.bmHeight;
	ds.dsBmih.biPlanes = Bmp.bmPlanes;
	ds.dsBmih.biBitCount = Bmp.bmBitsPixel;
	ds.dsBmih.biSizeImage = Bmp.bmWidthBytes * Bmp.bmHeight;
	BITMAPFILEHEADER	bfh;
	ZeroMemory(&bfh, sizeof(BITMAPFILEHEADER));
	bfh.bfType = 0x4d42;	// BM
	bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	bfh.bfSize = bfh.bfOffBits + ds.dsBmih.biSizeImage;
	static const double INCHES_PER_METER = .0254;
	int	MetricRes = round(Resolution / INCHES_PER_METER);
	ds.dsBmih.biXPelsPerMeter = MetricRes;
	ds.dsBmih.biYPelsPerMeter = MetricRes;
	File.Write(&bfh, sizeof(BITMAPFILEHEADER));
	File.Write(&ds.dsBmih, sizeof(BITMAPINFOHEADER));
}

void CPosterEngine::FileSeek64(CFile& File, __int64 llOff, UINT nFrom)
{
	ASSERT(File.m_hFile != CFile::hFileNull);
	LARGE_INTEGER	li;
	li.QuadPart = llOff;
	DWORD dwNew = SetFilePointer((HANDLE)File.m_hFile, li.LowPart, &li.HighPart, nFrom);
	if (dwNew == (DWORD)-1 && GetLastError() != NO_ERROR)
		CFileException::ThrowOsError((LONG)::GetLastError());
}

bool CPosterEngine::WritePoster(LPCTSTR BitmapPath, WORD BitCount, float Resolution)
{
	CSize	FrameSize = m_EscFrameSize;
	UINT	Antialias = m_PosterAntialias;
	CSize	ImgSize = CSize(FrameSize.cx / Antialias, FrameSize.cy / Antialias);
	CSize	DibSize = CSize(ImgSize.cx, 1);	// make frame DIB one pixel high
	m_RowFile.Flush();
	if (!CreateFrame(DibSize, BitCount, Antialias))
		return(FALSE);
	CFile	DibFile(BitmapPath, CFile::modeCreate | CFile::modeWrite);
	BITMAP	bi;
	m_Dib.GetBitmap(&bi);	// get destination bitmap attributes from frame
	bi.bmHeight = ImgSize.cy;	// but replace frame height with image height
	WriteDibHeader(DibFile, bi, Resolution);
	int	RowBytes = FrameSize.cx * sizeof(ESCTIME);
	int	row = 0;
	for (int i = 0; i < ImgSize.cy; i++) {
		ESCTIME	*pEscFrame = m_EscFrame.GetData();
		for (UINT j = 0; j < Antialias; j++) {
			__int64	FilePos = __int64(m_RowIndex[row]) * RowBytes;
			FileSeek64(m_RowFile, FilePos, FILE_BEGIN);
			m_RowFile.Read(pEscFrame, RowBytes);
			pEscFrame += FrameSize.cx;
			row++;
		}
		MapColor();
		DibFile.Write(m_Dib.GetBits(), m_DibInfo.Length);
	}
	m_RowFile.Close();
	CFile::Remove(m_RowFilePath);
	return(TRUE);
}

void CPosterEngine::EndPosterJob()
{
	CancelRender(TRUE);	// just in case; wait for idle
	if (IsRowFileOpen())
		m_RowFile.Close();
}

bool CPosterEngine::ResumePosterJob(const CDWordArray& RowIndex)
{
	m_RedoStrip.RemoveAll();
	m_NextRow = 0;
	m_RowsDone = 0;
	int	rows = INT64TO32(RowIndex.GetSize());
	if (!rows)
		return(FALSE);
	m_RowIndex.Copy(RowIndex);	// set member row index
	int	i;
	for (i = rows - 1; i >= 0; i--) {	// reverse search
		if (m_RowIndex[i] != DWORD(-1))	// if valid index entry
			break;	// found last entry
	}
	m_NextRow = i + 1;	// start sequential processing after last entry
	for (i = 0; i < m_NextRow; i++) {
		if (m_RowIndex[i] != DWORD(-1))	// if valid index entry
			m_RowsDone++;	// count as done
		else	// gap in sequence
			m_RedoStrip.Add(i);	// add to exception list
	}
	m_ResumingJob = TRUE;
	return(TRUE);
}
