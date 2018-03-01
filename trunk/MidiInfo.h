// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
		00		03mar09	initial version
		01		23mar09	move property enum to MidiSetupDlg.h

		MIDI assignment information
 
*/

#ifndef CMIDIINFO_DEFINED
#define	CMIDIINFO_DEFINED

#include "MidiSetupDlg.h"

enum {	// MIDI channel messages
	MC_NOTE_OFF		= 0x80,
	MC_NOTE_ON		= 0x90,
	MC_KEY_AFT		= 0xa0,
	MC_CTRL_CHG		= 0xb0,
	MC_PROG_CHG		= 0xc0,
	MC_CHAN_AFT		= 0xd0,
	MC_PITCH_BEND	= 0xe0
};

class CMidiInfo : public CObject, public CMidiSetupDlg::INFO {
public:
// Construction
	CMidiInfo();
	CMidiInfo(const CMidiSetupDlg::INFO& Info);

// Attributes

// Operations
	bool	Read(CStdioFile& fp);
	void	Write(CStdioFile& fp);
	bool	Read(LPCTSTR Path);
	bool	Write(LPCTSTR Path);

// Types
	typedef CMidiSetupDlg::ROWINFO	ROWINFO;
	typedef CMidiSetupDlg::INFO	INFO;

// Constants
	enum {
		ROWS = MIDI_PROPS,
		FILE_VERSION = 1,
	};

// Public data

protected:
// Helpers
	bool	DoIO(LPCTSTR Path, DWORD FileMode);
};

#endif
