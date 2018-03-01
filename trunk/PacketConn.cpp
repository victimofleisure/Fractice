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

#include "stdafx.h"
#include "PacketConn.h"
#include "ArrayEx.h"

// Unicode-compatible version of inet_addr
#ifdef UNICODE
#include <atlconv.h>	// ATL string conversion macros
unsigned long winet_addr(const WCHAR *cp)
{
	USES_CONVERSION;
	return(inet_addr(W2CA(cp)));	// convert to MBCS
}
#endif

const PACKET_HDR CPacketConn::m_AuthQuery = {
	AUTH_QUERY,
	PROTO_VERSION
};

const PACKET_HDR CPacketConn::m_AuthReply = {
	AUTH_REPLY,
	PROTO_VERSION
};

CPacketConn::CPacketConn()
{
	m_Tcp = NULL;
	m_hWnd = NULL;
	m_Cookie = 0;
	m_Server = FALSE;
	m_Kill = FALSE;
	m_Trusted = FALSE;
	m_Passive = FALSE;
}

CPacketConn::~CPacketConn()
{
	Destroy();
}

bool CPacketConn::Create(LPCTSTR HostName, DWORD Port, LPCTSTR ServerIP, HWND hWnd, int Cookie, WTcp *Tcp)
{
	ASSERT(hWnd);
	Destroy();
	if (Tcp != NULL) {
		m_Tcp = Tcp;	// take ownership of TCP object
		m_Passive = TRUE;
	} else {
		try {	// TCP constructor throws
#ifdef UNICODE
			USES_CONVERSION;
			m_Tcp = new WTcp(W2CA(HostName), Port, W2CA(ServerIP));	// convert to MBCS
#else
			m_Tcp = new WTcp(HostName, Port, ServerIP);
#endif
		}
		catch (const WError& e) {
			m_LastErrorStr = e.what();
			return(FALSE);
		}
	}
	m_hWnd = hWnd;
	m_Cookie = Cookie;
	m_Server = m_Tcp->Server();
	m_Kill = FALSE;
	m_Trusted = FALSE;
	m_ClientAddr = HostName;
	if (!m_RcvThr.Create(NULL, 0, RcvThrFunc, this, 0, NULL))
		return(FALSE);
	if (m_Server)
		m_Tcp->InstallConnectionHandler(ConnHand, this);
	else {	// we're a client
		if (!m_Passive) {
			if (!Write(m_AuthQuery, sizeof(m_AuthQuery))) {	// query server
				Destroy();
				return(FALSE);
			}
		}
		if (WaitForSingleObject(m_Tcp->GetReadEvent(), AUTH_TIMEOUT) != WAIT_OBJECT_0) {
			Destroy();
			return(FALSE);	// server didn't reply in time
		}
	}
	return(TRUE);
}

bool CPacketConn::Destroy()
{
	if (m_Tcp != NULL) {
		if (m_Server) {
			m_Kill = TRUE;
			SetEvent(m_Tcp->GetReadEvent());
		} else
			m_Tcp->Disconnect();	// causes Read to return
		WaitForSingleObject(m_RcvThr, THREAD_EXIT_TIMEOUT);
		try {
			delete m_Tcp;	// destructor throws
		}
		catch (const WError& e) {
			m_Tcp = NULL;
			m_LastErrorStr = e.what();
			return(FALSE);
		}
		m_Tcp = NULL;
	}
	return(TRUE);
}

bool CPacketConn::Authenticate()
{
	PACKET_HDR	hdr;
	if (TcpRead(&hdr, sizeof(hdr))) {
		if (m_Server || m_Passive) {
			if (hdr.Message == AUTH_QUERY && hdr.DataLen <= PROTO_VERSION) {
				if (!Write(m_AuthReply, sizeof(hdr)))	// reply to client
					return(FALSE);
				if (m_Passive)
					SetEvent(m_Tcp->GetReadEvent());	// unblock Create
				return(TRUE);	// client is trustworthy
			}
		} else {	// we're a client; assume server has replied to our query
			if (hdr.Message == AUTH_REPLY && hdr.DataLen <= PROTO_VERSION) {
				SetEvent(m_Tcp->GetReadEvent());	// unblock Create
				return(TRUE);	// server is trustworthy
			}
		}
	}
	m_Tcp->Disconnect();
	return(FALSE);	// authentication failed
}

bool CPacketConn::RcvPacket()
{
	if (!m_Trusted) {	// if not authenticated yet
		m_Trusted = Authenticate();
		// if we're a client and connection was bogus, end receive loop
		// if we're a server, there could be new clients, so keep looping
		return(m_Trusted || m_Server);
	}
	PACKET_HDR	hdr;
	if (!TcpRead(&hdr, sizeof(hdr)))	// read header
		return(FALSE);
	UINT	PackLen = sizeof(hdr) + hdr.DataLen;
	// allocate packet on heap; recipient is responsible for deleting packet
	GENERIC_PACKET	*pp = (GENERIC_PACKET *)new BYTE[PackLen];
	if (hdr.DataLen) {
		if (!TcpRead(pp->Data, hdr.DataLen))	// read data
			return(FALSE);
	}
	PACKET_HDR	*pHdr = pp;	// upcast to base class
	*pHdr = hdr;
	PostMessage(m_hWnd, UWM_RCVPACKET, (WPARAM)pp, m_Cookie);	// post packet to window
	return(TRUE);
}

void CPacketConn::RcvLoop()
{
	if (m_Server) {
		do {
			WaitForSingleObject(m_Tcp->GetReadEvent(), INFINITE);
		} while (!m_Kill && RcvPacket());
	} else {
		PostMessage(m_hWnd, UWM_TCPCONNECT, TRUE, m_Cookie);
		while (RcvPacket()) {
			;
		}
		m_Tcp->Disconnect();
		PostMessage(m_hWnd, UWM_TCPCONNECT, FALSE, m_Cookie);
	}
}

unsigned __stdcall CPacketConn::RcvThrFunc(PVOID arg)
{
	CPacketConn	*This = (CPacketConn *)arg;
	This->RcvLoop();
	return(0);
}

void CPacketConn::ConnHand(bool Connected, LPCSTR ClientAddr, void *UserData)
{
	CPacketConn	*This = (CPacketConn *)UserData;
	This->m_Trusted = FALSE;
	This->m_ClientAddr = ClientAddr;
	PostMessage(This->m_hWnd, UWM_TCPCONNECT, Connected, This->m_Cookie);
}

GENERIC_PACKET *CPacketConn::Read(UINT Message, int Cookie, DWORD Timeout)
{
	CArrayEx<MSG, MSG&>	Reject;
	GENERIC_PACKET	*pPacket = NULL;
	DWORD	Start = GetTickCount();	// milliseconds since system started
	DWORD	Elapsed = 0;
	while (Elapsed < Timeout) {
		DWORD	Remaining = Timeout - Elapsed;
		DWORD	retc = MsgWaitForMultipleObjects(0, NULL, FALSE, 
			Remaining, QS_POSTMESSAGE);	// wait for posted message
		if (retc == WAIT_OBJECT_0) {	// if we have a posted message
			MSG	msg;	// if message is a received packet, dequeue it
			if (PeekMessage(&msg, m_hWnd, UWM_RCVPACKET, UWM_RCVPACKET, PM_REMOVE)) {
				GENERIC_PACKET	*pp = (GENERIC_PACKET *)msg.wParam;
				// if dequeued packet matches caller's specifications
				if (pp->Message == Message && Cookie == msg.lParam) {
					pPacket = pp;	// success
					break;	// end loop
				} else	// caller doesn't want this packet
					Reject.Add(msg);	// store it and repost it later
			}
		} else	// wait timeout or error
			break;	// abort loop
		DWORD	Now = GetTickCount();
		if (Now >= Start)	// usual case
			Elapsed = Now - Start;
		else	// assume tick count wrapped around to zero
			Elapsed = Now + (UINT_MAX - Start);
	}
	// repost any packets that were dequeued and rejected
	for (int i = 0; i < Reject.GetSize(); i++) {
		MSG&	rm = Reject[i];
		PostMessage(m_hWnd, rm.message, rm.wParam, rm.lParam);
	}
	return(pPacket);	// caller is responsible for deleting packet
}
