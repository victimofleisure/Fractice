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

#include "stdafx.h"
#include "RecordLeg.h"
#include <math.h>

CRecordLeg::CRecordLeg()
{
	m_StartFrame = 0;
	m_Frames = 0;
	m_MathPrec = 0;
	m_Origin[0] = BigPoint(0, 0);
	m_Origin[1] = BigPoint(0, 0);
	m_OriginStep = BigPoint(0, 0);
	m_Zoom[0] = 0;
	m_Zoom[1] = 0;
	m_ZoomStep = 0;
	m_ZoomRange = 0;
	m_ZoomingOut = FALSE;
}

void CRecordLeg::Copy(const CRecordLeg& Info)
{
	SetMathPrec(Info.m_MathPrec);	// order matters
	m_StartFrame	= Info.m_StartFrame;
	m_Frames		= Info.m_Frames;
	m_Origin[0]		= Info.m_Origin[0];
	m_Origin[1]		= Info.m_Origin[1];
	m_OriginStep	= Info.m_OriginStep;
	m_Zoom[0]		= Info.m_Zoom[0];
	m_Zoom[1]		= Info.m_Zoom[1];
	m_ZoomStep		= Info.m_ZoomStep;
	m_ZoomRange		= Info.m_ZoomRange;
	m_ZoomingOut	= Info.m_ZoomingOut;
}

void CRecordLeg::SetMathPrec(UINT Bits)
{
	if (Bits == m_MathPrec)
		return;	// nothing to do
	// set precision of BigNum-derived member vars here
	m_Origin[0].SetPrec(Bits);
	m_Origin[1].SetPrec(Bits);
	m_OriginStep.SetPrec(Bits);
	m_Zoom[0].SetPrec(Bits);
	m_Zoom[1].SetPrec(Bits);
	m_ZoomStep.SetPrec(Bits);
	m_ZoomRange.SetPrec(Bits);
	m_MathPrec = Bits;
}

void CRecordLeg::CalcSteps()
{
	m_OriginStep = (m_Origin[1] - m_Origin[0]) / m_Frames;
	m_ZoomStep = pow(m_Zoom[1] / m_Zoom[0], 1.0 / m_Frames);
	m_ZoomRange = m_Zoom[0] / m_Zoom[1];
	m_ZoomingOut = m_Zoom[0] > m_Zoom[1];
}