// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda

		rev		date		comments
		00		12jan01		initial version
		01		13feb01		add bytes avail
		02		02apr02		add disconnect
		03		21oct02		in read, check for connection closed gracefully
		04		01mar04		ck: add pharlap set debug name
		05		15aug04		ck: wrap events, threads, and timers
		06		18oct04		ck: set event debug names
		07		18oct04		ck: make connection handler's client address a string
		08		14dec06		ck: Disconnect must clear m_AmConnected
		09		28jan08		ck: static cast htons arg to fix type warnings
		10		28jan08		ck: make thread function a static member to fix warning
		11		09jun08		ck: add GetRemoteAddr and GetConnection
		12		12jun08		ck: include limits.h
		13		04jul08		ck: close sockets in Abort to avoid leak
		14		12feb09		ck: add ServerIP argument to ctor

		establish a reliable network connection

*/

#include "stdafx.h"
#include <process.h>
#include <limits.h>
#include "Tcp.h"

WTcp::WTcp(LPCSTR HostName, DWORD Port, LPCSTR ServerIP, bool UsingReadEvent, EXCEPTION_HANDLER Handler)
{
	m_AmServer = (HostName == NULL);	// null host name means we're a server
	m_ListenSock = INVALID_SOCKET;
	m_ConnectSock = INVALID_SOCKET;
	memset(&m_RemoteAddr, 0, sizeof(SOCKADDR_IN));
	m_Port = Port;
	m_LastError = 0;
	m_ExceptionHandler = Handler;
	m_ConnectionHandler = NULL;
	m_ConnectionHandlerArg = NULL;
	m_AmConnected = FALSE;
	m_ReadEvent.Create(NULL, TRUE, FALSE, NULL, "WTcp Read");	// manual reset
	m_ReadDoneEvent.Create(NULL, FALSE, FALSE, NULL, "WTcp Done");
	if (m_ReadEvent == NULL || m_ReadDoneEvent == NULL)
		throw WError(TCP_CANT_CREATE_EVENT);
//
// If we're a server, create a listening socket; if we're a client, connect to
// the specified server.
//
	if (m_AmServer)
		Listen(ServerIP);
	else {
		if (Port != UINT_MAX)	// if not in GetConnection
			Connect(HostName);
	}
//
// If we're a server, or we're a client that's using event-driven reads, launch
// the receive thread.  Servers ALWAYS use event-driven reads.
//
	if (m_AmServer || UsingReadEvent) {
		m_ReceiveThread.Create(NULL, 0, ReceiveThread, this, 0, NULL, "WTcp Receive");
		if (m_ReceiveThread == NULL)
			throw WError(TCP_CANT_LAUNCH_THREAD);
	}
}

WTcp::~WTcp()
{
//
// Close all sockets.  Any sockets function that's blocked on one of these
// sockets will immediately unblock, and return an error.
//
	closesocket(m_ListenSock);
	closesocket(m_ConnectSock);
	m_ReadDoneEvent.Set();	// in case receive thread is blocked on this
//
// If the receive thread was launched, kill it.  It should exit immediately,
// because we've closed both sockets and set m_ReadDoneEvent.  If it fails to
// exit in a reasonable amount of time, throw an error.
//
	if (m_ReceiveThread != NULL) {
		if (WaitForSingleObject(m_ReceiveThread, THREAD_EXIT_TIMEOUT) != WAIT_OBJECT_0)
			throw WError(TCP_CANT_KILL_THREAD);
		m_ReceiveThread.Close();
	}
}

void WTcp::Disconnect()
{
	if (!m_AmConnected)
		return;
	m_AmConnected = FALSE;	// clear flag first to avoid race condition
	closesocket(m_ConnectSock);	// client may be blocked in select
	m_ReadDoneEvent.Set();	// in case receive thread is blocked on this
}

void WTcp::InstallConnectionHandler(CONNECTION_HANDLER Handler, void *UserData)
{
	m_ConnectionHandler = Handler;
	m_ConnectionHandlerArg = UserData;
}

void WTcp::Abort(WERROR_CODE Error)
{
	m_LastError = WSAGetLastError();	// save error before doing anything else
//
// Assume we're being called during construction; when we throw the exception,
// our destructor will NOT be called, so clean up resources explicitly first.
//
	closesocket(m_ListenSock);
	closesocket(m_ConnectSock);
	throw WError(Error, m_LastError);
}

void WTcp::Listen(LPCSTR ServerIP)
{
//
// Create a listening socket.
//
	m_ListenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_ListenSock == INVALID_SOCKET)
		Abort(TCP_CANT_CREATE_SOCKET);
//
// Bind the socket to any address on the agreed-upon port.
//
	SOCKADDR_IN LocalAddr;
	LocalAddr.sin_family = AF_INET;
	LocalAddr.sin_port = htons(static_cast<u_short>(m_Port));
	LocalAddr.sin_addr.s_addr = ServerIP != NULL ? 
		inet_addr(ServerIP) : htonl(INADDR_ANY);
	if (bind(m_ListenSock, (PSOCKADDR) &LocalAddr, sizeof(LocalAddr)) == SOCKET_ERROR)
		Abort(TCP_CANT_BIND_SOCKET);
//
// Start listening for a connection.
//
	if (listen(m_ListenSock, SOMAXCONN) == SOCKET_ERROR)
		Abort(TCP_CANT_LISTEN);
}

void WTcp::WaitForData()
{
	while (1) {
//
// Block until at least one byte of data is available for reading.
//
		char	Temp;
		int Rcvd = recv(m_ConnectSock, &Temp, 1, MSG_PEEK);
		if (Rcvd == SOCKET_ERROR)
			break;
		if (!Rcvd)		// client disconnected gracefully
			break;
//
// Unblock the reader, and wait for the read to be completed.
//
		m_ReadEvent.Set();
		WaitForSingleObject(m_ReadDoneEvent, INFINITE);
	}
}

void WTcp::WaitForConnection()
{
	while (1) {
//
// Block until a connection is established, or the socket is closed.
//
		int AddrSize = sizeof(SOCKADDR);
		m_ConnectSock = accept(m_ListenSock, (PSOCKADDR)&m_RemoteAddr, &AddrSize);
		if (m_ConnectSock == INVALID_SOCKET) {
			if (WSAGetLastError() == WSAEINTR)		// socket closed
				break;
			if (WSAGetLastError() == WSAESHUTDOWN)	// socket closed
				break;
			Abort(TCP_CANT_ACCEPT);		// socket error
		}
//
// Let the user know that a connection was established.
//
		m_AmConnected = TRUE;
		if (m_ConnectionHandler != NULL) {
			(*m_ConnectionHandler)(m_AmConnected, inet_ntoa(m_RemoteAddr.sin_addr),
				m_ConnectionHandlerArg);
		}
//
// When WaitForData exits, we're disconnected, so close the socket.
//
		WaitForData();	
		closesocket(m_ConnectSock);
//
// Let the user know that the connection was dropped.
//
		m_AmConnected = FALSE;
		if (m_ConnectionHandler != NULL) {
			(*m_ConnectionHandler)(m_AmConnected, inet_ntoa(m_RemoteAddr.sin_addr),
				m_ConnectionHandlerArg);
		}
	}
}

unsigned __stdcall WTcp::ReceiveThread(void *arg)
{
	WTcp	*Tcp = (WTcp *)arg;
	try {
		if (Tcp->m_AmServer)
			Tcp->WaitForConnection();
		else
			Tcp->WaitForData();
	}
	catch (const WError& e) {
//
// If there's an exception handler, call it, passing it the exception object.
//
		if (Tcp->m_ExceptionHandler != NULL)
			(Tcp->m_ExceptionHandler)(e);
	}
	return(0);
}

void WTcp::Connect(LPCSTR HostName)
{
//
// Create a connection socket.
//
	m_ConnectSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_ConnectSock == INVALID_SOCKET)
		Abort(TCP_CANT_CREATE_SOCKET);
//
// If the host name is a domain, look up the host's IP address.
//
	m_RemoteAddr.sin_family = AF_INET;
	m_RemoteAddr.sin_port = htons(static_cast<u_short>(m_Port));
	m_RemoteAddr.sin_addr.s_addr = inet_addr(HostName);
	if (m_RemoteAddr.sin_addr.s_addr == htonl(INADDR_NONE)) {
		HOSTENT	*pHostEnt = gethostbyname(HostName);
		if (pHostEnt == NULL)
			Abort(TCP_CANT_LOOKUP_HOST);
		m_RemoteAddr.sin_addr = *((IN_ADDR *)pHostEnt->h_addr_list[0]);
	}
//
// Connect to the specified server.
//
	if (connect(m_ConnectSock, (PSOCKADDR)&m_RemoteAddr, sizeof(m_RemoteAddr)) == SOCKET_ERROR)
		Abort(TCP_CANT_CONNECT);
	m_AmConnected = TRUE;
}

DWORD WTcp::BytesAvail()
{
	DWORD	Bytes;
	if (!ioctlsocket(m_ConnectSock, FIONREAD, &Bytes))
		return(Bytes);
	return(0);
}

DWORD WTcp::Read(void *Buffer, DWORD Length, DWORD Timeout)
{
//
// If we're doing event-driven reads, block until we're signaled.  If the wait
// times out, return zero, otherwise reset the read event.
//
	if (m_ReceiveThread != NULL) {
		if (WaitForSingleObject(m_ReadEvent, Timeout) != WAIT_OBJECT_0)
			return(0);
		m_ReadEvent.Reset();
	}
	m_LastError = 0;
	DWORD	TotalRcvd = 0;
	int		ErrVal;
//
// Build a socket set for select.
//
	fd_set	SockSet;
	FD_ZERO(&SockSet);
	FD_SET(m_ConnectSock, &SockSet);
//
// Build a timeout for select.
//
	struct	timeval	BlockTime;
	BlockTime.tv_sec = Timeout / 1000;
	BlockTime.tv_usec = (Timeout % 1000) * 1000;
	while (TotalRcvd < Length) {
//
// Block until the socket is readable, or the time limit expires.
//
		ErrVal = select(0, &SockSet, NULL, NULL, &BlockTime);
		if (ErrVal == SOCKET_ERROR) {
			m_LastError = WSAGetLastError();
			break;
		}
		if (!ErrVal)	// zero means the select timed out
			break;
//
// Receive whatever portion we haven't received yet.
//
		int	Rcvd = recv(m_ConnectSock, (char *)Buffer + TotalRcvd,
			Length - TotalRcvd, 0);
		if (Rcvd == SOCKET_ERROR) {
			m_LastError = WSAGetLastError();
			break;
		}
		if (!Rcvd)		// zero means the connection was closed gracefully
			break;
		TotalRcvd += Rcvd;		// add what we actually received to the total
	}
//
// If we're doing event-driven reads, unblock the receive thread.
//
	if (m_ReceiveThread != NULL)
		m_ReadDoneEvent.Set();
	return(TotalRcvd);
}

DWORD WTcp::Write(const void *Buffer, DWORD Length, DWORD Timeout)
{
	m_LastError = 0;
	DWORD	TotalSent = 0;
	int		ErrVal;
//
// Build a socket set for select.
//
	fd_set	SockSet;
	FD_ZERO(&SockSet);
	FD_SET(m_ConnectSock, &SockSet);
//
// Build a timeout for select.
//
	struct	timeval	BlockTime;
	BlockTime.tv_sec = Timeout / 1000;
	BlockTime.tv_usec = (Timeout % 1000) * 1000;
	while (TotalSent < Length) {
//
// Block until the socket is writeable, or the time limit expires.
//
		ErrVal = select(0, NULL, &SockSet, NULL, &BlockTime);
		if (ErrVal == SOCKET_ERROR) {
			m_LastError = WSAGetLastError();
			break;
		}
		if (!ErrVal)	// zero means the select timed out
			break;
//
// Send whatever portion we haven't sent yet.
//
		int	Sent = send(m_ConnectSock, (char *)Buffer + TotalSent,
			Length - TotalSent, 0);
		if (Sent == SOCKET_ERROR) {
			m_LastError = WSAGetLastError();
			break;
		}
		TotalSent += Sent;		// add what we actually sent to the total
	}
	return(TotalSent);
}

bool WTcp::GetConnection(WTcp *&Connection, bool UseReadEvent)
{
	if (!m_AmServer || !m_AmConnected)
		return(FALSE);
	WTcp	*tcp = NULL;
	try {
		tcp = new WTcp("", UINT_MAX, NULL, UseReadEvent);	// don't try to connect
	}
	catch (const WError&) {
		return(FALSE);
	}
	tcp->m_AmServer = FALSE;	// new instance is a client
	tcp->m_Port = m_Port;
	tcp->m_ConnectSock = m_ConnectSock;	// transfer socket to new instance
	m_ConnectSock = INVALID_SOCKET;	// relinquish socket
	tcp->m_RemoteAddr = m_RemoteAddr;
	tcp->m_AmConnected = TRUE;
	Connection = tcp;	// pass instance to caller
	return(TRUE);
}
