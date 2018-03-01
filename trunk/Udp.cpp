// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda

		rev		date		comments
		00		12jan01		initial version
		01		02feb04		ck: add optional host IP argument to ctor
		02		18oct04		ck: add get return address string
		03		28jan08		ck: static cast htons arg to fix type warnings

		send and receive datagrams

*/

#include "stdafx.h"
#include "Udp.h"

WUdp::WUdp(DWORD ReceivePort, LPCSTR HostIP)
{
	memset(&m_ReturnAddress, 0, sizeof(SOCKADDR_IN));
	m_LastError = 0;
//
// Create a socket.
//
	m_DgSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_DgSock == INVALID_SOCKET)
		Abort(UDP_CANT_CREATE_SOCKET);
//
// Set the socket to non-blocking mode.
//
	ULONG	Polling = TRUE;
	if (ioctlsocket(m_DgSock, FIONBIO, &Polling) == SOCKET_ERROR)
		Abort(UDP_CANT_SET_MODE);
//
// Bind the socket to an address.
//
	SOCKADDR_IN	LocalAddr;
	LocalAddr.sin_family = AF_INET;
	LocalAddr.sin_port = htons(static_cast<u_short>(ReceivePort));
	LocalAddr.sin_addr.s_addr = HostIP != NULL ? inet_addr(HostIP) : htonl(INADDR_ANY);
	if (bind(m_DgSock, (PSOCKADDR)&LocalAddr, sizeof(LocalAddr)) == SOCKET_ERROR)
		Abort(UDP_CANT_BIND_SOCKET);
}

WUdp::~WUdp()
{
	if (m_DgSock != INVALID_SOCKET)
		closesocket(m_DgSock);
}

void WUdp::Abort(WERROR_CODE Error)
{
	m_LastError = WSAGetLastError();
	throw WError(Error, m_LastError);
}

DWORD WUdp::Write(SOCKADDR_IN Address, const void *Buffer, DWORD Length, DWORD Timeout)
{
	m_LastError = 0;
//
// Build a socket set for select.
//
	fd_set	SockSet;
	FD_ZERO(&SockSet);
	FD_SET(m_DgSock, &SockSet);
//
// Build a timeout for select.
//
	struct	timeval	BlockTime;
	BlockTime.tv_sec = Timeout / 1000;
	BlockTime.tv_usec = (Timeout % 1000) * 1000;
//
// Block until the socket is writable, or the time limit expires.
//
	int ErrVal = select(0, NULL, &SockSet, NULL, &BlockTime);
	if (ErrVal == SOCKET_ERROR) {
		m_LastError = WSAGetLastError();
		return(0);
	}
	if (!ErrVal)		// zero means the select timed out
		return(0);
//
// Send the datagram.
//
	DWORD	Sent = sendto(m_DgSock, (char *)Buffer, Length, 0,
		(PSOCKADDR)&Address, sizeof(SOCKADDR_IN));
	if (Sent < Length)
		m_LastError = WSAGetLastError();
	return(Sent);
}

DWORD WUdp::Read(void *Buffer, DWORD Length, DWORD Timeout)
{
	m_LastError = 0;
//
// Build a socket set for select.
//
	fd_set	SockSet;
	FD_ZERO(&SockSet);
	FD_SET(m_DgSock, &SockSet);
//
// Build a timeout for select.
//
	struct	timeval	BlockTime;
	BlockTime.tv_sec = Timeout / 1000;
	BlockTime.tv_usec = (Timeout % 1000) * 1000;
//
// Block until the socket is readable, or the time limit expires.
//
	int ErrVal = select(0, &SockSet, NULL, NULL, &BlockTime);
	if (ErrVal == SOCKET_ERROR) {
		m_LastError = WSAGetLastError();
		return(0);
	}
	if (!ErrVal)		// zero means the select timed out
		return(0);
//
// Receive the datagram.
//
	int AddrSize = sizeof(SOCKADDR);
	DWORD	Rcvd = recvfrom(m_DgSock, (char *)Buffer, Length, 0,
		(PSOCKADDR)&m_ReturnAddress, &AddrSize);
	if (Rcvd < Length)
		m_LastError = WSAGetLastError();
	return(Rcvd);
}

DWORD WUdp::Reply(const void *Buffer, DWORD Length, DWORD Timeout)
{
	return(Write(m_ReturnAddress, Buffer, Length, Timeout));
}

DWORD WUdp::Broadcast(DWORD Port, const void *Buffer, DWORD Length, DWORD Timeout)
{
	m_LastError = 0;
//
// Enable broadcast on the socket.
//
	BOOL	Broad = TRUE;
	if (setsockopt(m_DgSock, SOL_SOCKET, SO_BROADCAST, (char *)&Broad, sizeof(BOOL)) == SOCKET_ERROR) {
		m_LastError = WSAGetLastError();
		return(0);
	}
//
// Use Write to broadcast the datagram.
//
	SOCKADDR_IN	Address;
	Address.sin_family = AF_INET;
	Address.sin_port = htons(static_cast<u_short>(Port));
	Address.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	DWORD	BytesSent = Write(Address, Buffer, Length, Timeout);
//
// Disable broadcast on the socket.
//
	Broad = FALSE;
	if (setsockopt(m_DgSock, SOL_SOCKET, SO_BROADCAST, (char *)&Broad, sizeof(BOOL)) == SOCKET_ERROR) {
		m_LastError = WSAGetLastError();
		return(0);
	}
	return(BytesSent);
}

SOCKADDR_IN WUdp::GetReturnAddress() const
{
	return(m_ReturnAddress);
}

LPCSTR WUdp::GetReturnAddressString() const
{
	return(inet_ntoa(m_ReturnAddress.sin_addr));
}
