// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*	
		chris korda

		revision history:
		rev		date	comments
        00      28feb09	initial version
        01      09mar09	add channel array
		02		30mar09	add Read/Write and doc paths

		container for mixer information

*/

#include "stdafx.h"
#include "MixerInfo.h"

void CMixerInfo::Copy(const CMixerInfo& Info)
{
	SetBaseInfo(Info);
	m_DocPath.Copy(Info.m_DocPath);
}

void CMixerInfo::Serialize(CArchive& ar)
{
	if (ar.IsStoring()) {
		ar << ARCHIVE_SIG;
		ar << ARCHIVE_VERSION;
		ar << sizeof(MIXER_MISC_INFO);
		const MIXER_MISC_INFO *pmmi = this;
		ar.Write(pmmi, sizeof(MIXER_MISC_INFO));
		ar << MIXER_CHANS;
		ar << sizeof(MIXER_CHAN_INFO);
		for (int i = 0; i < MIXER_CHANS; i++)
			ar.Write(&m_Chan[i], sizeof(MIXER_CHAN_INFO));
	} else {
		int	Sig, Version;
		ar >> Sig;
		ar >> Version;
		if (Sig != ARCHIVE_SIG || Version > ARCHIVE_VERSION)
			AfxThrowArchiveException(CArchiveException::badIndex, ar.m_strFileName);
		int	MiscInfoSize;
		ar >> MiscInfoSize;
		if (MiscInfoSize > sizeof(MIXER_MISC_INFO))
			AfxThrowArchiveException(CArchiveException::badIndex, ar.m_strFileName);
		MIXER_MISC_INFO	*pmmi = this;
		ar.Read(pmmi, MiscInfoSize);
		int	MixerChans, ChanInfoSize;
		ar >> MixerChans;
		ar >> ChanInfoSize;
		if (ChanInfoSize > sizeof(MIXER_CHAN_INFO))
			AfxThrowArchiveException(CArchiveException::badIndex, ar.m_strFileName);
		for (int i = 0; i < MIXER_CHANS; i++)
			ar.Read(&m_Chan[i], ChanInfoSize);
	}
	m_DocPath.Serialize(ar);
}

bool CMixerInfo::DoIO(LPCTSTR Path, DWORD FileMode, DWORD ArchiveMode)
{
	CFile	fp;
	CFileException	e;
	if (!fp.Open(Path, FileMode, &e)) {
		e.ReportError();
		return(FALSE);
	}
	TRY {
		CArchive	ar(&fp, ArchiveMode);
		ar.m_strFileName = fp.GetFileName();
		Serialize(ar);
	}
	CATCH(CArchiveException, e)
	{
		e->ReportError();
		return(FALSE);
	}
	CATCH(CFileException, e)
	{
		e->ReportError();
		return(FALSE);
	}
	END_CATCH
	return(TRUE);
}

bool CMixerInfo::Write(LPCTSTR Path)
{
	return(DoIO(Path, CFile::modeCreate | CFile::modeWrite, CArchive::store));
}

bool CMixerInfo::Read(LPCTSTR Path)
{
	return(DoIO(Path, CFile::modeRead | CFile::shareDenyWrite, CArchive::load));
}

