// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*	
		chris korda

		revision history:
		rev		date	comments
        00      14feb08	initial version

		container for options information

*/

#include "stdafx.h"
#include "OptionsInfo.h"

void COptionsInfo::Copy(const COptionsInfo& Info)
{
	SetBaseInfo(Info);
	m_DefSnapshot	= Info.m_DefSnapshot;
}

void COptionsInfo::Serialize(CArchive& ar)
{
	if (ar.IsStoring()) {
		ar << ARCHIVE_VERSION;
		ar << sizeof(BASE_OPTIONS_INFO);
		BASE_OPTIONS_INFO	*pboi = this;
		ar.Write(pboi, sizeof(BASE_OPTIONS_INFO));
		ar << m_DefSnapshot;
	} else {
		int	Version;
		ar >> Version;
		if (Version > ARCHIVE_VERSION)
			AfxThrowArchiveException(CArchiveException::badIndex, ar.m_strFileName);
		int	BaseInfoSize;
		ar >> BaseInfoSize;
		BASE_OPTIONS_INFO	*pboi = this;
		ar.Read(pboi, BaseInfoSize);
		ar >> m_DefSnapshot;
	}
}
