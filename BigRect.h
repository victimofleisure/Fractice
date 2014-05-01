// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
        01      04dec09	add ctors for double and int
        
		extended-precision rectangle
 
*/

#ifndef BIGRECT_INCLUDED
#define BIGRECT_INCLUDED

#include "BigPoint.h"

class BigRect : public WObject {
public:
// Construction
	BigRect();
	BigRect(UINT Prec, int, int);	// dummy args
	BigRect(const BigRect& r);
	BigRect(const BigPoint& p1, const BigPoint& p2);
	BigRect(const BigNum& x1, const BigNum& y1, const BigNum& x2, const BigNum& y2);
	BigRect(double x1, double y1, double x2, double y2);
	BigRect(UINT x1, UINT y1, UINT x2, UINT y2);
	BigRect(int x1, int y1, int x2, int y2);
	BigRect(UINT Prec, double x1, double y1, double x2, double y2);
	BigRect(UINT Prec, UINT x1, UINT y1, UINT x2, UINT y2);
	BigRect(UINT Prec, int x1, int y1, int x2, int y2);

// Assignment
	BigRect& operator=(const BigRect& r);

// Attributes
	UINT	GetPrec() const;
	void	SetPrec(UINT Bits);
	double	Height() const;
	double	Width() const;
	const BigPoint Size() const;
	BigPoint& p1();
	const BigPoint& p1() const;
	BigPoint& p2();
	const BigPoint& p2() const;

// Comparison
	bool operator==(const BigRect& n) const;
	bool operator!=(const BigRect& n) const;

// Arithmetic
	const BigRect operator+(const BigPoint& p) const;
	const BigRect operator-(const BigPoint& p) const;
	BigRect& operator+=(const BigPoint& p);
	BigRect& operator-=(const BigPoint& p);

// Conversion
	void	Serialize(CArchive& ar);

// Data members
	BigNum	x1;
	BigNum	y1;
	BigNum	x2;
	BigNum	y2;
};

inline BigRect::BigRect()
{
}

inline BigRect::BigRect(UINT Prec, int, int) :
	x1(Prec, 0), y1(Prec, 0), x2(Prec, 0), y2(Prec, 0)	// dummy args
{
}

inline BigRect::BigRect(const BigRect& r) :
	x1(r.x1), y1(r.y1), x2(r.x2), y2(r.y2)
{
}

inline BigRect::BigRect(const BigPoint& p1, const BigPoint& p2) :
	x1(p1.x), y1(p1.y), x2(p2.x), y2(p2.y)
{
}

inline BigRect::BigRect(const BigNum& x1, const BigNum& y1, const BigNum& x2, const BigNum& y2) :
	x1(x1), y1(y1), x2(x2), y2(y2)
{
}

inline BigRect::BigRect(double x1, double y1, double x2, double y2) :
	x1(x1), y1(y1), x2(x2), y2(y2)
{
}

inline BigRect::BigRect(UINT x1, UINT y1, UINT x2, UINT y2) :
	x1(x1), y1(y1), x2(x2), y2(y2)
{
}

inline BigRect::BigRect(int x1, int y1, int x2, int y2) :
	x1(x1), y1(y1), x2(x2), y2(y2)
{
}

inline BigRect::BigRect(UINT Prec, double x1, double y1, double x2, double y2) :
	x1(Prec, x1, 0), y1(Prec, y1, 0), x2(Prec, x2, 0), y2(Prec, y2, 0)
{
}

inline BigRect::BigRect(UINT Prec, UINT x1, UINT y1, UINT x2, UINT y2) :
	x1(Prec, x1, 0), y1(Prec, y1, 0), x2(Prec, x2, 0), y2(Prec, y2, 0)
{
}

inline BigRect::BigRect(UINT Prec, int x1, int y1, int x2, int y2) :
	x1(Prec, x1, 0), y1(Prec, y1, 0), x2(Prec, x2, 0), y2(Prec, y2, 0)
{
}

inline BigRect& BigRect::operator=(const BigRect& r)
{
	x1 = r.x1; 
	y1 = r.y1;
	x2 = r.x2;
	y2 = r.y2;
	return(*this);
}

inline UINT	BigRect::GetPrec() const
{
	return(p1().GetPrec());
}

inline void BigRect::SetPrec(UINT Prec)
{
	p1().SetPrec(Prec);
	p2().SetPrec(Prec);
}

inline double BigRect::Height() const
{
	return(y2 - y1);
}

inline double BigRect::Width() const
{
	return(x2 - x1);
}

inline const BigPoint BigRect::Size() const
{
	return(BigPoint(x2 - x1, y2 - y1));
}

inline BigPoint& BigRect::p1()
{
	return(*((BigPoint *)this));
}

inline const BigPoint& BigRect::p1() const
{
	return(*((BigPoint *)this));
}

inline BigPoint& BigRect::p2()
{
	return(*((BigPoint *)this + 1));
}

inline const BigPoint& BigRect::p2() const
{
	return(*((BigPoint *)this + 1));
}

inline bool BigRect::operator==(const BigRect& r) const
{
	return(p1() == r.p1() && p2() == r.p2());
} 

inline bool BigRect::operator!=(const BigRect& r) const
{
	return(!(*this == r));
} 

inline const BigRect BigRect::operator+(const BigPoint& p) const
{
	return(BigRect(p1() + p, p2() + p));
}

inline const BigRect BigRect::operator-(const BigPoint& p) const
{
	return(BigRect(p1() - p, p2() - p));
}

inline BigRect& BigRect::operator+=(const BigPoint& p)
{
	return(*this = *this + p);
}

inline BigRect& BigRect::operator-=(const BigPoint& p)
{
	return(*this = *this - p);
}

#endif
