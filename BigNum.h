// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
        01      06jan10	W64: in 64-bit version, replace GMP with MPIR 
        02      09jan10	add version support
        
		extended-precision number
 
*/

#ifndef BIGNUM_INCLUDED
#define BIGNUM_INCLUDED

#ifdef _WIN64
#pragma warning(push, 3)
#pragma warning(disable : 4244)
#include "mpir.h"
#pragma warning(pop)
#else
#pragma warning(push, 3)
#pragma warning(disable : 4146)
#include "gmp.h"
#pragma warning(pop)
#endif

class BigNum : public WObject {
public:
// Construction
	BigNum();
	BigNum(UINT Prec, int);	// dummy arg
	BigNum(const BigNum& n);
	BigNum(double d);
	BigNum(UINT u);
	BigNum(int i);
	BigNum(UINT Prec, double d, int);
	BigNum(UINT Prec, UINT ui, int);
	BigNum(UINT Prec, int i, int);
	~BigNum();

// Casting
	operator double() const;
	operator __mpf_struct*();
	operator const __mpf_struct*() const;

// Assignment
	BigNum& operator=(const BigNum& n);

// Attributes
	UINT	GetPrec() const;
	void	SetPrec(UINT Prec);

// Comparison
	// native
	bool operator==(const BigNum& n) const;
	bool operator!=(const BigNum& n) const;
	bool operator>(const BigNum& n) const;
	bool operator<(const BigNum& n) const;
	bool operator>=(const BigNum& n) const;
	bool operator<=(const BigNum& n) const;
	// double
	bool operator==(double d) const;
	bool operator!=(double d) const;
	bool operator>(double d) const;
	bool operator<(double d) const;
	bool operator>=(double d) const;
	bool operator<=(double d) const;
	// unsigned int
	bool operator==(UINT u) const;
	bool operator!=(UINT u) const;
	bool operator>(UINT u) const;
	bool operator<(UINT u) const;
	bool operator>=(UINT u) const;
	bool operator<=(UINT u) const;
	// signed int
	bool operator==(int i) const;
	bool operator!=(int i) const;
	bool operator>(int i) const;
	bool operator<(int i) const;
	bool operator>=(int i) const;
	bool operator<=(int i) const;

// Arithmetic
	// native
	const BigNum operator+(const BigNum& n) const;
	const BigNum operator-(const BigNum& n) const;
	const BigNum operator*(const BigNum& n) const;
	const BigNum operator/(const BigNum& n) const;
	BigNum& operator+=(const BigNum& n);
	BigNum& operator-=(const BigNum& n);
	BigNum& operator*=(const BigNum& n);
	BigNum& operator/=(const BigNum& n);
	// double
	const BigNum operator+(double d) const;
	const BigNum operator-(double d) const;
	const BigNum operator*(double d) const;
	const BigNum operator/(double d) const;
	BigNum& operator+=(double d);
	BigNum& operator-=(double d);
	BigNum& operator*=(double d);
	BigNum& operator/=(double d);
	// unsigned int
	const BigNum operator+(UINT u) const;
	const BigNum operator-(UINT u) const;
	const BigNum operator*(UINT u) const;
	const BigNum operator/(UINT u) const;
	BigNum& operator+=(UINT u);
	BigNum& operator-=(UINT u);
	BigNum& operator*=(UINT u);
	BigNum& operator/=(UINT u);

// Conversion
	void	Format(LPCTSTR Fmt, int Precision, CString& Dst) const;
	void	Format(int Precision, CString& Dst) const;
	CString ToStr(int Precision) const;
	CString ToStr() const;
	bool	FromStr(LPCTSTR s);
	bool	FromStr(LPCTSTR Fmt, LPCTSTR Src);
	void	Serialize(CArchive& ar);

// Version
	enum LIB_TYPE {
		LIB_GMP,
		LIB_MPIR,
		LIB_TYPES
	};
	enum OS_TYPE {
		OS_WIN32,
		OS_WIN64,
		OS_TYPES
	};
	enum CPU_TYPE {
		CPU_PENTIUM4,
		CPU_CORE2,
		CPU_TYPES
	};
	static	int		GetVersion();
	static	int		GetVersionMinor();
	static	int		GetPatchLevel();
	static	int		GetLibType();
	static	int		GetOSType();
	static	int		GetCPUType();
	static	LPCTSTR	GetLibName();
	static	LPCTSTR	GetLibName(int LibType);
	static	LPCTSTR	GetOSName();
	static	LPCTSTR	GetOSName(int OSType);
	static	LPCTSTR	GetCPUName();
	static	LPCTSTR	GetCPUName(int CPUType);

// Data members
	mpf_t	f;

// Implementation
protected:
	static	const LPCTSTR	m_LibName[LIB_TYPES];
	static	const LPCTSTR	m_OSName[OS_TYPES];
	static	const LPCTSTR	m_CPUName[CPU_TYPES];
};

inline BigNum::BigNum()
{
	mpf_init(f);
}

inline BigNum::BigNum(UINT Prec, int)
{
	mpf_init2(f, Prec);
}

inline BigNum::BigNum(const BigNum& n)
{
	mpf_init2(f, mpf_get_prec(n));	// copy precision
	mpf_set(f, n);
}

inline BigNum::BigNum(double d)
{
	mpf_init_set_d(f, d);
}

inline BigNum::BigNum(UINT u)
{
	mpf_init_set_ui(f, u);
}

inline BigNum::BigNum(int i)
{
	mpf_init_set_si(f, i);
}

inline BigNum::BigNum(UINT Prec, double d, int)
{
	mpf_init2(f, Prec);
	mpf_set_d(f, d);
}

inline BigNum::BigNum(UINT Prec, UINT u, int)
{
	mpf_init2(f, Prec);
	mpf_set_ui(f, u);
}

inline BigNum::BigNum(UINT Prec, int i, int)
{
	mpf_init2(f, Prec);
	mpf_set_si(f, i);
}

inline BigNum::~BigNum()
{
	mpf_clear(f);
}

inline BigNum::operator double() const
{
	return(mpf_get_d(f));
}

inline BigNum::operator __mpf_struct*()
{
	return(f);
}

inline BigNum::operator const __mpf_struct*() const
{
	return(f);
}

inline BigNum& BigNum::operator=(const BigNum& n)
{
	mpf_set(f, n);
	return(*this);
}

inline UINT BigNum::GetPrec() const
{
	return(mpf_get_prec(f));
}

inline void BigNum::SetPrec(UINT Bits)
{
	mpf_set_prec(f, Bits);
}

inline bool BigNum::operator==(const BigNum& n) const
{
	return(mpf_cmp(f, n) == 0);
} 

inline bool BigNum::operator!=(const BigNum& n) const
{
	return(mpf_cmp(f, n) != 0);
}

inline bool BigNum::operator>(const BigNum& n) const
{
	return(mpf_cmp(f, n) > 0);
}

inline bool BigNum::operator<(const BigNum& n) const
{
	return(mpf_cmp(f, n) < 0);
}

inline bool BigNum::operator>=(const BigNum& n) const
{
	return(mpf_cmp(f, n) >= 0);
}

inline bool BigNum::operator<=(const BigNum& n) const
{
	return(mpf_cmp(f, n) <= 0);
}

inline bool BigNum::operator==(double d) const
{
	return(mpf_cmp(f, BigNum(d)) == 0);
} 

inline bool BigNum::operator!=(double d) const
{
	return(mpf_cmp(f, BigNum(d)) != 0);
}

inline bool BigNum::operator>(double d) const
{
	return(mpf_cmp(f, BigNum(d)) > 0);
}

inline bool BigNum::operator<(double d) const
{
	return(mpf_cmp(f, BigNum(d)) < 0);
}

inline bool BigNum::operator>=(double d) const
{
	return(mpf_cmp(f, BigNum(d)) >= 0);
}

inline bool BigNum::operator<=(double d) const
{
	return(mpf_cmp(f, BigNum(d)) <= 0);
}

inline bool BigNum::operator==(UINT u) const
{
	return(mpf_cmp_ui(f, u) == 0);
} 

inline bool BigNum::operator!=(UINT u) const
{
	return(mpf_cmp_ui(f, u) != 0);
}

inline bool BigNum::operator>(UINT u) const
{
	return(mpf_cmp_ui(f, u) > 0);
}

inline bool BigNum::operator<(UINT u) const
{
	return(mpf_cmp_ui(f, u) < 0);
}

inline bool BigNum::operator>=(UINT u) const
{
	return(mpf_cmp_ui(f, u) >= 0);
}

inline bool BigNum::operator<=(UINT u) const
{
	return(mpf_cmp_ui(f, u) <= 0);
}

inline bool BigNum::operator==(int i) const
{
	return(mpf_cmp_si(f, i) == 0);
} 

inline bool BigNum::operator!=(int i) const
{
	return(mpf_cmp_si(f, i) != 0);
}

inline bool BigNum::operator>(int i) const
{
	return(mpf_cmp_si(f, i) > 0);
}

inline bool BigNum::operator<(int i) const
{
	return(mpf_cmp_si(f, i) < 0);
}

inline bool BigNum::operator>=(int i) const
{
	return(mpf_cmp_si(f, i) >= 0);
}

inline bool BigNum::operator<=(int i) const
{
	return(mpf_cmp_si(f, i) <= 0);
}

inline const BigNum BigNum::operator+(const BigNum& n) const
{
	BigNum	t(GetPrec(), 0);
	mpf_add(t, f, n);
	return(t);
}

inline const BigNum BigNum::operator-(const BigNum& n) const
{ 
	BigNum	t(GetPrec(), 0);
	mpf_sub(t, f, n);
	return(t);
}

inline const BigNum BigNum::operator*(const BigNum& n) const
{
	BigNum	t(GetPrec(), 0);
	mpf_mul(t, f, n);
	return(t);
}

inline const BigNum BigNum::operator/(const BigNum& n) const
{
	BigNum	t(GetPrec(), 0);
	mpf_div(t, f, n);
	return(t);
}

inline BigNum& BigNum::operator+=(const BigNum& n)
{
	*this = *this + n; return(*this);
}

inline BigNum& BigNum::operator-=(const BigNum& n)
{
	*this = *this - n; return(*this);
}

inline BigNum& BigNum::operator*=(const BigNum& n)
{
	*this = *this * n; return(*this);
}

inline BigNum& BigNum::operator/=(const BigNum& n)
{
	*this = *this / n; return(*this);
}

inline const BigNum BigNum::operator+(double d) const
{
	BigNum	t(GetPrec(), 0);
	mpf_add(t, f, BigNum(d));
	return(t);
}

inline const BigNum BigNum::operator-(double d) const
{ 
	BigNum	t(GetPrec(), 0);
	mpf_sub(t, f, BigNum(d));
	return(t);
}

inline const BigNum BigNum::operator*(double d) const
{
	BigNum	t(GetPrec(), 0);
	mpf_mul(t, f, BigNum(d));
	return(t);
}

inline const BigNum BigNum::operator/(double d) const
{
	BigNum	t(GetPrec(), 0);
	mpf_div(t, f, BigNum(d));
	return(t);
}

inline BigNum& BigNum::operator+=(double d)
{
	*this = *this + d; return(*this);
}

inline BigNum& BigNum::operator-=(double d)
{
	*this = *this - d; return(*this);
}

inline BigNum& BigNum::operator*=(double d)
{
	*this = *this * d; return(*this);
}

inline BigNum& BigNum::operator/=(double d)
{
	*this = *this / d; return(*this);
}

inline const BigNum BigNum::operator+(UINT u) const
{
	BigNum	t(GetPrec(), 0);
	mpf_add_ui(t, f, u);
	return(t);
}

inline const BigNum BigNum::operator-(UINT u) const
{ 
	BigNum	t(GetPrec(), 0);
	mpf_sub_ui(t, f, u);
	return(t);
}

inline const BigNum BigNum::operator*(UINT u) const
{
	BigNum	t(GetPrec(), 0);
	mpf_mul_ui(t, f, u);
	return(t);
}

inline const BigNum BigNum::operator/(UINT u) const
{
	BigNum	t(GetPrec(), 0);
	mpf_div_ui(t, f, u);
	return(t);
}

inline BigNum& BigNum::operator+=(UINT u)
{
	*this = *this + u; return(*this);
}

inline BigNum& BigNum::operator-=(UINT u)
{
	*this = *this - u; return(*this);
}

inline BigNum& BigNum::operator*=(UINT u)
{
	*this = *this * u; return(*this);
}

inline BigNum& BigNum::operator/=(UINT u)
{
	*this = *this / u; return(*this);
}

inline int BigNum::GetVersion()
{
#ifdef _WIN64
	return(__MPIR_VERSION);
#else
	return(__GNU_MP_VERSION);
#endif
}

inline int BigNum::GetVersionMinor()
{
#ifdef _WIN64
	return(__MPIR_VERSION_MINOR);
#else
	return(__GNU_MP_VERSION_MINOR);
#endif
}

inline int BigNum::GetPatchLevel()
{
#ifdef _WIN64
	return(__MPIR_VERSION_PATCHLEVEL);
#else
	return(__GNU_MP_VERSION_PATCHLEVEL);
#endif
}

inline int BigNum::GetLibType()
{
#ifdef _WIN64
	return(LIB_MPIR);
#else
	return(LIB_GMP);
#endif
}

inline int BigNum::GetOSType()
{
#ifdef _WIN64
	return(OS_WIN64);
#else
	return(OS_WIN32);
#endif
}

inline int BigNum::GetCPUType()
{
#ifdef _WIN64
	return(CPU_CORE2);
#else
	return(CPU_PENTIUM4);
#endif
}

inline LPCTSTR BigNum::GetLibName()
{
	return(GetLibName(GetLibType()));
}

inline LPCTSTR BigNum::GetLibName(int LibType)
{
	ASSERT(LibType >= 0 && LibType < LIB_TYPES);
	return(m_LibName[LibType]);
}

inline LPCTSTR BigNum::GetCPUName()
{
	return(GetCPUName(GetCPUType()));
}

inline LPCTSTR BigNum::GetCPUName(int CPUType)
{
	ASSERT(CPUType >= 0 && CPUType < CPU_TYPES);
	return(m_CPUName[CPUType]);
}

inline LPCTSTR BigNum::GetOSName()
{
	return(GetOSName(GetOSType()));
}

inline LPCTSTR BigNum::GetOSName(int OSType)
{
	ASSERT(OSType >= 0 && OSType < OS_TYPES);
	return(m_OSName[OSType]);
}

#endif
