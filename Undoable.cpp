// Copyleft 2004 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      22feb04	initial version
		01		22nov06	rename strings to start with IDS_
		02		26nov06	cast GetUndoKey to WORD
		03		18mar08	remove key support

        undoable edit interface
 
*/

#include "stdafx.h"
#include "Resource.h"
#include "Undoable.h"
#include "UndoManager.h"

CUndoable::CUndoable()
{
	m_UndoManager = NULL;
}

void CUndoable::NotifyUndoableEdit(WORD Code, UINT Flags)
{
	m_UndoManager->NotifyEdit(this, 0, Code, Flags);
}

void CUndoable::CancelUndoableEdit(WORD Code)
{
	m_UndoManager->CancelEdit(this, 0, Code);
}

void CUndoable::ClearUndoHistory()
{
	m_UndoManager->DiscardAllEdits();
}

bool CUndoable::IsUndoInProgress() const
{
	return(CUndoManager::IsUndoInProgress());
}

