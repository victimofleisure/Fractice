// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda

		rev		date		comments
		00		12jan01		initial version

		wrapper for sockets initialization and cleanup

*/

#include "stdafx.h"
#include "Sockets.h"
#include "Error.h"

UINT	WSockets::m_RefCount;

WSockets::WSockets()
{
	if (!(m_RefCount++)) {
		if (WSAStartup(0x0101, &m_wsaData))
			throw WError(SOCKETS_CANT_INITIALIZE);
		if (m_wsaData.wVersion != 0x0101)
			throw WError(SOCKETS_WRONG_VERSION);
	}
}

WSockets::~WSockets()
{
	if (!(--m_RefCount)) {
		WSACleanup();
	}
}

