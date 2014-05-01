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

#if !defined(AFX_RECENTITEMLIST_H__99B4EEB6_9380_4505_8BFB_AC4E2E67FD23__INCLUDED_)
#define AFX_RECENTITEMLIST_H__99B4EEB6_9380_4505_8BFB_AC4E2E67FD23__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RecentItemList.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRecentItemList 

class CRecentItemList : public CObject
{
// Construction
public:
	CRecentItemList(UINT nStart, LPCTSTR lpszSection, LPCTSTR lpszEntryFormat, int nSize);

// Attributes
public:
	int		GetSize() const;
	CString&	operator[](int nIndex);

// Operations
public:
	void	Remove(int nIndex);
	void	Add(LPCTSTR lpszItem);
	void	UpdateMenu(CCmdUI* pCmdUI);
	void	ReadList();
	void	WriteList();
	void	SetRadio(CCmdUI* pCmdUI, int nIndex, BOOL Enable);
	void	SetCheck(CCmdUI* pCmdUI, int nIndex, BOOL Enable);

// Implementation
protected:
// Member data
	int		m_nSize;			// maximum number of items
	UINT	m_nStart;			// menu numbering offset
	int		m_nFirstItemIdx;	// index of first menu item
	CStringArray	m_arrItem;	// array of item strings
	CString	m_strSectionName;   // registry section name
	CString m_strEntryFormat;	// format string for entries
	CString	m_strOriginal;		// original menu item contents
};

inline int CRecentItemList::GetSize() const
{
	return(m_nSize);
}

inline CString& CRecentItemList::operator[](int nIndex)
{
	return(m_arrItem[nIndex]);
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RECENTITEMLIST_H__99B4EEB6_9380_4505_8BFB_AC4E2E67FD23__INCLUDED_)
