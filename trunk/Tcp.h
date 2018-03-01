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
		03		15aug04		ck: wrap events, threads, and timers
		04		18oct04		ck: make connection handler's client address a string
		05		19oct04		ck: include sync object headers for portability
		06		28jan08		ck: make thread function a static member to fix warning
		07		09jun08		ck: add GetRemoteAddr and GetConnection
		08		12feb09		ck: add ServerIP argument to ctor

		establish a reliable network connection

*/

#ifndef TCP_INCLUDED
#define TCP_INCLUDED

#include "Sockets.h"
#include "Error.h"
#include "Event.h"
#include "Thread.h"

// The WTcp object provides a reliable, byte-oriented network connection between
// two nodes, using TCP/IP protocol.  One node must be constructed as a server,
// and the other node must be constructed as a client.  The same constructor is
// used in both cases; a server is constructed if the host name is NULL.  Both
// the server and the client must specify the same port.  Certain well-known
// ports should be avoided, as they may already be in use.
//
// Server objects always use event-driven reads.  The user can block on the
// server's read event, using WaitForSingleObject or WaitForMultipleObjects.
// The read event handle is obtained using the GetReadHandle member function.
// When the user becomes unblocked, there is at least one byte of data to be
// read.  The data is read using the Read member function.
//
// Client objects can use event-driven or polled reads, depending on whether
// the constructor's UseReadEvent argument is true (selects event-driven reads)
// or false (selects polled reads).  If polled reads are selected, the client
// must not attempt to block on the server's read event; it will never signal.
//
class WTcp : public WObject {
public:
//
// Define an exception handler that's called by the server's receive thread.
//
	typedef void (*EXCEPTION_HANDLER)(const WError& Error);
//
// Define a handler that's called when a connection is established or dropped.
//
	typedef void (*CONNECTION_HANDLER)(bool Connected,
		LPCSTR ClientAddress, void *UserData);
//
// Constructs a TCP server or client.
//
// Remarks: This constructor throws a WError exception if sockets or TCP can't
// be initialized, or if the server can't be launched.  The other public member
// functions do NOT throw exceptions.
//
	WTcp(
		LPCSTR	HostName,					// If null, the object is a server;
											// otherwise the object is a client
											// and attempts to connect to the
											// specified host.  The host can be
											// a domain name or an IP address.
		DWORD	Port,						// The agreed-upon port which both
											// the server and the client use.
		LPCSTR	ServerIP = NULL,			// For servers, the IP address to
											// bind the server to, or null to
											// let sockets assign an address.
											// Not used by clients.
		bool	UsingReadEvent = FALSE,		// For clients, true if reads will
											// be event-driven; otherwise reads
											// must poll.  Ignored for servers,
											// because server reads are always
											// event-driven.
		EXCEPTION_HANDLER Handler = NULL	// If non-null, a handler to call
											// if the receive thread throws an
											// exception.  Only meaningful for
											// servers or event-driven clients.
	);
//
// Destroys a WTcp server or client.
//
	~WTcp();
//
// Retrieves the number of bytes available for reading.
//
// Returns: The number of bytes available; zero if not connected.
//
	DWORD	BytesAvail();
//
// Reads data from the connection.
//
// Remarks: If all of the requested data is available, the function returns
// immediately.  Otherwise the function blocks and reads chunks of data in a
// loop, until all of the requested data is read, or an error or timeout
// occurs.  Note that the timeout value limits any given iteration of this
// loop, rather than the entire read; it is effectively an inter-chunk timeout.
//
// For servers, or clients doing event-driven reads, this function should only
// be called after the read event has signaled; otherwise performance will
// suffer.  Clients select event-driven or polled reads in the constructor.
//
// Returns: The number of bytes actually read.  This number can be less than
// the specified length if a timeout or error occurs; call GetLastError to
// determine the cause of the failure.
//
	DWORD	Read(
		void	*Buffer,					// Receives the read data.
		DWORD	Length,						// How many bytes to read.
		DWORD	Timeout = INFINITE			// The maximum time allowed for any
											// subset of the data to be read,
											// in milliseconds.
	);
//
// Writes data to the connection.
//
// Remarks: If the connection can accept all of the data, the function returns
// immediately.  Otherwise the function blocks and writes chunks of data in a
// loop, until all of the data is written, or an error or timeout occurs.  Note
// that the timeout value limits any given iteration of this loop, rather than
// the entire write; it is effectively an inter-chunk timeout.
//
// Returns: The number of bytes actually written.  This number can be less than
// the specified length if a timeout or error occurs; call GetLastError to
// determine the cause of the failure.
//
	DWORD	Write(
		const	void	*Buffer,			// The data to write.
		DWORD	Length,						// How many bytes to write.
		DWORD	Timeout = INFINITE			// The maximum time allowed for any
											// subset of the data to be written,
											// in milliseconds.
	);
//
// Retrieves the handle of the read event.
//
// Returns: The read event handle.
//
	WHANDLE	GetReadEvent() const;
//
// Retrieves the most recent sockets error number, if any.
//
// Remarks: This function can be called after a Read or Write fails.  If the
// function returns zero, the failure was caused by a timeout; otherwise the
// failure was caused by a sockets error.
//
// Returns: The sockets error number, or zero if there was no error.
//
	DWORD	GetLastError() const;
//
// Determines whether a connection has been established.
//
// Returns: True if connected.
//
	bool	Connected() const;
//
// Determines whether we're a server.
//
// Returns: True if we're a server.
//
	bool	Server() const;
//
// Installs a handler to call when a connection is established or dropped.
//
// Remarks: The handler will be passed a boolean that's true if the client is
// connected, the client's IP address, and the user-defined pointer that was 
// passed to this installer.  This function is only meaningful for servers.
//
	void	InstallConnectionHandler(
		CONNECTION_HANDLER	Handler,	// Pointer to the handler; if NULL,
										// no handler will be called.
		void	*UserData = NULL		// An optional user-defined pointer
										// that gets passed to the handler;
										// useful for giving the handler a
										// copy of your "this".
	);
//
// Breaks the current connection, if any.
//
	void	Disconnect();
//
// Retrieves the remote IP address.
//
// Remarks: Works for both clients and servers, but only while connected.
//
// Returns: The IP address.
//
	SOCKADDR_IN	GetRemoteAddr() const;
//
// Retrieves the server's current connection as a new WTcp object.
//
// Remarks: A server's connection handler can call this function to create a
// new WTcp object for the current connection.  If the function succeeds, the
// new WTcp object is returned to the handler, and the server resumes listening
// for connections as soon as the handler exits.  The handler typically creates
// a new thread, and then passes the new WTcp object to the thread.  The new WTcp
// object is dynamically allocated, and the caller is responsible for deleting
// it when it's no longer needed.  This function should only be called from a
// connection handler, and only when a new connection is made.
//
// Returns: True if successful; false if the WTcp object could not be created.
//
	bool	GetConnection(
		WTcp	*&Connection,			// Receives a pointer to the new WTcp
										// object, which is created on the heap.
		bool	UseReadEvent = FALSE	// True if reads should be event-driven.
	);

protected:
	enum {
		THREAD_EXIT_TIMEOUT = 3000	// How long we'll wait for the receive
									// thread to exit, in milliseconds.
	};
	bool	m_AmServer;				// True if we're a server; false if
									// we're a client.
	WThread	m_ReceiveThread;		// In event-driven mode, the receive
									// thread's handle; otherwise NULL.
	WEvent	m_ReadEvent;			// In event-driven mode, will be signaled
									// when there's at least one byte to read.
									// Manually reset by the Read function.
	WEvent	m_ReadDoneEvent;		// In event-driven mode, the receive thread
									// blocks on this while waiting for the
									// user to read the data.
	WSockets	m_Sockets;			// This object initializes and cleans up
									// the windows sockets layer as needed.
	SOCKET	m_ListenSock;			// In event-driven mode, the receive thread
									// listens on this socket.
	SOCKET	m_ConnectSock;			// This socket is used for the connection,
									// by both servers and clients.
	SOCKADDR_IN	m_RemoteAddr;		// If we're connected, the remote address.
	DWORD	m_Port;					// Whatever port both the server and the
									// client agree to use.
	DWORD	m_LastError;			// The last sockets error resulting from
									// a read or write, or zero if no error.
	EXCEPTION_HANDLER	m_ExceptionHandler;	// The receive thread catches its
									// own exceptions; if an exception occurs
									// in the receive thread, and this handler 
									// is non-null, the handler will be called 
									// and passed the exception object.
	CONNECTION_HANDLER	m_ConnectionHandler;	// If non-null, the server calls 
									// this handler whenever a connection is
									// established or dropped.
	void	*m_ConnectionHandlerArg;	// The connection handler is passed this
									// value; it's set by the install function.
	volatile bool	m_AmConnected;	// True if we're currently connected.

// Throws an exception for a sockets error.
//
// Remarks: The error's parameter will be the sockets error number.
//
	void	Abort(
		WERROR_CODE	Error			// The error code, as defined in the
									// system error handler.
	);
//
// Creates a listening socket.
//
// Remarks: This function is called by servers only.
//
	void	Listen(
		LPCSTR	ServerIP			// The IP address to bind the server to, or
									// null to let sockets assign an address.
	);
//
// Waits for a client to connect.
//
// Remarks: This function is called by servers only.
//
	void	WaitForConnection();
//
// Waits for at least one byte of data to arrive.
//
// Remarks: This function can be called by servers or event-driven clients.
//
	void	WaitForData();
//
// Connects to a server.
//
// Remarks: This function is called by clients only.
//
	void	Connect(
		LPCSTR	HostName			// The server's domain name or IP address.
	);
	static	unsigned __stdcall ReceiveThread(void *arg);
};

inline WHANDLE WTcp::GetReadEvent() const
{
	return(m_ReadEvent);
}

inline DWORD WTcp::GetLastError() const
{
	return(m_LastError);
}

inline bool	WTcp::Connected() const
{
	return(m_AmConnected);
}

inline bool	WTcp::Server() const
{
	return(m_AmServer);
}

inline SOCKADDR_IN WTcp::GetRemoteAddr() const
{
	return(m_RemoteAddr);
}

#endif
