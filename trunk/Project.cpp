// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda

		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		06feb09	add color cycling
		02		07feb09	add escape time array
		03		14feb09	avoid redundant storage of shared escape time arrays
		04		13mar09	replace image/escape times with snapshot

        project container

*/

#include "stdafx.h"
#include "Project.h"

CProject::CProject()
{
	Reset();
}

void CProject::Reset()
{
	m_Version = ARCHIVE_VERSION;
	m_History.RemoveAll();
	m_HistPos = -1;
	m_ThumbSize = CSize(0, 0);
	m_RecInfo.Reset();
	m_Snapshot.SetEmpty();
	m_CycleColors = FALSE;
	m_ColorCycleRate = 0;
}

void CProject::Copy(const CProject& Proj)
{
	m_Version			= Proj.m_Version;
	m_History.Copy(Proj.m_History);
	m_HistPos			= Proj.m_HistPos;
	m_ThumbSize			= Proj.m_ThumbSize;
	m_RecInfo			= Proj.m_RecInfo;
	m_Snapshot			= Proj.m_Snapshot;
	m_CycleColors		= Proj.m_CycleColors;
	m_ColorCycleRate	= Proj.m_ColorCycleRate;
}

void CProject::Serialize(CArchive& ar)
{
	if (ar.IsStoring()) {
		ar << ARCHIVE_SIG;
		ar << m_Version;
		StoreHistory(ar);
		ar << m_HistPos;
		ar << m_ThumbSize;
		StoreBool(ar, m_CycleColors);
		ar << m_ColorCycleRate;
	} else {
		UINT	sig;
		ar >> sig;
		ar >> m_Version;
		if (sig != ARCHIVE_SIG || m_Version > ARCHIVE_VERSION)
			AfxThrowArchiveException(CArchiveException::badIndex, ar.m_strFileName);
		LoadHistory(ar);
		ar >> m_HistPos;
		ar >> m_ThumbSize;
		LoadBool(ar, m_CycleColors);
		ar >> m_ColorCycleRate;
	}
	m_RecInfo.Serialize(ar);
	m_Snapshot.Serialize(ar);
}

// The Escape Time Array (ETA) is a reference-counted object, and it is
// therefore possible for multiple history snapshots to share the same ETA.
// Some edits don't generate new escape times (for example edits that only
// change the palette or color mapping), and redundantly storing the ETA in
// these cases would be a huge waste of space.
//
// When a snapshot with an ETA is serialized, it unconditionally stores the ETA
// in the archive.  Consequently, if we simply iterated over the history, and
// serialized each of its snapshots, shared ETAs would be stored redundantly in
// the document.  Even worse, when we reopened the document, the redundancy
// would now occur in memory too.
//
// To avoid this, we need a more sophisticated scheme that preserves the ETA
// sharing information.  The scheme requires adding an extra member to the ETA,
// called m_OwnerSnapIdx.  This member contains either the history index of the
// snapshot that serialized the ETA (the "owner" index), or a reserved value
// (ETAO_NONE) indicating that the ETA hasn't been serialized yet.

// Before we store the history, all ETAs linked to the history have their owner
// index initialized to ETAO_NONE.  We then iterate over the history, and for
// each snapshot that has an ETA, we check the ETA's owner.  If the ETA doesn't
// have an owner, we know it hasn't been stored yet, so we write ETAO_NONE to
// the archive, and then store the snapshot as usual, including the ETA.  We
// also set the ETA's owner to the index of the current snapshot.
//
// If the ETA does have an owner, we know that the ETA is shared, and was
// already stored in the archive.  In this case we write the owner index to the
// archive, and then store the snapshot, but omitting the ETA.  We prevent the
// snapshot from storing its ETA using the following method:
//
// 1.  Back up the snapshot's ETA pointer.  The backup pointer's constructor
// increments the ETA's reference count.  By saving the pointer, we're also
// adding a reference to the ETA.
//
// 2.  Set the snapshot's ETA pointer to NULL.  This unlinks the snapshot from
// the ETA and decrements the ETA's reference count, but our backup pointer
// keeps the ETA from being deleted.
//
// 3.  Serialize the snapshot.  Since the snapshot's ETA pointer is temporarily
// NULL, the snapshot writes a "no escape times" value to the archive, avoiding
// the potential duplicate.
//
// 4.  Restore the snapshot's ETA pointer.  This adds a reference to the ETA,
// but since the backup pointer is destroyed when it goes out of scope, there's
// no net change to the ETA's reference count.
//
// When we reload the project, we read the owner index, and then the snapshot.
// If the owner index is ETAO_NONE, no special action is required.  Otherwise,
// we know the current snapshot has a shared ETA, and we also know the location
// of the ETA's owner in the history.  We can then link the current snapshot to
// its ETA, by adding a reference to the ETA.
//
void CProject::StoreHistory(CArchive& ar)
{
	int	snaps = m_History.GetSize();
	ar << snaps;	// store history size
	int	i;
	for (i = 0; i < snaps; i++) {	// for each history snapshot
		ASSERT(!m_History[i].IsEmpty());
		CSnapshot&	snap = m_History[i];
		if (snap.HasEscTimes())	// if snapshot has an ETA
			snap.m_EscTimes->m_OwnerSnapIdx = ETAO_NONE;	// init owner to none
	}
	for (i = 0; i < snaps; i++) {	// for each history snapshot
		CSnapshot&	snap = m_History[i];
		if (snap.HasEscTimes()) {	// if snapshot has an ETA
			// if ETA hasn't been stored in the archive yet
			if (snap.m_EscTimes->m_OwnerSnapIdx == ETAO_NONE) {
				ar << ETAO_NONE;	// identify current snapshot as ETA's owner
				snap.Serialize(ar);	// serialize snapshot, including ETA
				snap.m_EscTimes->m_OwnerSnapIdx = i;	// set ETA's owner index
			} else {	// ETA is shared, and was already stored
				ar << snap.m_EscTimes->m_OwnerSnapIdx;	// store owner index
				CEscTimeArrayPtr	temp(snap.m_EscTimes);	// back up pointer
				snap.m_EscTimes = NULL;	// unlink snapshot from ETA
				snap.Serialize(ar);	// serialize snapshot, excluding ETA
				snap.m_EscTimes = temp;	// restore snapshot's ETA pointer
			}
		} else {	// snapshot doesn't have an ETA
			ar << ETAO_NONE;	// owner index is moot
			snap.Serialize(ar);	// serialize snapshot
		}
	}
}

void CProject::LoadHistory(CArchive& ar)
{
	int	snaps;
	ar >> snaps;	// load history size
	m_History.SetSize(snaps);	// allocate space
	for (int i = 0; i < snaps; i++) {	// for each history snapshot
		int	OwnerSnapIdx;
		ar >> OwnerSnapIdx;	// load owner index
		m_History[i].CreateObj();	// create snapshot
		m_History[i]->Serialize(ar);	// serialize snapshot
		if (OwnerSnapIdx != ETAO_NONE) {	// if snapshot's ETA has a owner
			ASSERT(OwnerSnapIdx >= 0 && OwnerSnapIdx < m_History.GetSize());
			// link snapshot to its ETA, adding a reference to ETA
			m_History[i]->m_EscTimes = m_History[OwnerSnapIdx]->m_EscTimes;
		}
	}
}
