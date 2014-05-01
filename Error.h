// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda

		rev		date		comments
		00		12jan01		initial version

		provide centralized exception handling

*/

#ifndef WERROR_INCLUDED
#define WERROR_INCLUDED

#include <exception>

enum WERROR_CODE {

	SOCKETS_CANT_INITIALIZE		= 1000,
	SOCKETS_WRONG_VERSION		= 1001,

	TCP_CANT_CREATE_EVENT		= 1100,
	TCP_CANT_LAUNCH_THREAD		= 1101,
	TCP_CANT_KILL_THREAD		= 1102,
	TCP_CANT_CREATE_SOCKET		= 1103,
	TCP_CANT_SET_OPTION			= 1104,
	TCP_CANT_BIND_SOCKET		= 1105,
	TCP_CANT_LISTEN				= 1106,
	TCP_CANT_ACCEPT				= 1107,
	TCP_CANT_SELECT				= 1108,
	TCP_CANT_GET_RCV_COUNT		= 1109,
	TCP_CANT_RECEIVE			= 1110,
	TCP_CANT_LOOKUP_HOST		= 1111,
	TCP_CANT_CONNECT			= 1112,
	TCP_CANT_SEND				= 1113,
	TCP_CANT_SET_NON_BLOCK		= 1114,

	UDP_CANT_CREATE_SOCKET		= 1200,
	UDP_CANT_SET_MODE			= 1201,
	UDP_CANT_SET_OPTION			= 1202,
	UDP_CANT_BIND_SOCKET		= 1203,
	UDP_CANT_LOOKUP_HOST		= 1204,

	FIND_CANT_LAUNCH_THREAD		= 1300,
	FIND_CANT_KILL_THREAD		= 1301
};

class WError : public std::exception {
public:
	WError(WERROR_CODE Code, int Arg = 0);
	const char	*what() const;

protected:
	enum {
		MAX_MESSAGE = 256
	};
	typedef struct {
		WERROR_CODE	Code;				// the error code
		const char *Format;				// format for error message
	} TRANTAB;
	static const TRANTAB TranTab[];		// table of error code translations
	static const char *DefaultFormat;	// default error message format
	char	Message[MAX_MESSAGE];
};

#endif
