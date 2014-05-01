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
        
		extended-precision point
 
*/

#ifndef BIGPOINT_INCLUDED
#define BIGPOINT_INCLUDED

#include "BigNum.h"

class BigPoint : public WObject {
public:
// Construction
	BigPoint();
	BigPoint(UINT Prec, int, int);	// dummy args
	BigPoint(const BigPoint& p);
	BigPoint(const BigNum& x, const BigNum& y);
	BigPoint(double x, double y);
	BigPoint(UINT x, UINT y);
	BigPoint(int x, int y);
	BigPoint(UINT Prec, double x, double y, int);
	BigPoint(UINT Prec, UINT x, UINT y, int);
	BigPoint(UINT Prec, int x, int y, int);

// Assignment
	BigPoint& operator=(const BigPoint& p);

// Attributes
	UINT	GetPrec() const;
	void	SetPrec(UINT Bits);

// Comparison
	bool operator==(const BigPoint& n) const;
	bool operator!=(const BigPoint& n) const;

// Arithmetic
	const BigPoint operator+(const BigPoint& p) const;
	const BigPoint operator-(const BigPoint& p) const;
	const BigPoint operator*(const BigPoint& p) const;
	const BigPoint operator/(const BigPoint& p) const;
	const BigPoint operator+(const BigNum& n) const;
	const BigPoint operator-(const BigNum& n) const;
	const BigPoint operator*(const BigNum& n) const;
	const BigPoint operator/(const BigNum& n) const;
	BigPoint& operator+=(const BigPoint& p);
	BigPoint& operator-=(const BigPoint& p);
	BigPoint& operator*=(const BigPoint& p);
	BigPoint& operator/=(const BigPoint& p);
	BigPoint& operator+=(const BigNum& n);
	BigPoint& operator-=(const BigNum& n);
	BigPoint& operator*=(const BigNum& n);
	BigPoint& operator/=(const BigNum& n);

// Conversion
	void	Serialize(CArchive& ar);

// Data members
	BigNum	x;
	BigNum	y;
};

inline BigPoint::BigPoint()
{
}

inline BigPoint::BigPoint(UINT Prec, int, int) :
	x(Prec, 0), y(Prec, 0)	// dummy args
{
}

inline BigPoint::BigPoint(const BigPoint& p) :
	x(p.x), y(p.y)
{ 
}

inline BigPoint::BigPoint(const BigNum& x, const BigNum& y) :
	x(x), y(y)
{
}

inline BigPoint::BigPoint(double x, double y) :
	x(x), y(y)
{
}

inline BigPoint::BigPoint(UINT x, UINT y) :
	x(x), y(y)
{
}

inline BigPoint::BigPoint(int x, int y) :
	x(x), y(y)
{
}

inline BigPoint::BigPoint(UINT Prec, double x, double y, int) :
	x(Prec, x, 0), y(Prec, y, 0)
{
}

inline BigPoint::BigPoint(UINT Prec, UINT x, UINT y, int) :
	x(Prec, x, 0), y(Prec, y, 0)
{
}

inline BigPoint::BigPoint(UINT Prec, int x, int y, int) :
	x(Prec, x, 0), y(Prec, y, 0)
{
}

inline BigPoint& BigPoint::operator=(const BigPoint& p)
{
	x = p.x; 
	y = p.y;
	return(*this);
}

inline UINT	BigPoint::GetPrec() const
{
	return(x.GetPrec());
}

inline void BigPoint::SetPrec(UINT Prec)
{
	x.SetPrec(Prec);
	y.SetPrec(Prec);
}

inline bool BigPoint::operator==(const BigPoint& p) const
{
	return(x == p.x && y == p.y);
} 

inline bool BigPoint::operator!=(const BigPoint& p) const
{
	return(!(*this == p));
} 

const inline BigPoint BigPoint::operator+(const BigPoint& p) const
{
	return(BigPoint(x + p.x, y + p.y));
}

const inline BigPoint BigPoint::operator-(const BigPoint& p) const
{
	return(BigPoint(x - p.x, y - p.y));
}

const inline BigPoint BigPoint::operator*(const BigPoint& p) const
{
	return(BigPoint(x * p.x, y * p.y));
}

const inline BigPoint BigPoint::operator/(const BigPoint& p) const
{
	return(BigPoint(x / p.x, y / p.y));
}

const inline BigPoint BigPoint::operator+(const BigNum& n) const
{
	return(*this + BigPoint(n, n));
}

const inline BigPoint BigPoint::operator-(const BigNum& n) const
{
	return(*this - BigPoint(n, n));
}

const inline BigPoint BigPoint::operator*(const BigNum& n) const
{
	return(*this * BigPoint(n, n));
}

const inline BigPoint BigPoint::operator/(const BigNum& n) const
{
	return(*this / BigPoint(n, n));
}

inline BigPoint& BigPoint::operator+=(const BigPoint& p)
{
	return(*this = *this + p);
}

inline BigPoint& BigPoint::operator-=(const BigPoint& p)
{
	return(*this = *this - p);
}

inline BigPoint& BigPoint::operator*=(const BigPoint& p)
{
	return(*this = *this * p);
}

inline BigPoint& BigPoint::operator/=(const BigPoint& p)
{
	return(*this = *this / p);
}

inline BigPoint& BigPoint::operator+=(const BigNum& n)
{
	return(*this = *this + n);
}

inline BigPoint& BigPoint::operator-=(const BigNum& n)
{
	return(*this = *this - n);
}

inline BigPoint& BigPoint::operator*=(const BigNum& n)
{
	return(*this = *this * n);
}

inline BigPoint& BigPoint::operator/=(const BigNum& n)
{
	return(*this = *this / n);
}

#endif
