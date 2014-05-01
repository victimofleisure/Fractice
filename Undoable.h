// Copyleft 2004 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      22feb04	initial version
        01      22nov06 derive from WObject
		02		18mar08	remove key support

        undoable edit interface
 
*/

#ifndef CUNDOABLE_INCLUDED
#define CUNDOABLE_INCLUDED

#include "UndoState.h"

class CUndoManager;

class CUndoable : public WObject {
public:
// Constants
	enum UE_FLAGS {
		UE_COALESCE			= 0x01,
		UE_INSIGNIFICANT	= 0x02
	};

// Construction
	CUndoable();

// Attributes
	CUndoManager	*GetUndoManager() const;
	void	SetUndoManager(CUndoManager *Mgr);
	CUndoable	*GetUndoHandler() const;
	void	SetUndoHandler(CUndoable *Handler);
	bool	IsUndoInProgress() const;

// Operations
	void	NotifyUndoableEdit(WORD Code, UINT Flags = 0);
	void	CancelUndoableEdit(WORD Code);
	void	ClearUndoHistory();

// Overridables
	virtual	void	SaveUndoState(CUndoState& State) = 0;
	virtual	void	RestoreUndoState(const CUndoState& State) = 0;
	virtual	void	GetUndoTitle(const CUndoState& State, CString& Title) = 0;

private:
// Member data
	CUndoManager	*m_UndoManager;
};

inline CUndoManager *CUndoable::GetUndoManager() const
{
	return(m_UndoManager);
}

inline void CUndoable::SetUndoManager(CUndoManager *Mgr)
{
	m_UndoManager = Mgr;
}

#endif
