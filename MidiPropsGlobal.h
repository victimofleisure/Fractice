// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      03mar09	initial version
		01		18mar09	rename toggle channel back to select channel
		02		23mar09	move channel properties to their own file

		define MIDI-assignable global properties
 
*/

//													range	range
//			name				tag					start	end		controller
MIDI_PROP(	FaderPos,			FADER_POS,			0,		1,		0)
MIDI_PROP(	AutofadeTime,		AUTOFADE_TIME,		0,		1,		0)
MIDI_PROP(	AutofadePlay,		AUTOFADE_PLAY,		0,		1,		0)
MIDI_PROP(	AutofadeLoop,		AUTOFADE_LOOP,		0,		1,		0)
MIDI_PROP(	Mirror,				MIRROR,				0,		1,		0)
MIDI_PROP(	PalTweenTime,		PAL_TWEEN_TIME,		0,		1,		0)
