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

#include "stdafx.h"
#include "FindServers.h"
#include <process.h>
#include "Thread.h"
#include "Udp.h"

static DWORD ID_STRING = 0x6c616e41;	// Anal

WFindServers::WFindServers(LPCSTR ServerName, DWORD PortNumber, LPCSTR HostIP)
{
	m_AmServer = (ServerName != NULL);	// non-null name means we're a server
	m_PortNumber = PortNumber;
	m_Udp = NULL;
	m_ServerName[0] = 0;
	m_StatusCallback = NULL;
	m_StatusCallbackArg = NULL;
//
// If we're a server, we have to listen on a specific port; otherwise we can
// take whatever port we get.
//
	m_Udp = new WUdp(m_AmServer ? PortNumber : 0, HostIP);
	m_ServerThread = new WThread;
	if (m_AmServer) {
		strncpy(m_ServerName, ServerName, MAX_SERVER_NAME);
		m_ServerThread->Create(NULL, 0, ServerThread, this, 0, NULL, "WFindServers");
		if (*m_ServerThread == NULL)
			throw WError(FIND_CANT_LAUNCH_THREAD);
	}
}

WFindServers::~WFindServers()
{
	delete m_Udp;
	if (*m_ServerThread != NULL) {
		if (WaitForSingleObject(*m_ServerThread, THREAD_EXIT_TIMEOUT) != WAIT_OBJECT_0)
			throw WError(FIND_CANT_KILL_THREAD);
		m_ServerThread->Close();
	}
	delete m_ServerThread;
}

bool WFindServers::CheckPacket(const PACKET& Packet) const
{
	if (Packet.IdString != ID_STRING)
		return(FALSE);
	if (Packet.Version != VERSION_CODE)
		return(FALSE);
	USHORT	cs = 0;
	USHORT	*p = (USHORT *)&Packet.Record;
	for (int i = 0; i < MAX_RECORD / 2 - 1; i++)
		cs ^= p[i];
	if (cs != Packet.Checksum)
		return(FALSE);
	return(TRUE);
}

void WFindServers::MakePacket(RECTYPE RecordType, const RECORD &Record, PACKET& Packet) const
{
	Packet.IdString = ID_STRING;
	Packet.Version = VERSION_CODE;
	Packet.RecordType = (USHORT)RecordType;
	Packet.Record = Record;
	USHORT	cs = 0;
	USHORT	*p = (USHORT *)&Packet.Record;
	for (int i = 0; i < MAX_RECORD / 2 - 1; i++)
		cs ^= p[i];
	Packet.Checksum = cs;
}

void WFindServers::ServerLoop()
{
	PACKET	Packet;
	while (1) {
		DWORD Rcvd = m_Udp->Read(&Packet, PACKET_SIZE);	// block forever
		if (Rcvd < PACKET_SIZE) {
			if (!Rcvd || m_Udp->GetLastError())
				break;		// timeout or socket error: we're done
			else
				continue;	// truncated packet: keep on going
		}
		if (!CheckPacket(Packet))
			continue;		// packet was bogus: keep on going
		switch (Packet.RecordType) {
		case REQUEST_IP:
//
// If the request contains a non-null server name string, the client only wants
// to see responses from servers whose names match the request.
//
			if (Packet.Record.ServerInfo.Name[0]) {
				if (_stricmp(Packet.Record.ServerInfo.Name, m_ServerName))
					break;
			}
			RECORD	Record;
			memset(&Record, 0, MAX_RECORD);	// zero the record, just in case
//
// If there's a status callback, call it, and pass the returned status back to
// the client.  The status format is user-defined.
//
			if (m_StatusCallback != NULL) {
				Record.ServerInfo.Status = m_StatusCallback(m_Udp->GetReturnAddressString(),
					m_StatusCallbackArg, Record.ServerInfo.ExtStatus);
			}
//
// Fill in the server name with our own, but leave the server IP address blank;
// it will be filled in by the client.  That way we don't have to know our own
// IP address, which saves us a lot of headaches.
//
			strcpy(Record.ServerInfo.Name, m_ServerName);
			MakePacket(SUPPLY_IP, Record, Packet);
			m_Udp->Reply(&Packet, PACKET_SIZE, SERVER_WRITE_TIMEOUT);
			break;
		}
	}
}

unsigned __stdcall WFindServers::ServerThread(void *arg)
{
	WFindServers *Zdns = (WFindServers *)arg;
	Zdns->ServerLoop();
	return(0);
}

bool WFindServers::Find(DWORD Index, SERVER_INFO& ServerInfo, LPCSTR MatchName)
{
	PACKET	Packet;
//
// If index is zero, do the broadcast.
//
	if (!Index) {
		RECORD	Record;
		memset(&Record, 0, MAX_RECORD);	// zero the record, just in case
		if (MatchName != NULL)
			strncpy(Record.ServerInfo.Name, MatchName, MAX_SERVER_NAME);
		MakePacket(REQUEST_IP, Record, Packet);
		if (m_Udp->Broadcast(m_PortNumber, &Packet, PACKET_SIZE, CLIENT_WRITE_TIMEOUT) < PACKET_SIZE)
			return(FALSE);
	}	
//
// Loop until we get a valid reply, or a timeout.
//
	while (1) {
		DWORD Rcvd = m_Udp->Read(&Packet, PACKET_SIZE, CLIENT_READ_TIMEOUT);
		if (Rcvd < PACKET_SIZE) {
			if (!Rcvd)
				return(FALSE);	// timeout: we're done
			if (m_Udp->GetLastError())
				continue;		// sockets error: ignore
			// truncated packet: ignore
		} else {
			if (CheckPacket(Packet) && Packet.RecordType == SUPPLY_IP)
				break;			// valid reply: give it to the caller
			// invalid packet: ignore
		}
	}
//
// Get the server's IP address.
//
	strcpy(Packet.Record.ServerInfo.IPAddress, m_Udp->GetReturnAddressString());
	ServerInfo = Packet.Record.ServerInfo;
	return(TRUE);
}

void WFindServers::InstallStatusCallback(STATUS_CALLBACK StatusCallback, void *UserData)
{
	m_StatusCallback = StatusCallback;
	m_StatusCallbackArg = UserData;
}

bool WFindServers::RequestReply(SOCKADDR_IN IPAddress)
{
	PACKET	Packet;
	RECORD	Record;
	memset(&Record, 0, MAX_RECORD);	// zero the record, just in case
	MakePacket(REQUEST_IP, Record, Packet);
	return(m_Udp->Write(IPAddress, &Packet, PACKET_SIZE, CLIENT_WRITE_TIMEOUT) < PACKET_SIZE);
}
