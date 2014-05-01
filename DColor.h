// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
		00		17oct07	initial version
		01		05mar08	derive from structs, add COLORREF ctors

		double-precision RGB and HLS color containers
 
*/

#ifndef DCOLOR_DEFINED
#define DCOLOR_DEFINED

#pragma pack(push)
#pragma pack(1)	// byte packing so we can cast 'this' to an array of doubles

typedef struct tagDRGB_S {
	double	m_r;	// red
	double	m_g;	// green
	double	m_b;	// blue
} DRGB_S;

class DRGB : public DRGB_S {	// derive from struct
public:
// Construction
	DRGB();
	DRGB(double r, double g, double b);
	DRGB(const DRGB_S& Color);
	DRGB(COLORREF Color);

// Constants
	enum {
		R, G, B, CHANS
	};

// Comparison
	bool	operator==(const DRGB& Color) const;
	bool	operator!=(const DRGB& Color) const;

// Operations
	void	Invert();
	void	RotateHue(double Rotation);

// Operators
	operator COLORREF() const;
	double&	operator[](int Idx);
};

inline DRGB::DRGB()
{
	ZeroMemory(this, sizeof(DRGB));
}

inline DRGB::DRGB(double r, double g, double b)
{
	m_r = r;
	m_g = g;
	m_b = b;
}

inline double& DRGB::operator[](int Idx)
{
	ASSERT(Idx >= 0 && Idx < CHANS);
	return(((double *)this)[Idx]);	// cast 'this' to an array of doubles
}

inline DRGB::DRGB(const DRGB_S& Color)
{
	DRGB_S&	rgb = *this;
	rgb = Color;
}

inline bool DRGB::operator==(const DRGB& Color) const
{
	return(!memcmp(this, &Color, sizeof(DRGB)));	// bitwise compare
}

inline bool DRGB::operator!=(const DRGB& Color) const
{
	return(!(*this == Color));
}

typedef struct tagDHLS_S {
	double	m_h;	// hue
	double	m_l;	// lightness
	double	m_s;	// saturation
} DHLS_S;

class DHLS : public DHLS_S {	// derive from struct
public:
// Construction
	DHLS();
	DHLS(double h, double l, double s);
	DHLS(const DHLS_S& Color);
	DHLS(COLORREF Color);

// Constants
	enum {
		H, L, S, CHANS
	};

// Comparison
	bool	operator==(const DHLS& Color) const;
	bool	operator!=(const DHLS& Color) const;

// Operations
	void	Invert();
	void	RotateHue(double Rotation);

// Operators
	operator COLORREF() const;
	double&	operator[](int Idx);
};

inline DHLS::DHLS()
{
	ZeroMemory(this, sizeof(DHLS));
}

inline DHLS::DHLS(double h, double l, double s)
{
	m_h = h;
	m_l = l;
	m_s = s;
}

inline double& DHLS::operator[](int Idx)
{
	ASSERT(Idx >= 0 && Idx < CHANS);
	return(((double *)this)[Idx]);	// cast 'this' to an array of doubles
}

inline DHLS::DHLS(const DHLS_S& Color)
{
	DHLS_S&	hls = *this;
	hls = Color;
}

inline bool DHLS::operator==(const DHLS& Color) const
{
	return(!memcmp(this, &Color, sizeof(DHLS)));	// bitwise compare
}

inline bool DHLS::operator!=(const DHLS& Color) const
{
	return(!(*this == Color));
}

#pragma pack(pop)

extern	void	hls2rgb(const DHLS_S& hls, DRGB_S& rgb);
extern	void	rgb2hls(const DRGB_S& rgb, DHLS_S& hls);

#endif
