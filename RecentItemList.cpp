// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version

        list of most recently used items
 
*/

// RecentItemList.cpp : implementation file
//

#include "stdafx.h"
#include "RecentItemList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRecentItemList

CRecentItemList::CRecentItemList(UINT nStart, LPCTSTR lpszSection, LPCTSTR lpszEntryFormat, int nSize)
{
	ASSERT(nSize != 0);
	m_nSize = nSize;
	m_nStart = nStart;
	m_nFirstItemIdx = -1;
	m_arrItem.SetSize(nSize);
	m_strSectionName = lpszSection;
	m_strEntryFormat = lpszEntryFormat;
}

void CRecentItemList::Remove(int nIndex)
{
	m_arrItem[nIndex].Empty();
	int i;
	for (i = nIndex; i < m_nSize - 1; i++)
		m_arrItem[i] = m_arrItem[i + 1];
	m_arrItem[i].Empty();
}

void CRecentItemList::Add(LPCTSTR lpszItem)
{
	int	i;
	for (i = 0; i < m_nSize - 1; i++) {
		if (m_arrItem[i] == lpszItem)
			break;
	}
	for (; i > 0; i--)
		m_arrItem[i] = m_arrItem[i - 1];
	m_arrItem[0] = lpszItem;
}

void CRecentItemList::UpdateMenu(CCmdUI* pCmdUI)
{
	CMenu* pMenu = pCmdUI->m_pMenu;
	if (m_strOriginal.IsEmpty() && pMenu != NULL)
		pMenu->GetMenuString(pCmdUI->m_nID, m_strOriginal, MF_BYCOMMAND);
	if (m_arrItem[0].IsEmpty()) {
		if (!m_strOriginal.IsEmpty())
			pCmdUI->SetText(m_strOriginal);
		pCmdUI->Enable(FALSE);
		return;
	}
	if (pCmdUI->m_pMenu == NULL)
		return;
	int	i;
	for (i = 0; i < m_nSize; i++)
		pCmdUI->m_pMenu->DeleteMenu(pCmdUI->m_nID + i, MF_BYCOMMAND);
	m_nFirstItemIdx = pCmdUI->m_nIndex;
	CString strName;
	CString strTemp;
	for (i = 0; i < m_nSize; i++) {
		if (m_arrItem[i].IsEmpty())
			break;
		pCmdUI->m_pMenu->InsertMenu(pCmdUI->m_nIndex++,
			MF_STRING | MF_BYPOSITION, pCmdUI->m_nID++,
			m_arrItem[i]);
	}
	// update end menu count
	pCmdUI->m_nIndex--; // point to last menu added
	pCmdUI->m_nIndexMax = pCmdUI->m_pMenu->GetMenuItemCount();
	pCmdUI->m_bEnableChanged = TRUE;    // all the added items are enabled
}

void CRecentItemList::ReadList()
{
	CWinApp	*pApp = AfxGetApp();
	CString	strEntry;
	for (int i = 0; i < m_nSize; i++) {
		strEntry.Format(m_strEntryFormat, i + 1);
		m_arrItem[i] = pApp->GetProfileString(
			m_strSectionName, strEntry);
	}
}

void CRecentItemList::WriteList()
{
	CWinApp	*pApp = AfxGetApp();
	pApp->WriteProfileString(m_strSectionName, NULL, NULL);
	CString	strEntry;
	for (int i = 0; i < m_nSize; i++) {
		strEntry.Format(m_strEntryFormat, i + 1);
		if (!m_arrItem[i].IsEmpty())
			pApp->WriteProfileString(m_strSectionName, strEntry, m_arrItem[i]);
	}
}

void CRecentItemList::SetRadio(CCmdUI* pCmdUI, int nIndex, BOOL Enable)
{
	if (m_nFirstItemIdx >= 0) {
		int	tmp = pCmdUI->m_nIndex;
		pCmdUI->m_nIndex = m_nFirstItemIdx + nIndex;
		pCmdUI->SetRadio(Enable);
		pCmdUI->m_nIndex = tmp;
	}
}

void CRecentItemList::SetCheck(CCmdUI* pCmdUI, int nIndex, BOOL Enable)
{
	if (m_nFirstItemIdx >= 0) {
		int	tmp = pCmdUI->m_nIndex;
		pCmdUI->m_nIndex = m_nFirstItemIdx + nIndex;
		pCmdUI->SetCheck(Enable);
		pCmdUI->m_nIndex = tmp;
	}
}
