// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		05jan09	add image DIB
		02		10jan09	move GetBounds here
		03		17jan09	add version number
		04		01feb09	in GetRGB, add color offset
		05		03feb09	in EqualIgnoreColor, add color offset
		06		07feb09	add escape time array

        snapshot container
 
*/

#include "stdafx.h"
#include "Snapshot.h"
#include "Dib.h"

CSnapshot::CSnapshot()
{
	m_Origin.x = 0;
	m_Origin.y = 0;
	m_Zoom = 0;
	m_FractalType = 0;
	m_ImageSize = CSize(0, 0);
	m_Quality = 0;
	m_Antialias = 0;
	m_MathPrec = m_Origin.GetPrec();
	m_CycleLen = 0;
	m_DeepZoom = FALSE;
	m_Exponent = 0;
	m_ColorOffset = 0;
	m_ThumbFrame.SetRectEmpty();
	m_ThumbDetail.SetRectEmpty();
}

CSnapshot::CSnapshot(const CSnapshot& Snap) :
	// set math precision during construction to avoid reallocation in Copy
	m_Origin(Snap.m_MathPrec, 0, 0),	// dummy args
	m_Zoom(Snap.m_MathPrec, 0)	// dummy arg
{
	m_MathPrec = Snap.m_MathPrec;	// make sure Copy doesn't set precision again
	Copy(Snap);
}

void CSnapshot::SetMathPrec(UINT MathPrec)
{
	// SetPrec reallocates memory, so only call it if precision changed
	if (MathPrec != m_MathPrec) {
		// set precision of BigNum-derived member vars here
		m_Origin.SetPrec(MathPrec);
		m_Zoom.SetPrec(MathPrec);
		m_MathPrec = MathPrec;
	}
}

void CSnapshot::GetBounds(BigRect& Bounds) const
{
	BigNum	sz(m_MathPrec, GetNominalSize() / 2, 0);
	sz /= m_Zoom;
	BigPoint	Scale(sz * GetFrameAspectBig(), sz);
	Bounds.p1() = m_Origin - Scale;
	Bounds.p2() = m_Origin + Scale;
}

void CSnapshot::Copy(const CSnapshot& Snap)
{
	SetMathPrec(Snap.m_MathPrec);	// order matters; do before setting bignums
	m_Origin		= Snap.m_Origin; 
	m_Zoom			= Snap.m_Zoom;
	m_FractalType	= Snap.m_FractalType;
	m_ImageSize		= Snap.m_ImageSize;
	m_Quality		= Snap.m_Quality;
	m_Antialias		= Snap.m_Antialias;
	m_CycleLen		= Snap.m_CycleLen;
	m_DeepZoom		= Snap.m_DeepZoom;
	m_Exponent		= Snap.m_Exponent;
	m_ColorOffset	= Snap.m_ColorOffset;
	m_Thumb			= Snap.m_Thumb;
	m_ThumbFrame	= Snap.m_ThumbFrame;
	m_ThumbDetail	= Snap.m_ThumbDetail;
	m_Palette		= Snap.m_Palette;
	m_Image			= Snap.m_Image;
	m_EscTimes		= Snap.m_EscTimes;
}

void CSnapshot::SetEmpty()
{
	CSnapshot	def;
	*this = def;
}

bool CSnapshot::operator==(const CSnapshot& Snap) const
{
	return(m_Origin == Snap.m_Origin
		&& m_Zoom == Snap.m_Zoom
		&& m_FractalType == Snap.m_FractalType
		&& m_ImageSize == Snap.m_ImageSize
		&& m_Quality == Snap.m_Quality
		&& m_Antialias == Snap.m_Antialias
		&& m_MathPrec == Snap.m_MathPrec
		&& m_CycleLen == Snap.m_CycleLen
		&& m_ColorOffset == Snap.m_ColorOffset 
		&& m_Palette == Snap.m_Palette
		&& m_DeepZoom == Snap.m_DeepZoom
		&& m_Exponent == Snap.m_Exponent);
	// comparison doesn't include thumbnail, image, or escape times
}

bool CSnapshot::EqualIgnoreColor(const CSnapshot& Snap) const
{
	CSnapshot	ts(Snap);
	// add color-related members here
	ts.m_Palette = m_Palette;
	ts.m_CycleLen = m_CycleLen;
	ts.m_ColorOffset = m_ColorOffset;
	return(ts == *this);
}

void CSnapshot::Serialize(CArchive& ar)
{
	if (ar.IsStoring()) {
		ar << ARCHIVE_VERSION;
		ar << m_FractalType;
		ar << m_ImageSize;
		ar << m_Quality;
		ar << m_Antialias;
		ar << m_MathPrec;
		ar << m_CycleLen;
		StoreBool(ar, m_DeepZoom);
		ar << m_Exponent;
		ar << m_ColorOffset;
		bool	GotThumb = HasThumb();
		StoreBool(ar, GotThumb);
		if (GotThumb) {
			CDib	dib;
			if (!dib.Create(m_Thumb))
				AfxThrowArchiveException(CArchiveException::genericException, ar.m_strFileName);
			dib.Serialize(ar);
		}
		ar << m_ThumbFrame;
		ar << m_ThumbDetail;
	} else {	// loading
		UINT	Version;
		ar >> Version;
		if (Version > ARCHIVE_VERSION)
			AfxThrowArchiveException(CArchiveException::badIndex, ar.m_strFileName);
		ar >> m_FractalType;
		ar >> m_ImageSize;
		ar >> m_Quality;
		ar >> m_Antialias;
		int	prec;
		ar >> prec;
		SetMathPrec(prec);	// order matters; do before setting bignums
		ar >> m_CycleLen;
		LoadBool(ar, m_DeepZoom);
		ar >> m_Exponent;
		ar >> m_ColorOffset;
		bool	GotThumb;
		LoadBool(ar, GotThumb);
		if (GotThumb) {
			m_Thumb.CreateObj();
			CDib	dib;
			dib.Serialize(ar);
			if (!dib.CreateBitmap(m_Thumb))
				AfxThrowArchiveException(CArchiveException::genericException, ar.m_strFileName);
		}
		ar >> m_ThumbFrame;
		ar >> m_ThumbDetail;
	}
	m_Origin.Serialize(ar);
	m_Zoom.Serialize(ar);
	m_Palette.Serialize(ar);
	m_Image.Serialize(ar);
	m_EscTimes.Serialize(ar);
}

#define FMT_VERSION			_T("Fractice\t%d\n")
#define FMT_MATH_PREC		_T("MathPrec\t%d\n")
#define FMT_FRACTAL_TYPE	_T("FracType\t%d\n")
#define FMT_IMAGE_WIDTH		_T("ImgWidth\t%d\n")
#define FMT_IMAGE_HEIGHT	_T("ImgHeight\t%d\n")
#define FMT_QUALITY			_T("Quality\t%d\n")
#define FMT_ANTIALIAS		_T("Antialias\t%d\n")
#define FMT_CYCLE_LEN		_T("CycleLen\t%d\n")
#define FMT_COLOR_OFFSET	_T("ColorOfs\t%lg\n")
#define FMT_DEEP_ZOOM		_T("DeepZoom\t%d\n")
#define FMT_EXPONENT		_T("Exponent\t%d\n")

bool CSnapshot::Write(CStdioFile& fp)
{
	CString	s;
	s.Format(FMT_VERSION, FILE_VERSION);
	fp.WriteString(s);
	s.Format(FMT_MATH_PREC, m_MathPrec);
	fp.WriteString(s);
	s.Format(_T("X\t%s\n"), m_Origin.x.ToStr());
	fp.WriteString(s);
	s.Format(_T("Y\t%s\n"), m_Origin.y.ToStr());
	fp.WriteString(s);
	s.Format(_T("Z\t%s\n"), m_Zoom.ToStr());
	fp.WriteString(s);
	s.Format(FMT_FRACTAL_TYPE, m_FractalType);
	fp.WriteString(s);
	s.Format(FMT_IMAGE_WIDTH, m_ImageSize.cx);
	fp.WriteString(s);
	s.Format(FMT_IMAGE_HEIGHT, m_ImageSize.cy);
	fp.WriteString(s);
	s.Format(FMT_QUALITY, m_Quality);
	fp.WriteString(s);
	s.Format(FMT_ANTIALIAS, m_Antialias);
	fp.WriteString(s);
	s.Format(FMT_CYCLE_LEN, m_CycleLen);
	fp.WriteString(s);
	s.Format(FMT_COLOR_OFFSET, m_ColorOffset);
	fp.WriteString(s);
	s.Format(FMT_DEEP_ZOOM, m_DeepZoom);
	fp.WriteString(s);
	s.Format(FMT_EXPONENT, m_Exponent);
	fp.WriteString(s);
	return(m_Palette.Write(fp));
}

bool CSnapshot::Read(CStdioFile& fp)
{
	CString	s;
	fp.ReadString(s);
	int	Version;
	if (_stscanf(s, FMT_VERSION, &Version) != 1 || Version > FILE_VERSION)
		return(FALSE);
	fp.ReadString(s);
	int	prec;
	if (_stscanf(s, FMT_MATH_PREC, &prec) != 1)
		return(FALSE);
	SetMathPrec(prec);	// order matters; do before setting bignums
	fp.ReadString(s);
	if (!m_Origin.x.FromStr(_T("X %Ff"), s))
		return(FALSE);
	fp.ReadString(s);
	if (!m_Origin.y.FromStr(_T("Y %Ff"), s))
		return(FALSE);
	fp.ReadString(s);
	if (!m_Zoom.FromStr(_T("Z %Ff"), s))
		return(FALSE);
	fp.ReadString(s);
	if (_stscanf(s, FMT_FRACTAL_TYPE, &m_FractalType) != 1)
		return(FALSE);
	fp.ReadString(s);
	if (_stscanf(s, FMT_IMAGE_WIDTH, &m_ImageSize.cx) != 1)
		return(FALSE);
	fp.ReadString(s);
	if (_stscanf(s, FMT_IMAGE_HEIGHT, &m_ImageSize.cy) != 1)
		return(FALSE);
	fp.ReadString(s);
	if (_stscanf(s, FMT_QUALITY, &m_Quality) != 1)
		return(FALSE);
	fp.ReadString(s);
	if (_stscanf(s, FMT_ANTIALIAS, &m_Antialias) != 1)
		return(FALSE);
	fp.ReadString(s);
	if (_stscanf(s, FMT_CYCLE_LEN, &m_CycleLen) != 1)
		return(FALSE);
	fp.ReadString(s);
	if (_stscanf(s, FMT_COLOR_OFFSET, &m_ColorOffset) != 1)
		return(FALSE);
	fp.ReadString(s);
	int	DeepZoom;
	if (_stscanf(s, FMT_DEEP_ZOOM, &DeepZoom) != 1)
		return(FALSE);
	m_DeepZoom = DeepZoom != 0;
	fp.ReadString(s);
	int	Exponent;
	if (_stscanf(s, FMT_EXPONENT, &Exponent) != 1)
		return(FALSE);
	m_Exponent = BYTE(Exponent);
	return(m_Palette.Read(fp));
}

bool CSnapshot::Write(LPCTSTR Path)
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

bool CSnapshot::Read(LPCTSTR Path)
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
