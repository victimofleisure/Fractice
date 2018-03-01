// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version

        parameter definitions
 
*/

#pragma once

#undef PARMDEF
#define PARMDEF(name, member, type, defval, minval, maxval, combo) \
MIN_##name = minval, MAX_##name = maxval, DEF_##name = defval,

enum {	// define minimum, maximum, and default for each parameter
	#include "ParamData.h"
};

#undef PARMDEF
#define PARMDEF(name, member, type, defval, minval, maxval, combo) PARM_##name,
enum {	// enumerate parameters
	#include "ParamData.h"
	NUM_PARMS
};
