// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14mar09	initial version
		01		05jul09	in ShowNextSlide, remove unused var

        slide show manager
 
*/

#include "stdafx.h"
#include "Fractice.h"
#include "SlideShowMgr.h"
#include "FracticeDoc.h"
#include "FracticeView.h"
#include "MainFrm.h"

#define RK_SLIDE_ORDER		_T("SlideOrder")
#define RK_SLIDE_LOOP		_T("SlideLoop")
#define RK_SLIDE_INTERVAL	_T("SlideInterval")

CSlideShowMgr::CSlideShowMgr()
{
	m_Hist = NULL;
	m_TimerID = 0;
	m_Interval = theApp.RdRegDouble(RK_SLIDE_INTERVAL, DEFAULT_INTERVAL);
	m_Order = theApp.RdRegInt(RK_SLIDE_ORDER, OT_SEQUENTIAL);
	m_Loop = theApp.RdRegBool(RK_SLIDE_LOOP, FALSE);
	m_Running = FALSE;
	m_Transport = 0;
	m_CurPos = 0;
}

CSlideShowMgr::~CSlideShowMgr()
{
	theApp.WrRegDouble(RK_SLIDE_INTERVAL, m_Interval);
	theApp.WrRegInt(RK_SLIDE_ORDER, m_Order);
	theApp.WrRegInt(RK_SLIDE_LOOP, m_Loop);
}

void CSlideShowMgr::OnSelectionChange()
{
	if (!m_Hist->HaveSelection())	// if no selection
		m_Hist->SelectAll();	// select all
	m_Hist->GetSelection(m_SelIdx);
	m_RandList.Init(GetItemCount());
}

void CSlideShowMgr::OnIntervalChange()
{
	m_Timer.SetPeriod(round(1000.0 * m_Interval));
}

void CSlideShowMgr::SetInterval(double Interval)
{
	m_Interval = Interval;
	OnIntervalChange();
}

bool CSlideShowMgr::SetTransport(int State)
{
	if (State == m_Transport)
		return(TRUE);	// nothing to do
	bool	retc = TRUE;	// assume success
	switch (State) {
	case TS_STOP:
		m_Timer.Destroy();
		m_Running = FALSE;
		break;
	case TS_PAUSE:
		m_Timer.Run(FALSE);
		break;
	case TS_PLAY:
		if (m_Running) {
			if (ShowNextSlide())
				m_Timer.Run(TRUE);
			else
				retc = FALSE;
		} else {
			OnSelectionChange();
			m_CurPos = -1;
			if (ShowNextSlide()) {
				m_Timer.Create(m_Hist->m_hWnd, m_TimerID, 0);
				OnIntervalChange();
				m_Running = TRUE;
			} else
				retc = FALSE;
		}
		break;
	default:
		ASSERT(0);
	}
	m_Transport = retc ? State : TS_STOP;
	if (m_Hist->GetParentView() == theApp.GetMain()->GetView())
		theApp.GetMain()->GetSlideShowDlg().Update();
	return(retc);
}

bool CSlideShowMgr::ShowNextSlide()
{
	while (m_SelIdx.GetSize()) {
		switch (m_Order) {
		case OT_RANDOM:
			if (!m_RandList.GetAvail() && !m_Loop) {
				m_CurPos = -1;
				break;
			}
			m_CurPos = m_RandList.GetNext();
			break;
		case OT_SEQUENTIAL:
			m_CurPos++;
			if (m_CurPos >= m_SelIdx.GetSize()) {
				if (!m_Loop) {
					m_CurPos = -1;
					break;
				}
				m_CurPos = 0;
			}
			break;
		default:
			ASSERT(0);
		}
		if (m_CurPos < 0)	// no more items
			break;
		int	HistPos = m_SelIdx[m_CurPos];
		if (HistPos < m_Hist->GetItemCount()) {	// if valid history position
			m_Hist->RenderItem(HistPos, CHistoryView::RF_ALLOW_REMAP);
			if (m_Hist->GetParentView() == theApp.GetMain()->GetView())
				theApp.GetMain()->GetSlideShowDlg().UpdateStatus();
			return(TRUE);	// success
		}
		OnSelectionChange();	// assume history items deleted; rebuild our list
	}
	Stop();
	return(FALSE);
}
