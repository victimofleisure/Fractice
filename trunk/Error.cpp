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

#include "stdafx.h"
#include "Error.h"

const char *WError::DefaultFormat = "ERROR %d:%d";

const WError::TRANTAB WError::TranTab[] = {

{SOCKETS_CANT_INITIALIZE,	"SOCKETS: can't initialize WinSock interface"},
{SOCKETS_WRONG_VERSION,		"SOCKETS: WinSock 1.1 not supported"},

{TCP_CANT_CREATE_EVENT,		"TCP: can't create event"},
{TCP_CANT_LAUNCH_THREAD, 	"TCP: can't launch thread"},
{TCP_CANT_KILL_THREAD,		"TCP: thread refuses to die"},
{TCP_CANT_CREATE_SOCKET, 	"TCP: error %d creating socket"},
{TCP_CANT_SET_OPTION, 		"TCP: error %d setting socket option"},
{TCP_CANT_BIND_SOCKET,		"TCP: error %d binding socket"},
{TCP_CANT_LISTEN,			"TCP: error %d listening on socket"},
{TCP_CANT_ACCEPT,			"TCP: error %d accepting socket"},
{TCP_CANT_SELECT,			"TCP: error %d selecting socket"},
{TCP_CANT_GET_RCV_COUNT, 	"TCP: error %d getting received byte count"},
{TCP_CANT_RECEIVE,			"TCP: error %d receiving from socket"},
{TCP_CANT_LOOKUP_HOST,		"TCP: error %d looking up host by name"},
{TCP_CANT_CONNECT,			"TCP: error %d connecting to host"},
{TCP_CANT_SEND,				"TCP: error %d sending to socket"},
{TCP_CANT_SET_NON_BLOCK, 	"TCP: error %d setting non-blocking mode"},

{UDP_CANT_CREATE_SOCKET, 	"UDP: error %d creating socket"},
{UDP_CANT_SET_MODE,			"UDP: error %d setting socket mode"},
{UDP_CANT_SET_OPTION,		"UDP: error %d setting socket option"},
{UDP_CANT_BIND_SOCKET,		"UDP: error %d binding socket"},
{UDP_CANT_LOOKUP_HOST,		"UDP: error %d looking up host by name"},

{FIND_CANT_LAUNCH_THREAD, 	"FIND: can't launch thread"},
{FIND_CANT_KILL_THREAD,		"FIND: thread refuses to die"},

{(WERROR_CODE)0, NULL}};	// end-of-list marker

WError::WError(WERROR_CODE Code, int Arg)
{
	for (int i = 0; TranTab[i].Format != NULL; i++) {
		if (TranTab[i].Code == Code) {
			sprintf(Message, TranTab[i].Format, Arg);
			return;
		}
	}
	sprintf(Message, DefaultFormat, Code, Arg);
}

const char	*WError::what() const
{
	return(Message);
}
