// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		12feb09	in Create, add ServerIP

		connection for exchanging packets
 
*/

#ifndef CPACKETCONN_INCLUDED
#define CPACKETCONN_INCLUDED

#include "Tcp.h"

// Unicode-compatible version of inet_addr
#ifdef UNICODE
#define _tinet_addr winet_addr
unsigned long winet_addr(const WCHAR *cp);
#else	// MBCS
#define _tinet_addr inet_addr
#endif

typedef	struct tagPACKET_HDR {	// base struct for all packets
	UINT	Message;	// message ID; see enum above
	UINT	DataLen;	// size of packet's payload, in bytes
} PACKET_HDR;

typedef	struct tagGENERIC_PACKET : public PACKET_HDR {
	BYTE	Data[1];	// variable-size payload
} GENERIC_PACKET;

class CPacketConn : public WObject {
public:
// Construction
	CPacketConn();
	~CPacketConn();
	bool	Create(LPCTSTR HostName, DWORD Port, LPCTSTR ServerIP, HWND hWnd, int Cookie, WTcp *Tcp = NULL);
	bool	Destroy();

// Attributes
	bool	IsConnected() const;
	bool	IsServer() const;
	CString	GetClientAddress() const;
	CString	GetLastErrorString() const;
	SOCKADDR_IN	GetRemoteAddr() const;

// Operations
	bool	Write(const PACKET_HDR& Packet, DWORD Length, DWORD Timeout = INFINITE);
	GENERIC_PACKET	*Read(UINT Message, int Cookie, DWORD Timeout = INFINITE);
	void	Disconnect();

protected:
// Constants
	enum {
		AUTH_QUERY = 0x6c616e41,	// authentication query (Anal)
		AUTH_REPLY = 0x65726157,	// authentication reply (Ware)
		PROTO_VERSION = 1,			// protocol version
		AUTH_TIMEOUT = 1000,		// authentication timeout, in milliseconds
		THREAD_EXIT_TIMEOUT = 3000,	// thread exit timeout, in milliseconds
	};
	static const PACKET_HDR m_AuthQuery;	// authentication query packet
	static const PACKET_HDR m_AuthReply;	// authentication reply packet

// Member data
	WTcp	*m_Tcp;		// pointer to TCP object
	WThread	m_RcvThr;	// receive thread handle
	HWND	m_hWnd;		// notification target window
	int		m_Cookie;	// user-defined message parameter
	CString	m_ClientAddr;	// if server and connected, client's IP address
	CString	m_LastErrorStr;	// string describing last TCP error
	bool	m_Server;	// true if we're a server
	bool	m_Kill;		// true if shutting down server
	bool	m_Trusted;	// true if authentication is complete
	bool	m_Passive;	// true if server initiated connection

// Helpers
	void	RcvLoop();
	bool	RcvPacket();
	bool	Authenticate();
	bool	TcpRead(void *Buffer, DWORD Length, DWORD Timeout = INFINITE);
	bool	TcpWrite(const void *Buffer, DWORD Length, DWORD Timeout = INFINITE);
	static	unsigned __stdcall RcvThrFunc(PVOID arg);
	static	void	ConnHand(bool Connected, LPCSTR ClientAddr, void *UserData);
};

inline bool CPacketConn::IsConnected() const
{
	return(m_Tcp != NULL && m_Tcp->Connected());
}

inline bool CPacketConn::IsServer() const
{
	return(m_Server);
}

inline bool CPacketConn::TcpRead(void *Buffer, DWORD Length, DWORD Timeout)
{
	return(m_Tcp->Read(Buffer, Length, Timeout) == Length);
}

inline bool CPacketConn::TcpWrite(const void *Buffer, DWORD Length, DWORD Timeout)
{
	return(m_Tcp->Write(Buffer, Length, Timeout) == Length);
}

inline bool CPacketConn::Write(const PACKET_HDR& Packet, DWORD Length, DWORD Timeout)
{
	return(TcpWrite(&Packet, Length, Timeout));
}

inline CString CPacketConn::GetClientAddress() const
{
	return(m_ClientAddr);
}

inline CString CPacketConn::GetLastErrorString() const
{
	return(m_LastErrorStr);
}

inline void CPacketConn::Disconnect()
{
	m_Tcp->Disconnect();
}

inline SOCKADDR_IN CPacketConn::GetRemoteAddr() const
{
	return(m_Tcp->GetRemoteAddr());
}

#endif
