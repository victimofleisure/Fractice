// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda

		rev		date		comments
        00      14feb08	initial version

		locate named servers on a network

*/

#ifndef WFINDSERVERS_INCLUDED
#define WFINDSERVERS_INCLUDED

#include <winsock.h>

class WUdp;
class WThread;

#pragma pack(1)		// pack structures on 1-byte boundaries

class WFindServers : public WObject {
public:
	enum {
		MAX_SERVER_NAME	= 128,	// The maximum size of a server name.
		MAX_IP_ADDRESS	= 32,	// The maximum size of an IP address.
	};
	// Define the server extended status information; this structure is APPEND ONLY.
	typedef	struct {
		USHORT	StructSize;			// size of this structure in bytes
		in_addr	ClientIPAddress;	// if server is connected, client's IP
									// address in host order, else zero
	} EXT_STATUS;
	// Define the server information; this structure is APPEND ONLY.
	typedef struct {
		char	Name[MAX_SERVER_NAME + 1];		// The server's name.
		char	IPAddress[MAX_IP_ADDRESS + 1];	// The server's IP address.
		DWORD	Status;							// The server's user-defined status;
												// in this implementation, non-zero if
												// the server has a connection.
		EXT_STATUS	ExtStatus;					// The server's extended status.
	} SERVER_INFO;
//
// Define a function that the server calls to obtain status information.
//
	typedef DWORD (*STATUS_CALLBACK)(LPCSTR ClientAddress, void *UserData,
		EXT_STATUS& ExtStatus);
//
// Constructs a name server or client.
//
// Remarks: This constructor throws a WError exception if sockets or UDP can't
// be initialized, or if the server can't be launched.
//
	WFindServers(
		LPCSTR	ServerName,		// If non-null, construct a server with this
								// name; otherwise construct a client.
		DWORD	PortNumber,		// Identifies which port server listens on.
		LPCSTR	HostIP = NULL	// If non-null, a host IP address to use;
								// otherwise sockets obtains the host IP.
	);
//
// Destroys a name server or client.
//
	~WFindServers();
//
// Finds one or more servers.
//
// Remarks: To find all servers, call this function in a loop until it returns
// false.  Index be must zero for the loop's first call, and non-zero for the
// loop's subsequent calls.  To find a server with a specific name, specify a
// non-null MatchName argument.  If a MatchName argument is specified, and all
// servers have unique names, it is acceptable to call this function just once,
// with an Index of zero.  This function should be called by clients only.
//
// Returns: True if a server was found.
//
	bool	Find(
		DWORD	Index,				// Must be zero on the first call, and
									// non-zero for each subsequent call.
		SERVER_INFO& ServerInfo,	// If the function succeeds, a server's
									// name and address are returned here.
		LPCSTR	MatchName = NULL	// If non-null, only servers with the
									// specified name will respond.
	);
//
// Installs a status callback function.
//
// Remarks: The specified callback function is called by the server whenever it
// replies to a client's REQUEST_IP record.  The callback function returns a
// user-defined status, which is passed to client, as part of the SUPPLY_IP.
// The callback is passed the client's IP address, and the user-defined pointer
// that was passed to this installer.
//
	void	InstallStatusCallback(
		STATUS_CALLBACK StatusCallback,		// The function to call; if NULL,
											// no function will be called.
		void	*UserData = NULL			// An optional user-defined pointer
											// that gets passed to the callback
											// function; useful for giving the
											// function a copy of your "this".
	);
//
// Requests a reply from a server.
//
// Remarks: To read the reply, call the FindServer function with the Index
// argument set to a non-zero value.  It's possible to request replies from
// multiple servers, and then read their replies all at once, but the network
// stack could run out of room and drop some of the replies.  To avoid this,
// read the replies from a separate thread, so that they don't pile up.
//
// Returns: True if successful.
//
	bool	RequestReply(
		SOCKADDR_IN	IPAddress		// The server's IP address.
	);

private:
	enum {
		VERSION_CODE			= 0x0101,	// The protocol version; major in
											// high byte, minor in low byte.
		THREAD_EXIT_TIMEOUT		= 3000,		// Maximum time for thread to exit,
											// in milliseconds.
		SERVER_READ_TIMEOUT		= 1000,		// Maximum time for a server read.
		SERVER_WRITE_TIMEOUT	= 1000,		// Maximum time for a server write.
		CLIENT_READ_TIMEOUT		= 1000,		// Maximum time for a client read.
		CLIENT_WRITE_TIMEOUT	= 1000,		// Maximum time for a client write.
	};
	union	RECORD;				// Needed because MAX_RECORD is private.
	friend	RECORD;				// Needed because MAX_RECORD is private.
	enum {
		MAX_RECORD	= 256		// The maximum size of a record.  This must be
								// an even number, and small enough so that the
								// total packet size does not exceed the
								// maximum UDP datagram.
	};
	enum RECTYPE {
		REQUEST_IP,				// Broadcast by a client; one or more servers
								// may reply with their names and addresses.
								// The payload is a SERVER_INFO structure, of
								// which only the name field is used.  If the
								// name field contains a non-null string, only
								// servers with matching names should reply.
		SUPPLY_IP				// Sent by a server, in reply to a REQUEST_IP.
								// The payload is a SERVER_INFO structure,
								// containing the server's name and address.
	};
	typedef union RECORD {
		SERVER_INFO	ServerInfo;			// Contains a server name and address.
		char	MaxRecord[MAX_RECORD];	// Records must not exceed this size.
	};
	typedef struct {
		DWORD	IdString;		// Identifies the packet.
		USHORT	Version;		// The protocol version; major in
								// high byte, minor in low byte.
		USHORT	RecordType;		// The record type.
		RECORD	Record;			// The record data; its definition
								// depends on the record type.
		USHORT	Checksum;		// 16-bit XOR checksum calculated
								// on the entire record.
	} PACKET;
	enum {
		PACKET_SIZE = sizeof(PACKET)	// Actual size of a packet.
	};
	bool	m_AmServer;				// True if we're a server.
	DWORD	m_PortNumber;			// If we're a server, port to receive on.
	WUdp	*m_Udp;					// The datagram object.
	WThread	*m_ServerThread;		// If we're a server, our thread handle.
	char	m_ServerName[MAX_SERVER_NAME + 1];	// If we're a server, our name.
	STATUS_CALLBACK	m_StatusCallback;	// If we're a server, and this is non-null,
									// it points to a function we'll call when
									// we reply to a REQUEST_IP.  The function
									// returns a user-defined status that we
									// include in our reply.
	void	*m_StatusCallbackArg;	// The callback receives this pointer as
									// argument; it's set by install function.

// Checks a packet to make sure it's correctly formatted.
//
// Returns: True if the packet is valid.
//
	bool	CheckPacket(
		const	PACKET& Packet		// The packet to check.
	) const;
//
// Assembles a packet for transmission.
//
	void	MakePacket(
		RECTYPE	RecordType,			// The record type.
		const	RECORD&	Record,		// The record data.
		PACKET& Packet				// Receives the assembled packet.
	) const;
//
// The server's main loop.
//
	void	ServerLoop();
	static	unsigned __stdcall ServerThread(void *arg);
};

#pragma pack()		// restore default structure packing

#endif
