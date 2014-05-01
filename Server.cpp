// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda

		rev		date		comments
        00      14feb08	initial version
		01		06jan10	W64: in FindPending, use GetPendingCount
		02		09jan10	add extended capabilities

		derived packets server

*/

#include "stdafx.h"
#include "Server.h"

CServer::CServer()
{
	m_BinIPAddr = 0;
	m_CPUCount = 0;
	m_Threads = 0;
	m_RenderTime = 0;
	m_Progress = 0;
	m_Rendered = 0;
	m_Options = 0;
}

int CServer::FindPending(UINT Item) const
{
	int	items = GetPendingCount();
	for (int i = 0; i < items; i++) {
		if (m_Pending[i] == Item)
			return(i);
	}
	return(-1);
}

bool CServer::RemovePending(UINT Item)
{
	int	ItemIdx = FindPending(Item);
	if (ItemIdx < 0)
		return(FALSE);
	m_Pending.RemoveAt(ItemIdx);
	return(TRUE);
}

void CServer::ResetStatistics()
{
	m_RenderTime = 0;
	m_Progress = 0;
	m_Rendered = 0;
}
