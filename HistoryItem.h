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

#ifndef CHISTORYITEM_INCLUDED
#define CHISTORYITEM_INCLUDED

#include "Snapshot.h"

class CHistoryItem : public CSnapshotPtr {
public:
// Construction
	CHistoryItem();
	CHistoryItem(const CHistoryItem& Item);
	CHistoryItem& operator=(const CHistoryItem& Item);

// Member data
	bool	m_ShowDetail;	// true if detail rectangle should be visible
	bool	m_Selected;		// true if snapshot is currently selected

// Helpers
	void	Copy(const CHistoryItem& Item);
};

inline CHistoryItem::CHistoryItem(const CHistoryItem& Item)
{
	Copy(Item);
}

inline CHistoryItem& CHistoryItem::operator=(const CHistoryItem& Item)
{
	Copy(Item);
	return(*this);
}

typedef CArrayEx<CHistoryItem, CHistoryItem&> CHistory;

#endif
