// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
		00		03mar09	initial version

		MIDI assignment information
 
*/

#include "stdafx.h"
#include "Resource.h"
#include "MidiInfo.h"

CMidiInfo::CMidiInfo()
{
	INFO&	ip = *this;
	ZeroMemory(&ip, sizeof(INFO));
}

CMidiInfo::CMidiInfo(const CMidiSetupDlg::INFO& Info)
{
	INFO&	ip = *this;
	ip = Info;
}

#define HEADER_FMT	_T("FracticeMIDI\t%d\n")
#define	MIDIROW_FMT	_T("%s\t%lg\t%lg\t%d\t%d\t%d\n")

bool CMidiInfo::Read(CStdioFile& fp)
{
	CString	s;
	fp.ReadString(s);
	int	Version;
	if (_stscanf(s, HEADER_FMT, &Version) != 1 || Version > FILE_VERSION)
		return(FALSE);
	while (fp.ReadString(s)) {
		ROWINFO	ri;
		TCHAR	RowName[256];
		if (_stscanf(s, MIDIROW_FMT, RowName, 
			&ri.Range.Start, &ri.Range.End, &ri.Event, &ri.Chan, &ri.Ctrl) != 6)
			return(FALSE);
		int	RowIdx = CMidiSetupDlg::FindRow(RowName);
		if (RowIdx < 0)	// row name not found
			return(FALSE);
		Row[RowIdx] = ri;
	}
	return(TRUE);
}

void CMidiInfo::Write(CStdioFile& fp)
{
	CString	s;
	s.Format(HEADER_FMT, FILE_VERSION);
	fp.WriteString(s);
	for (int i = 0; i < ROWS; i++) {
		const ROWINFO&	ri = Row[i];
		s.Format(MIDIROW_FMT, CMidiSetupDlg::GetRowName(i),
			ri.Range.Start, ri.Range.End, ri.Event, ri.Chan, ri.Ctrl);
		fp.WriteString(s);
	}
}

bool CMidiInfo::DoIO(LPCTSTR Path, DWORD FileMode)
{
	TRY {
		CStdioFile	fp(Path, FileMode);
		if (FileMode & CFile::modeWrite)
			Write(fp);
		else {
			if (!Read(fp)) {
				CString	msg;
				AfxFormatString1(msg, IDS_BAD_FORMAT, Path);
				AfxMessageBox(msg);
				return(FALSE);
			}
		}
	}
	CATCH(CFileException, e)
	{
		e->ReportError();
		return(FALSE);
	}
	END_CATCH
	return(TRUE);
}

bool CMidiInfo::Read(LPCTSTR Path)
{
	return(DoIO(Path, CFile::modeRead | CFile::shareDenyWrite));
}

bool CMidiInfo::Write(LPCTSTR Path)
{
	return(DoIO(Path, CFile::modeCreate | CFile::modeWrite));
}
