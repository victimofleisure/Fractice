// Copyleft 2004 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      22feb04	initial version
		01		07mar04	add limit
		02		08mar04	in SwapState, set busy before finding undoable
		03		08mar04	in NotifyEdit, ctor isn't called when array shrinks
		04		12mar04	if coalescing, remove states above current position
		05		29sep04	cancel edit must update titles
		06		19mar05	bump m_Edits regardless of number of undo levels
        07      22nov06 rename strings to start with IDS_
        08      25nov06 use CArrayishList instead of CList
		09		23nov07	support Unicode
		10		03jan08	replace CSmartBuf with CRefPtr
		11		18mar08	remove key support
		12		19feb09	in NotifyEdit, fix stack size limit case

        undoable edit interface
 
*/

#include "stdafx.h"
#include "Resource.h"
#include "UndoManager.h"

#if UNDO_NATTER
#define	UNDO_DUMP_STATE(Tag, Title, Pos) DumpState(Tag, Title, Pos);
#else
#define	UNDO_DUMP_STATE(Tag, Title, Pos)
#endif

bool CUndoManager::m_Busy;

CUndoManager::CUndoManager(CUndoable *Root) :
	m_Root(Root)
{
	m_Pos = 0;
	m_Levels = INT_MAX;
	m_Edits = 0;
}

CUndoManager::~CUndoManager()
{
}

void CUndoManager::Undo()
{
	if (IsUndoable()) {
		UNDO_DUMP_STATE(_T("Undo"), m_UndoTitle, m_Pos - 1);
		SwapState(--m_Pos);
	}
}

void CUndoManager::Redo()
{
	if (IsRedoable()) {
		UNDO_DUMP_STATE(_T("Redo"), m_RedoTitle, m_Pos);
		SwapState(m_Pos++);
	}
}

void CUndoManager::SwapState(int Pos)
{
	ASSERT(Pos >= 0 && Pos < m_List.GetSize());
	CUndoState	PrevState = m_List[Pos];
	m_Busy = TRUE;
	CUndoable	*uap = m_Root;
	if (uap != NULL) {
		uap->SaveUndoState(m_List[Pos]);
		uap->RestoreUndoState(PrevState);
	} 
	m_Busy = FALSE;
	UpdateTitles();
#if UNDO_NATTER
	if (uap == NULL)
		_tprintf(_T("Can't find instance.\n"));
#endif
}

void CUndoManager::NotifyEdit(CUndoable *Adr, WORD CtrlID, WORD Code, UINT Flags)
{
	if (Adr == NULL)
		return;
	if (!m_Busy) {
		// if coalesce requested and notifier's key matches top of stack
		if ((Flags & CUndoable::UE_COALESCE) && m_Pos 
		&& m_List[m_Pos - 1].IsMatch(CtrlID, Code)) {
			if (m_List.GetSize() > m_Pos)	// don't resize array needlessly
				m_List.SetSize(m_Pos);	// remove states above current position
			return;
		}
		if (!(Flags & CUndoable::UE_INSIGNIFICANT)) {
			if (!m_Edits++)			// if first modification
				OnModify(TRUE);		// call derived handler
		}
		if (m_Levels <= 0)
			return;
		if (m_Pos >= m_Levels) {	// if stack size at limit
			m_List.RemoveAt(0);	// remove bottom state
			m_Pos--;
		}
		m_List.SetSize(m_Pos);	// array shrinks if we've undone
		CUndoState	us;
		us.m_Val.i64 = 0;
		us.m_CtrlID = CtrlID;
		us.m_Code = Code;
		m_List.Add(us);
		CUndoState	*usp = &m_List[m_Pos];
		Adr->SaveUndoState(*usp);
		Adr->GetUndoTitle(*usp, m_UndoTitle);
		m_RedoTitle.Empty();
		UNDO_DUMP_STATE(_T("Notify"), m_UndoTitle, m_Pos);
		m_Pos++;
	} else {
#if UNDO_NATTER
		_tprintf(_T("Ignoring notify.\n"));
#endif
	}
}

void CUndoManager::CancelEdit(CUndoable *Adr, WORD CtrlID, WORD Code)
{
	if (Adr == NULL)
		return;
#if UNDO_NATTER
	_tprintf(_T("CancelEdit CtrlID=%d Code=%d\n"), CtrlID, Code);
#endif
	int	i;
	for (i = m_Pos - 1; i >= 0; i--) {
		if (m_List[i].IsMatch(CtrlID, Code))
			break;
	}
	if (i >= 0) {
		m_List.RemoveAt(i);
		m_Pos--;
		if (!--m_Edits)			// if last modification
			OnModify(FALSE);	// call derived handler
		UpdateTitles();
	}
#if UNDO_NATTER
	if (i < 0)
		_tprintf(_T("Can't cancel edit.\n"));
#endif
}

void CUndoManager::DiscardAllEdits()
{
#if UNDO_NATTER
	_tprintf(_T("DiscardAllEdits\n"));
#endif
	m_List.SetSize(0);
	m_Pos = 0;
	m_Edits = 0;
	OnModify(FALSE);	// call derived handler
	UpdateTitles();
}

bool CUndoManager::IsUndoable() const
{
	return(m_Pos > 0);
}

bool CUndoManager::IsRedoable() const
{
	return(m_Pos < m_List.GetSize());
}

void CUndoManager::DumpState(LPCTSTR Tag, LPCTSTR Title, int Pos)
{
	_tprintf(_T("%s '%s' Pos=%d %s Obj=0x%x\n"), Tag, Title, Pos, 
		m_List[Pos].DumpState(), m_List[Pos].GetObj());
}

void CUndoManager::UpdateTitles()
{
	if (IsUndoable())
		GetTitle(m_Pos - 1, m_UndoTitle);
	else
		m_UndoTitle.Empty();
	if (IsRedoable())
		GetTitle(m_Pos, m_RedoTitle);
	else
		m_RedoTitle.Empty();
}

void CUndoManager::GetTitle(int Pos, CString& Title)
{
	CUndoable	*uap = m_Root;
	if (uap != NULL)
		uap->GetUndoTitle(m_List[Pos], Title);
	else
		Title.Empty();
}

void CUndoManager::SetLevels(int Levels)
{
	if (Levels < 0)
		Levels = INT_MAX;
	if (Levels < m_List.GetSize()) {	// if shrinking history
		if (m_Pos < Levels)	// if undo depth is below new size
			DiscardAllEdits();	// history can't be saved
		else {	// trim excess history from the bottom
			int	Excess = m_List.GetSize() - Levels;
			m_List.RemoveAt(0, Excess);
			m_Pos = max(m_Pos - Excess, 0);
		}
	}
	m_Levels = Levels;
}

void CUndoManager::OnModify(bool Modified)
{
}
