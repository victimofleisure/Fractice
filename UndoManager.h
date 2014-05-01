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
        02      25nov06 use CArrayishList instead of CList
		03		23nov07	support Unicode
		04		18mar08	remove key support
		05		03feb09	add GetSize, GetPos, GetState

        undoable edit interface
 
*/

#ifndef CUNDOMANAGER_INCLUDED
#define CUNDOMANAGER_INCLUDED

#include "Undoable.h"
#include "ArrayEx.h"

class CUndoManager : public WObject {
public:
// Construction
	CUndoManager(CUndoable *Root);
	virtual	~CUndoManager();

// Attributes
	bool	IsUndoable() const;
	bool	IsRedoable() const;
	static	bool	IsUndoInProgress();
	bool	IsModified() const;
	void	ResetModifiedFlag();
	LPCTSTR	GetUndoTitle();
	LPCTSTR	GetRedoTitle();
	int		GetLevels() const;
	void	SetLevels(int Levels);
	void	SetRoot(CUndoable *Root);
	int		GetSize() const;
	int		GetPos() const;
	const	CUndoState& GetState(int Pos) const;

// Operations
	void	Undo();
	void	Redo();
	void	NotifyEdit(CUndoable *Adr, WORD CtrlID, WORD Code, UINT Flags = 0);
	void	CancelEdit(CUndoable *Adr, WORD CtrlID, WORD Code);
	void	DiscardAllEdits();

protected:
// Overridables
	virtual	void	OnModify(bool Modified);

private:
// Types
	typedef	CArrayEx<CUndoState, CUndoState&> CUndoStateArray;

// Constants
	enum {
		BLOCK_SIZE = 100	// list grows in blocks of this many elements
	};

// Member data
	CUndoable	*m_Root;	// owner of this undo stack
	CUndoStateArray	m_List;	// undo stack; array of undo states
	int		m_Pos;			// current position in undo stack
	int		m_Levels;		// number of undo levels, or -1 for unlimited
	int		m_Edits;		// total number of edits made so far
	static	bool	m_Busy;	// true if an undo/redo is in progress
	CString	m_UndoTitle;	// current undo title for edit menu
	CString	m_RedoTitle;	// current redo title for edit menu

// Helpers
	void	SwapState(int Pos);
	void	DumpState(LPCTSTR Tag, LPCTSTR Title, int Pos);
	void	GetTitle(int Pos, CString& Title);
	void	UpdateTitles();
};

inline bool CUndoManager::IsUndoInProgress()
{
	return(m_Busy);
}

inline bool CUndoManager::IsModified() const
{
	return(m_Edits > 0);
}

inline void CUndoManager::ResetModifiedFlag()
{
	m_Edits = 0;
}

inline int CUndoManager::GetLevels() const
{
	return(m_Levels);
}

inline LPCTSTR CUndoManager::GetUndoTitle()
{
	return(m_UndoTitle);
}

inline LPCTSTR CUndoManager::GetRedoTitle()
{
	return(m_RedoTitle);
}

inline void CUndoManager::SetRoot(CUndoable *Root)
{
	m_Root = Root;
}

inline int CUndoManager::GetSize() const
{
	return(m_List.GetSize());
}

inline int CUndoManager::GetPos() const
{
	return(m_Pos);
}

inline const CUndoState& CUndoManager::GetState(int Pos) const
{
	return(m_List[Pos]);
}

#endif
