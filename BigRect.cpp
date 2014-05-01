// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
        
		extended-precision rectangle
 
*/

#include "stdafx.h"
#include "BigRect.h"

void BigRect::Serialize(CArchive& ar)
{
	p1().Serialize(ar);	
	p2().Serialize(ar);	
}
