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
		04		01feb09	add color offset
		05		07feb09	add escape time array
		06		14feb09	in CRefEscTimeArray, add owner snapshot index
		07		06jan10	W64: in SerializeElements, nCount is 64-bit

        snapshot container
 
*/

#ifndef CSNAPSHOT_INCLUDED
#define CSNAPSHOT_INCLUDED

#include "RefPtr.h"
#include "BigRect.h"
#include "Palette.h"
#include "Dib.h"
#include "Params.h"
#include "EngineTypes.h"

// reference-counted bitmap
class CRefBitmap : public CBitmap, public CRefObj {
public:
};

// pointer to reference-counted bitmap
typedef CRefPtr<CRefBitmap> CBitmapPtr;

class CRefDib: public CDib, public CRefObj {
public:
};

// pointer to reference-counted DIB
typedef CRefPtr<CRefDib> CDibPtr;

// array of escape times
typedef	CArrayEx<ESCTIME, ESCTIME> CEscTimeArray;

// reference-counted array of escape times
class CRefEscTimeArray: public CEscTimeArray, public CRefObj {
public:
	int		m_OwnerSnapIdx;	// index of owner snapshot in history
};

// pointer to reference-counted escape time array
typedef CRefPtr<CRefEscTimeArray> CEscTimeArrayPtr;

class CSnapshot : public CRefObj {
public:
// Construction
	CSnapshot();
	CSnapshot(const CSnapshot& Snap);
	CSnapshot& operator=(const CSnapshot& Snap);

// Comparison
	bool	operator==(const CSnapshot& Snap) const;
	bool	operator!=(const CSnapshot& Snap) const;
	bool	EqualIgnoreColor(const CSnapshot& Snap) const;

// Attributes
	bool	IsEmpty() const;
	void	SetEmpty();
	void	SetMathPrec(UINT MathPrec);
	double	GetFrameAspect() const;
	BigNum	GetFrameAspectBig() const;
	void	GetBounds(BigRect& Bounds) const;
	double	GetNominalSize() const;
	bool	HasThumb() const;
	bool	HasImage() const;
	bool	HasEscTimes() const;

// Operations
	void	Serialize(CArchive& ar);
	bool	Write(CStdioFile& fp);
	bool	Read(CStdioFile& fp);
	bool	Write(LPCTSTR Path);
	bool	Read(LPCTSTR Path);

// Constants
	enum {
		FILE_VERSION = 1,	// file version number
		ARCHIVE_VERSION = 1	// archive version number
	};

// Member data
	// add new members to ctor, Copy, operator==, Serialize, and Read/Write
	BigPoint	m_Origin;	// fractal origin
	BigNum	m_Zoom;			// fractal zoom
	UINT	m_FractalType;	// type of fractal
	CSize	m_ImageSize;	// image size in pixels
	UINT	m_Quality;		// iteration limit
	UINT	m_Antialias;	// oversampling factor
	UINT	m_MathPrec;		// math precision in bits
	UINT	m_CycleLen;		// palette cycle length
	bool	m_DeepZoom;		// true if using bignums
	BYTE	m_Exponent;		// exponent to raise complex Z to
	double	m_ColorOffset;	// palette color offset
	CBitmapPtr	m_Thumb;	// thumbnail bitmap smart pointer
	CRect	m_ThumbFrame;	// thumbnail frame rectangle
	CRect	m_ThumbDetail;	// detail rectangle within thumbnail
	DPalette	m_Palette;	// palette data
	CDibPtr	m_Image;		// rendered image DIB smart pointer
	CEscTimeArrayPtr	m_EscTimes;	// escape time array smart pointer

// Helpers
	void	Copy(const CSnapshot& Snap);
};

typedef CRefPtr<CSnapshot> CSnapshotPtr;
typedef CArrayEx<CSnapshot, CSnapshot&> CSnapshotArray;

inline CSnapshot& CSnapshot::operator=(const CSnapshot& Snap)
{
	Copy(Snap);
	return(*this);
}

inline bool CSnapshot::IsEmpty() const
{
	return(!m_ImageSize.cx);
}

inline bool CSnapshot::operator!=(const CSnapshot& Snap) const
{
	return(!(*this == Snap));
}

inline double CSnapshot::GetFrameAspect() const
{
	return(double(m_ImageSize.cx) / m_ImageSize.cy);
}

inline BigNum CSnapshot::GetFrameAspectBig() const
{
	return(BigNum(m_MathPrec, m_ImageSize.cx, 0) / UINT(m_ImageSize.cy));
}

inline double CSnapshot::GetNominalSize() const
{
	return(3.0);	// fractal size at zoom == 1
}

inline bool CSnapshot::HasThumb() const
{
	return(!m_Thumb.IsEmpty());
}

inline bool CSnapshot::HasImage() const
{
	return(!m_Image.IsEmpty());
}

inline bool CSnapshot::HasEscTimes() const
{
	return(!m_EscTimes.IsEmpty());
}

template<> inline void AFXAPI 
SerializeElements<CSnapshot>(CArchive& ar, CSnapshot *pNew, W64INT nCount)
{
	for (int i = 0; i < nCount; i++, pNew++)
		pNew->Serialize(ar);
}

#endif
