// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
        01      09jan10	add version support
        
		extended-precision number
 
*/

#include "stdafx.h"
#include "BigNum.h"
#ifdef UNICODE
#include <atlconv.h>	// ATL string conversion macros
#endif

const LPCTSTR BigNum::m_OSName[OS_TYPES] = {
	_T("Win32"),
	_T("Win64"),
};

const LPCTSTR BigNum::m_LibName[LIB_TYPES] = {
	_T("GMP"),
	_T("MPIR"),
};

const LPCTSTR BigNum::m_CPUName[CPU_TYPES] = {
	_T("Pentium4"),
	_T("Core2"),
};

void BigNum::Format(LPCTSTR Fmt, int Precision, CString& Dst) const
{
	int	MaxLen = Precision + 16;
#ifdef UNICODE
	USES_CONVERSION;
	char	*p = new char[MaxLen];
	gmp_snprintf(p, MaxLen, W2CA(Fmt), Precision, f);
	Dst = p;
	delete p;
#else
	LPTSTR	p = Dst.GetBuffer(MaxLen);
	gmp_snprintf(p, MaxLen, Fmt, Precision, f);
	Dst.ReleaseBuffer();
#endif
}

void BigNum::Format(int Precision, CString& Dst) const
{
	Format(_T("%.*FG"), Precision, Dst);
}

CString BigNum::ToStr(int Precision) const
{
	CString	s;
	Format(Precision, s);
	return(s);
}

CString BigNum::ToStr() const
{
	long	exp;
	char	*p = mpf_get_str(
		NULL,	// GMP allocates string
		&exp,	// receives exponent
		10,		// base 10
		0,		// get all significant digits
		f		// source number
	);
	CString	s(p);
	free(p);
	if (s.IsEmpty())
		s = _T("0");
	else {
		int	idecp = (s[0] == '-') + 1;	// index of decimal point
		if (idecp < s.GetLength())
			s.Insert(idecp, '.');
		exp--;	// we moved decimal point right one, so adjust exponent
	}
	CString	expstr;
	expstr.Format(_T("E%+d"), exp);	// prefix value with a sign
	s += expstr;	// append exponent
	return(s);
}

bool BigNum::FromStr(LPCTSTR Src)
{
#ifdef UNICODE
	USES_CONVERSION;
	return(gmp_sscanf(W2CA(Src), "%Ff", f) == 1);
#else
	return(gmp_sscanf(Src, "%Ff", f) == 1);
#endif
}

bool BigNum::FromStr(LPCTSTR Fmt, LPCTSTR Src)
{
#ifdef UNICODE
	USES_CONVERSION;
	return(gmp_sscanf(W2CA(Src), W2CA(Fmt), f) == 1);
#else
	return(gmp_sscanf(Src, Fmt, f) == 1);
#endif
}

void BigNum::Serialize(CArchive& ar)
{
	if (ar.IsStoring()) {
		ar << ToStr();
	} else {
		CString	s;
		ar >> s;
		FromStr(s);
	}
}
