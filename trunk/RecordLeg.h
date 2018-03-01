// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      11jul09	initial version

        recording leg container
 
*/

#ifndef CRECORDLEG_INCLUDED
#define CRECORDLEG_INCLUDED

#include "ArrayEx.h"
#include "BigPoint.h"

class CRecordLeg : public WObject {
public:
// Construction
	CRecordLeg();
	CRecordLeg(const CRecordLeg& Info);
	CRecordLeg& operator=(const CRecordLeg& Info);

// Data members
	// don't forget to add new members to Copy
	UINT	m_StartFrame;		// frame index of leg's first frame
	UINT	m_Frames;			// total number of frames in this leg
	UINT	m_MathPrec;			// floating-point precision, in bits
	BigPoint	m_Origin[2];	// initial and final origin
	BigPoint	m_OriginStep;	// origin shift per frame
	BigNum	m_Zoom[2];			// initial and final zoom
	BigNum	m_ZoomStep;			// zoom scaling per frame
	BigNum	m_ZoomRange;		// zoom range (zoom1 / zoom2)
	bool	m_ZoomingOut;		// true if we're zooming out

// Attributes
	void	SetMathPrec(UINT Bits);
	UINT	GetLastFrame() const;
	bool	SpansFrame(UINT FrameIdx);

// Operations
	void	CalcSteps();

protected:
// Helpers
	void	Copy(const CRecordLeg& Info);
};

inline CRecordLeg::CRecordLeg(const CRecordLeg& Info)
{
	Copy(Info);
}

inline CRecordLeg& CRecordLeg::operator=(const CRecordLeg& Info)
{
	Copy(Info);
	return(*this);
}

inline UINT CRecordLeg::GetLastFrame() const
{
	return(m_StartFrame + m_Frames - 1);
}

inline bool CRecordLeg::SpansFrame(UINT FrameIdx)
{
	return(FrameIdx >= m_StartFrame && FrameIdx <= GetLastFrame());
}

typedef CArrayEx<CRecordLeg, CRecordLeg&> CRecordLegArray;

#endif
