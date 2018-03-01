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

#ifndef DPALETTE_INCLUDED
#define DPALETTE_INCLUDED

#include "Gradient.h"

class DPalette : public CGradient {
public:
	DECLARE_SERIAL(DPalette);

// Construction
	DPalette();
	DPalette(const DPalette& Palette);
	DPalette& operator=(const DPalette& Palette);

// Constants
	enum {	// import/export flags
		IEF_FIRST_BKGND		= 0x01,	// use first entry as background color
		IEF_SMOOTH_WRAP		= 0x02,	// smooth wraparound from last entry
	};

// Comparison
	bool	operator==(const DPalette& Palette) const;
	bool	operator!=(const DPalette& Palette) const;

// Attributes
	void	GetRGB(COLORREF *Color, UINT Count, int CycleLen, double Offset) const;
	void	GetRGBBk(COLORREF *Color, UINT Count, int CycleLen, double Offset) const;

// Operations
	void	Serialize(CArchive& ar);
	bool	Write(CStdioFile& fp);
	bool	Read(CStdioFile& fp);
	bool	Write(LPCTSTR Path);
	bool	Read(LPCTSTR Path);
	bool	Import(CStdioFile& fp, UINT Flags);
	bool	Export(CStdioFile& fp, UINT Entries, UINT Flags) const;
	bool	Import(LPCTSTR Path, UINT Flags);
	bool	Export(LPCTSTR Path, UINT Entries, UINT Flags) const;
	static	bool	IsNativeFormat(LPCTSTR Path);

// Member data
	// add new members to ctor, Copy, operator==, Serialize, and Read/Write
	COLORREF	m_BkColor;	// background color
	CString	m_Name;			// palette name

// Non-persistent member data
	// add new members to ctor and copy ONLY
	int		m_CurSel;		// currently selected gradient point
	bool	m_Modified;		// true there are changes to apply

// Helpers
	void	Copy(const DPalette& Palette);
};

inline DPalette::DPalette(const DPalette& Palette)
{
	Copy(Palette);
}

inline DPalette& DPalette::operator=(const DPalette& Palette)
{
	Copy(Palette);
	return(*this);
}

inline bool DPalette::operator!=(const DPalette& Palette) const
{
	return(!(*this == Palette));
}

inline void DPalette::GetRGBBk(COLORREF *Color, UINT Count, int CycleLen, double Offset) const
{
	GetRGB(Color, Count, CycleLen, Offset);
	Color[Count] = m_BkColor;	// one extra element for background color
}

#endif