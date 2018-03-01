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

#ifndef SOCKETS_INCLUDED
#define SOCKETS_INCLUDED

#include <winsock.h>

// The WSockets object wraps the initialization and cleanup of the windows
// sockets API.  When the first instance of this object is created, the sockets
// API is initialized, and when the last instance of this object is destroyed,
// the sockets API is cleaned up.  Thus, if every object that uses the sockets
// API includes a WSockets object as a non-static data member, the sockets API
// will always be correctly initialized and cleaned up.
//
class WSockets : public WObject {
public:
//
// If this is the first instance, initializes the sockets API.
//
// Remarks: If other WSockets objects exist, does nothing.  If the sockets API
// can't be initialized, or isn't version 1.1, a ZERROR exception is thrown.
//
	WSockets();
//
// If this is the last instance, cleans up the sockets API.
//
// Remarks: If other WSockets objects exist, does nothing.
//
	~WSockets();
//
// Retrieves the windows sockets initialization information.
//
	const WSADATA&	GetData() const;

protected:
	WSADATA	m_wsaData;				// Our copy of the initialization info.
	static	UINT	m_RefCount;		// The number of instances of this object.
};

inline const WSADATA& WSockets::GetData() const
{
	return(m_wsaData);
}

#endif
