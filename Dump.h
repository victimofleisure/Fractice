// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version

        diagnostic output functions
 
*/

#ifndef CDUMP_INCLUDED
#define CDUMP_INCLUDED

#include "DRect.h"
#include "DColor.h"
#include "BigRect.h"

#define dmp(x) { _tprintf(_T(#x)_T(" = %s\n"), CDump::ToStr(x)); }
#define dmp2(x, Prec) { _tprintf(_T(#x)_T(" = %s\n"), CDump::ToStr(x, Prec)); }

class CDump {
public:
	enum {
		DEF_PREC = 6
	};
	static CString ToStr(const POINT& pt);
	static CString ToStr(const SIZE& sz);
	static CString ToStr(const RECT& r);
	static CString ToStr(const DPOINT& pt, int Prec = DEF_PREC);
	static CString ToStr(const DRECT& r, int Prec = DEF_PREC);
	static CString ToStr(const DRGB& c, int Prec = DEF_PREC);
	static CString ToStr(const DHLS& c, int Prec = DEF_PREC);
	static CString ToStr(const BigNum& n, int Prec = DEF_PREC);
	static CString ToStr(const BigPoint& pt, int Prec = DEF_PREC);
	static CString ToStr(const BigRect& r, int Prec = DEF_PREC);
};

inline CString CDump::ToStr(const POINT& pt)
{
	CString	s;
	s.Format(_T("%d, %d"), pt.x, pt.y);
	return(s);
}

inline CString CDump::ToStr(const SIZE& sz)
{
	CString	s;
	s.Format(_T("%d, %d"), sz.cx, sz.cy);
	return(s);
}

inline CString CDump::ToStr(const RECT& r)
{
	CString	s;
	s.Format(_T("%d, %d, %d, %d"), r.left, r.top, r.right, r.bottom);
	return(s);
}

inline CString CDump::ToStr(const DPOINT& pt, int Prec)
{
	CString	s;
	s.Format(_T("%.*f, %.*f"), Prec, pt.x, Prec, pt.y);
	return(s);
}

inline CString CDump::ToStr(const DRECT& r, int Prec)
{
	CString	s;
	s.Format(_T("%.*f, %.*f, %.*f, %.*f"), Prec, r.x1, Prec, r.y1, Prec, r.x2, Prec, r.y2);
	return(s);
}

inline CString CDump::ToStr(const DRGB& c, int Prec)
{
	CString	s;
	s.Format(_T("%.*f, %.*f, %.*f"), Prec, c.m_r, Prec, c.m_g, Prec, c.m_b);
	return(s);
}

inline CString CDump::ToStr(const DHLS& c, int Prec)
{
	CString	s;
	s.Format(_T("%.*f, %.*f, %.*f"), Prec, c.m_h, Prec, c.m_l, Prec, c.m_s);
	return(s);
}

inline CString CDump::ToStr(const BigNum& n, int Prec)
{
	return(n.ToStr(Prec));
}

inline CString CDump::ToStr(const BigPoint& pt, int Prec)
{
	CString	s;
	s.Format(_T("%s, %s"), pt.x.ToStr(Prec), pt.y.ToStr(Prec));
	return(s);
}

inline CString CDump::ToStr(const BigRect& r, int Prec)
{
	CString	s;
	s.Format(_T("%s, %s, %s, %s"), 
		r.x1.ToStr(Prec), r.y1.ToStr(Prec),
		r.x2.ToStr(Prec), r.y2.ToStr(Prec));
	return(s);
}

#endif
