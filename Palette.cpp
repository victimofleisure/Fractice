// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		01feb09	in GetRGB, add color offset

        palette container
 
*/

#include "stdafx.h"
#include "Resource.h"
#include "Palette.h"
#include <math.h>

IMPLEMENT_SERIAL(DPalette, CGradient, 1)

DPalette::DPalette()
{
	m_BkColor = 0;
	m_CurSel = -1;
	m_Modified = FALSE;
}

void DPalette::Copy(const DPalette& Palette)
{
	CGradient::Copy(Palette);
	m_BkColor = Palette.m_BkColor;
	m_Name = Palette.m_Name;
	m_CurSel = Palette.m_CurSel;
	m_Modified = Palette.m_Modified;
}

bool DPalette::operator==(const DPalette& Palette) const
{
	const CGradient& Gradient = *this;
	return(Gradient == Palette	// base class compare
		&& Palette.m_BkColor == m_BkColor
		&& Palette.m_Name == m_Name);
}

void DPalette::Serialize(CArchive& ar)
{
	CGradient::Serialize(ar);
	if (ar.IsStoring()) {
		ar << m_BkColor;
		ar << m_Name;
	} else {
		ar >> m_BkColor;
		ar >> m_Name;
	}
}

#define PAL_HEADER	_T("Palette\t%d\t%d\t%d\t%x\n")
#define PAL_GRAD_PT_W	_T("%.15g\t%.15g\t%.15g\t%.15g\n")
#define PAL_GRAD_PT_R	_T("%lf\t%lf\t%lf\t%lf\n")

bool DPalette::Write(CStdioFile& fp)
{
	int	pts = GetSize();
	CString	s;
	s.Format(PAL_HEADER, pts, m_ColorMode, m_SpaceEvenly, m_BkColor);
	fp.WriteString(s);
	for (int i = 0; i < pts; i++) {
		const CGradientPt&	pt = (*this)[i]; 
		const DRGB_S&	c = pt.m_Color.rgb;
		s.Format(PAL_GRAD_PT_W, pt.m_Pos, c.m_r, c.m_g, c.m_b);
		fp.WriteString(s);
	}
	fp.WriteString(m_Name + "\n");
	return(TRUE);
}

bool DPalette::Read(CStdioFile& fp)
{
	int	pts = GetSize();
	CString	s;
	fp.ReadString(s);
	if (_stscanf(s, PAL_HEADER, &pts, 
	&m_ColorMode, &m_SpaceEvenly, &m_BkColor) != 4)
		return(FALSE);
	SetSize(pts);
	for (int i = 0; i < pts; i++) {
		fp.ReadString(s);
		CGradientPt&	pt = (*this)[i]; 
		DRGB_S&	c = pt.m_Color.rgb;
		if (_stscanf(s, PAL_GRAD_PT_R, &pt.m_Pos, &c.m_r, &c.m_g, &c.m_b) != 4)
			return(FALSE);
	}
	fp.ReadString(m_Name);
	return(TRUE);
}

bool DPalette::Write(LPCTSTR Path)
{
	TRY {
		CStdioFile	fp(Path, CFile::modeCreate | CFile::modeWrite);
		if (!Write(fp))
			AfxThrowArchiveException(CArchiveException::badIndex, Path);
	}
	CATCH(CFileException, e)
	{
		e->ReportError();
		return(FALSE);
	}
	CATCH(CArchiveException, e)
	{
		e->ReportError();
		return(FALSE);
	}
	END_CATCH
	return(TRUE);
}

bool DPalette::Read(LPCTSTR Path)
{
	TRY {
		CStdioFile	fp(Path, CFile::modeRead | CFile::shareDenyWrite);
		if (!Read(fp))
			AfxThrowArchiveException(CArchiveException::badIndex, Path);
	}
	CATCH(CFileException, e)
	{
		e->ReportError();
		return(FALSE);
	}
	CATCH(CArchiveException, e)
	{
		e->ReportError();
		return(FALSE);
	}
	END_CATCH
	return(TRUE);
}

bool DPalette::IsNativeFormat(LPCTSTR Path)
{
	TRY {
		CStdioFile	fp(Path, CFile::modeRead | CFile::shareDenyWrite);
		CString	s;
		fp.ReadString(s);
		int	pts, ColorMode, SpaceEvenly, BkColor;
		if (_stscanf(s, PAL_HEADER, &pts,
		&ColorMode, &SpaceEvenly, &BkColor) != 4)	// if we can't read header
			return(FALSE);	// non-native format
	}
	CATCH(CFileException, e)
	{
		return(TRUE);	// format unknown, presumed native
	}
	END_CATCH
	return(TRUE);	// native format
}

void DPalette::GetRGB(COLORREF *Color, UINT Count, int CycleLen, double Offset) const
{
	int	sz = Count;
	DRGB	c;
	CGradientIter	iter(*this);
	int	clen = CycleLen > 0 ? min(CycleLen, sz) : sz;
	int	i;
	double	ofs = Offset * clen;
	for (i = 0; i < clen; i++) {
		double	pos = fmod(i + ofs, clen) / clen;
		iter.Interpolate(pos, c);
		Color[i] = c;
	}
	if (CycleLen > 0) {
		for (; i < sz; i++) {
			int	pos = i % CycleLen;
			Color[i] = Color[pos];
		}
	}
}

bool DPalette::Import(CStdioFile& fp, UINT Flags)
{	
	// this function throws CFileException
	DPalette	pal;	// start with defaults
	CString	s;
	while (fp.ReadString(s)) {
		int	r, g, b;
		if (_stscanf(s, _T("%d %d %d\n"), &r, &g, &b) != 3)
			return(FALSE);
		COLORREF	c = RGB(r, g, b);
		CGradientPt	pt(0, c);	// all position zero for now
		pal.Add(pt);
	}
	if (Flags & IEF_FIRST_BKGND) {	// if first entry is background color
		if (!pal.GetSize())
			return(FALSE);
		DRGB	rgb(pal[0].m_Color.rgb);	
		pal.m_BkColor = rgb;
		pal.RemoveAt(0);
	}
	pal.SpaceEvenly((Flags & IEF_SMOOTH_WRAP) != 0);
	*this = pal;
	return(TRUE);
}

bool DPalette::Export(CStdioFile& fp, UINT Entries, UINT Flags) const
{
	// this function throws CFileException
	DPalette	pal;	// start with defaults
	if (Flags & IEF_FIRST_BKGND) {	// if first entry is background color
		CString	s;
		s.Format(_T("%d %d %d\n"), GetRValue(m_BkColor), 
			GetGValue(m_BkColor), GetBValue(m_BkColor));
		fp.WriteString(s);
		Entries--;
	}
	CGradientIter	iter(*this);
	int	WrapOfs = (Flags & IEF_SMOOTH_WRAP) ? 0 : 1;
	if (WrapOfs && Entries <= 1)
		return(FALSE);	// avoid divide by zero
	for (UINT i = 0; i < Entries; i++) {
		double	pos = double(i) / (Entries - WrapOfs);
		DRGB	rgb;
		iter.Interpolate(pos, rgb);
		COLORREF	c = rgb;
		CString	s;
		s.Format(_T("%d %d %d\n"), GetRValue(c), GetGValue(c), GetBValue(c));
		fp.WriteString(s);
	}
	return(TRUE);
}

bool DPalette::Import(LPCTSTR Path, UINT Flags)
{
	TRY {
		CStdioFile	fp(Path, CFile::modeRead | CFile::shareDenyWrite);
		if (!Import(fp, Flags)) {
			AfxMessageBox(IDS_CANT_IMPORT_PALETTE);
			return(FALSE);
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

bool DPalette::Export(LPCTSTR Path, UINT Entries, UINT Flags) const
{
	TRY {
		CStdioFile	fp(Path, CFile::modeCreate | CFile::modeWrite);
		if (!Export(fp, Entries, Flags)) {
			AfxMessageBox(IDS_CANT_EXPORT_PALETTE);
			return(FALSE);
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
