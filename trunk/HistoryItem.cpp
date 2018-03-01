// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version

        history item container
 
*/

#include "stdafx.h"
#include "HistoryItem.h"

CHistoryItem::CHistoryItem()
{
	m_ShowDetail = TRUE;
	m_Selected = FALSE;
}

void CHistoryItem::Copy(const CHistoryItem& Item)
{
	CRefPtr<CSnapshot>::Copy(Item);	// copy base class
	m_ShowDetail	= Item.m_ShowDetail;
	m_Selected		= Item.m_Selected;
}

