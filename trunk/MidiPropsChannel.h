// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      23mar09	initial version

		define MIDI-assignable channel properties
 
*/

//													range	range
//			name				tag					start	end		controller
MIDI_PROP(	CycleLength,		CYCLE_LENGTH,		0,		1,		0)
MIDI_PROP(	ColorOffset,		COLOR_OFFSET,		0,		1,		0)
MIDI_PROP(	ColorCycleRate,		COLOR_CYCLE_RATE,	0,		1,		0)
MIDI_PROP(	ColorCycleEnable,	COLOR_CYCLE_ENABLE,	0,		1,		0)
MIDI_PROP(	ZoomIn,				ZOOM_IN,			0,		1,		0)
MIDI_PROP(	ZoomOut,			ZOOM_OUT,			0,		1,		0)
MIDI_PROP(	ScrollUp,			SCROLL_UP,			0,		1,		0)
MIDI_PROP(	ScrollDown,			SCROLL_DOWN,		0,		1,		0)
MIDI_PROP(	ScrollLeft,			SCROLL_LEFT,		0,		1,		0)
MIDI_PROP(	ScrollRight,		SCROLL_RIGHT,		0,		1,		0)
MIDI_PROP(	OriginMotion,		ORIGIN_MOTION,		0,		1,		0)
MIDI_PROP(	OriginX,			ORIGIN_X,			0,		1,		0)
MIDI_PROP(	OriginY,			ORIGIN_Y,			0,		1,		0)
MIDI_PROP(	Damping,			DAMPING,			0,		1,		0)
MIDI_PROP(	Tempo,				TEMPO,				0,		1,		0)
MIDI_PROP(	PongSpeed,			PONG_SPEED,			0,		1,		0)
MIDI_PROP(	LissSpeed,			LISS_SPEED,			0,		1,		0)
MIDI_PROP(	LissPhase,			LISS_PHASE,			0,		1,		0)
