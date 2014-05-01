// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		09jan10	add extended capabilities

		app-specific packet methods
 
*/

#ifndef PACKETS_INCLUDED
#define PACKETS_INCLUDED

#include "PacketConn.h"
#include "PacketDefs.h"
#include "Snapshot.h"
#include "DibEngine.h"

class CPackets : public CPacketConn {
public:
// Types
	typedef CDibEngine::DIB_INFO DIB_INFO;

// Operations
	bool	WriteMessage(UINT Message, WPARAM wParam = 0, LPARAM lParam = 0);
	bool	WriteCapsQuery();
	bool	WriteCapsReply(UINT CPUCount, UINT ThreadCount);
	bool	WriteExCapsReply();
	bool	WriteHostNameReply();
	bool	WriteSnapshot(UINT JobID, UINT FrameID, CSnapshot& Snap);
	bool	WriteDib(UINT JobID, UINT FrameID, float RenderTime, PVOID DibBits, const DIB_INFO& DibInfo);
	bool	WriteStripSnapshot(UINT JobID, UINT StripRows, CSnapshot& Snap);
	bool	WriteStripRequest(UINT JobID, UINT ThreadIdx, UINT y1, UINT y2);
	bool	WriteStripDone(UINT JobID, UINT ThreadIdx, float RenderTime, UINT y1, UINT y2, const ESCTIME *Strip, UINT StripSize);
	static	void	SetOSInfo(FRAP_EX_CAPS_REPLY::OSINFO& OSInfo, const OSVERSIONINFOEX& vi, const SYSTEM_INFO& si);
	static	void	GetOSInfo(const FRAP_EX_CAPS_REPLY::OSINFO& OSInfo, OSVERSIONINFOEX& vi, SYSTEM_INFO& si);

protected:
// Helpers
	void	InitHdr(PACKET_HDR& Packet, UINT Message, UINT PacketLen);
};

inline void CPackets::InitHdr(PACKET_HDR& Packet, UINT Message, UINT PacketLen)
{
	Packet.Message = Message;
	Packet.DataLen = PacketLen - sizeof(PACKET_HDR);
}

#endif
