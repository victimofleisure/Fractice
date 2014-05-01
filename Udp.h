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

		send and receive datagrams

*/

#ifndef WUDP_INCLUDED
#define WUDP_INCLUDED

#include "Sockets.h"
#include "Error.h"

// The WUdp object allows data to be written, read, replied to, and broadcast
// over a network, using User Datagram Protocol.  UDP is a connectionless,
// unreliable protocol; datagrams are not guaranteed to arrive intact or in any
// particular order.
//
// Note that there is a size limit on datagrams, imposed by the underlying
// sockets layer.  Any portion of a datagram that exceeds the limit will be
// quietly discarded.  The limit can be retrieved using the GetMaximumDatagram
// member function.
//
class WUdp : public WObject {
public:
//
// Constructs a WUdp node.
//
// Remarks: This constructor throws a WError exception if sockets or UDP can't
// be initialized.  The other public member functions do NOT throw exceptions.
//
	WUdp(
		DWORD	ReceivePort = 0,	// If non-zero, specifies a particular
									// port to receive on; otherwise a port
									// is assigned automatically.
		LPCSTR	HostIP = NULL		// If non-null, a host IP address to use;
									// otherwise sockets obtains the host IP.
	);
//
// Destroys a WUdp node.
//
	~WUdp();
//
// Sends a datagram to a specific address.
//
// Returns: The number of bytes actually sent.  This number can be less than
// the specified length if a timeout or an error occurs.  Zero indicates a
// timeout; otherwise the cause is a sockets error, or a truncated datagram.
// Call GetLastError to retrieve the error information.
//
	DWORD	Write(
		SOCKADDR_IN	Address,		// The address to send to.
		const	void *Buffer,		// The source buffer.
		DWORD	Length, 			// The number of bytes to write.
		DWORD	Timeout = INFINITE	// Maximum time allowed for the write
									// to complete, in milliseconds.
	);
//
// Receives a datagram.
//
// Remarks: This function sets the return address; subsequent replies will go
// to the address from which this datagram was received.
//
// Returns: The number of bytes actually received.  This number can be less
// than the specified length if a timeout or an error occurs.  Zero indicates a
// timeout; otherwise the cause is a sockets error, or a truncated datagram.
// Call GetLastError to retrieve the error information.
//
	DWORD	Read(
		void	*Buffer,			// The destination buffer.
		DWORD	Length,				// The number of bytes to read.
		DWORD	Timeout = INFINITE	// Maximum time allowed for the read
									// to complete, in milliseconds.
	);
//
// Replies to a datagram.
//
// Remarks: The datagram is sent to the current return address, which is the
// address from which a datagram was most recently received.  To send to a
// different address, use the Write member function.
//
// Returns: The number of bytes actually sent.  This number can be less than
// the specified length if a timeout or an error occurs.  Zero indicates a
// timeout; otherwise the cause is a sockets error, or a truncated datagram.
// Call GetLastError to retrieve the error information.
//
	DWORD	Reply(
		const	void *Buffer,		// The source buffer.
		DWORD	Length, 			// The number of bytes to write.
		DWORD	Timeout = INFINITE	// Maximum time allowed for the reply
									// to complete, in milliseconds.
	);
//
// Broadcasts a datagram.
//
// Remarks: A broadcast datagram will be received by any and all users on the
// broadcaster's port.  The broadcaster must prepare for unexpected replies.
//
// Returns: The number of bytes actually sent.  This number can be less than
// the specified length if a timeout or an error occurs.  Zero indicates a
// timeout; otherwise the cause is a sockets error, or a truncated datagram.
// Call GetLastError to retrieve the error information.
//
	DWORD	Broadcast(
		DWORD	Port,				// The port to broadcast on.
		const	void *Buffer,		// The source buffer.
		DWORD	Length, 			// The number of bytes to broadcast.
		DWORD	Timeout = INFINITE	// Maximum time allowed for the broadcast
									// to complete, in milliseconds.
	);
//
// Retrieves the current return address.
//
// Remarks: This is the address from which a datagram was most recently
// received.
//
// Returns: The address.
//
	SOCKADDR_IN	GetReturnAddress() const;
//
// Retrieves the current return address, as a string.
//
// Remarks: This is the address from which a datagram was most recently
// received.  Note that this string is only guaranteed to be valid until the
// next sockets call within the same thread.
//
// Returns: The address string.
//
	LPCSTR	GetReturnAddressString() const;
//
// Retrieves the most recent sockets error number, if any.
//
// Remarks: This function can be called after any sockets operation fails.  If
// the function returns zero, the failure was caused by a timeout or truncated
// datagram.
//
// Returns: The sockets error number, or zero if there was no error.
//
	DWORD	GetLastError() const;
//
// Retrieve the maximum size of a datagram.
//
// Returns: The maximum datagram size, in bytes.
//
	DWORD	GetMaxDatagram() const;

protected:
	WSockets	m_Sockets;			// This object initializes and cleans up
									// the windows sockets layer as needed.
	SOCKET	m_DgSock;				// The datagram socket.
	SOCKADDR_IN m_ReturnAddress;	// The address from which we've most
									// recently received a datagram.
	DWORD	m_LastError;			// The last sockets error resulting from
									// a read or write, or zero if no error.

// Throws an exception for a sockets error.
//
// Remarks: The error's parameter will be the sockets error number.
//
	void	Abort(
		WERROR_CODE	Error			// The error code, as defined in the
									// system error handler.
	);
};

inline DWORD WUdp::GetLastError() const
{
	return(m_LastError);
}

inline DWORD WUdp::GetMaxDatagram() const
{
	return(m_Sockets.GetData().iMaxUdpDg);
}

#endif
