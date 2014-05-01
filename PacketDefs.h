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

		packet definitions
 
*/

#ifndef PACKETDEFS_INCLUDED
#define PACKETDEFS_INCLUDED

#include "PacketConn.h"
#include "EngineTypes.h"	// for ESCTIME

enum {	// packet message IDs
	PMID_CAPS_QUERY = 1,	// sent by client to get server's capabilities
	PMID_CAPS_REPLY,		// sent by server to report its capabilities
	PMID_HOST_NAME_QUERY,	// sent by client to get server's host name
	PMID_HOST_NAME_REPLY,	// sent by server to report its host name
	PMID_SNAPSHOT,			// sent by client to begin a render
	PMID_DIB,				// sent by server to return a bitmap
	PMID_SHUTDOWN,			// sent by client to shutdown server
	PMID_CANCEL_RENDER,		// sent by client to cancel current render
	PMID_PROGRESS_CONFIG,	// sent by client to configure progress reports
	PMID_PROGRESS_REPORT,	// sent by server to report rendering progress
	PMID_STRIP_SNAPSHOT,	// sent by client to begin strip processing
	PMID_STRIP_REQUEST,		// sent by client to start rendering a strip
	PMID_STRIP_DONE,		// sent by server to return a rendered strip
	PMID_EX_CAPS_QUERY,		// sent by client to get extended capabilities
	PMID_EX_CAPS_REPLY,		// sent by server to report extended capabilities
};

enum {	// server shutdown actions
	SSDA_EXIT,			// exit application
	SSDA_LOG_OFF,		// log off user
	SSDA_REBOOT,		// reboot computer
	SSDA_POWER_OFF,		// power off computer
	SSDA_ACTIONS
};

enum {
	FRACTICE_NET_PORT = 35321,				// TCP/UDP port number
	FRACTICE_NET_SIGNATURE = 0x74637246,	// protocol signature (Frct)
	FRACTICE_NET_VERSION = 1,				// protocol version
};

enum {	// optional capability flags
	FRAP_OCF_HAS_EX_CAPS	= 0x01			// supports extended capabilities
};

typedef	struct tagFRAP_MESSAGE : public PACKET_HDR {
	UINT	wParam;		// message-dependent data
	UINT	lParam;		// message-dependent data
} FRAP_MESSAGE;

typedef	struct tagFRAP_CAPS_QUERY : public PACKET_HDR {
	UINT	Signature;	// protocol signature
	UINT	Version;	// protocol version
} FRAP_CAPS_QUERY;

typedef	struct tagFRAP_CAPS_REPLY : public PACKET_HDR {
	UINT	Signature;	// protocol signature
	UINT	Version;	// protocol version
	UINT	CPUCount;	// number of CPUs
	UINT	Threads;	// number of rendering threads
	UINT	SnapVersion;	// snapshot archive version
	UINT	Options;	// optional capability flags
} FRAP_CAPS_REPLY;

typedef	struct tagFRAP_EX_CAPS_REPLY : public PACKET_HDR {
	DWORD	ServerVersionMS;	// server version most significant
    DWORD	ServerVersionLS;	// server version least significant
	typedef struct tagBIGNUM {
		int		Version;		// bignum version
		int		VersionMinor;	// bignum minor version
		int		PatchLevel;		// bignum patch level
		int		LibType;		// bignum library type
		int		OSType;			// bignum OS type
		int		CPUType;		// bignum CPU type 
	} BIGNUM;
	BIGNUM	BigNum;				// bignum info
	typedef struct tagOSINFO {
		DWORD	MajorVersion;
		DWORD	MinorVersion;
		DWORD	BuildNumber;
		DWORD	PlatformId;
	    WORD	ServicePackMajor;
		WORD	ServicePackMinor;
		WORD	SuiteMask;
		BYTE	ProductType;
		BYTE	Reserved;
        WORD	ProcessorArchitecture;
        WORD	Reserved2;
	    DWORD	ProcessorType;
		WORD	ProcessorLevel;
		WORD	ProcessorRevision;
	} OSINFO;
	OSINFO	OSInfo;				// operating system info
	char	Reserved[244];		// for future use; set to zero
} FRAP_EX_CAPS_REPLY;

typedef	struct tagFRAP_HOST_NAME_REPLY : public PACKET_HDR {
	char	HostName[1];	// null-terminated host name
} FRAP_HOST_NAME_REPLY;

typedef	struct tagFRAP_SNAPSHOT_HDR : public PACKET_HDR {
	UINT	JobID;		// identifies this rendering job
	UINT	FrameID;	// identifies frame within this job
	UINT	SnapLen;	// size of snapshot archive, in bytes
} FRAP_SNAPSHOT_HDR;

typedef	struct tagFRAP_SNAPSHOT : public FRAP_SNAPSHOT_HDR {
	BYTE	Snapshot[1];	// variable-size snapshot archive
} FRAP_SNAPSHOT;

typedef	struct tagFRAP_DIB_HDR : public PACKET_HDR {
	UINT	JobID;		// identifies this rendering job
	UINT	FrameID;	// identifies frame within this job
	float	RenderTime;	// elapsed time, in seconds
	UINT	Width;		// bitmap width, in pixels
	UINT	Height;		// bitmap height, in pixels
	WORD	BitCount;	// bitmap color depth, in bits
	UINT	DibLen;		// size of bitmap data, in bytes
} FRAP_DIB_HDR;

typedef	struct tagFRAP_DIB : public FRAP_DIB_HDR {
	BYTE	DibBits[1];	// variable-size bitmap data
} FRAP_DIB;

typedef	struct tagFRAP_STRIP_SNAPSHOT_HDR : public PACKET_HDR {
	UINT	JobID;		// identifies this rendering job
	UINT	StripRows;	// number of rows in a strip
	UINT	SnapLen;	// size of snapshot archive, in bytes
} FRAP_STRIP_SNAPSHOT_HDR;

typedef	struct tagFRAP_STRIP_SNAPSHOT : public FRAP_STRIP_SNAPSHOT_HDR {
	BYTE	Snapshot[1];	// variable-size snapshot archive
} FRAP_STRIP_SNAPSHOT;

typedef	struct tagFRAP_STRIP_REQUEST : public PACKET_HDR {
	UINT	JobID;		// identifies this rendering job
	UINT	ThreadIdx;	// index of rendering thread
	UINT	y1;			// first row to render
	UINT	y2;			// last row to render, plus one
} FRAP_STRIP_REQUEST;

typedef	struct tagFRAP_STRIP_DONE_HDR : public PACKET_HDR {
	UINT	JobID;		// identifies this rendering job
	UINT	ThreadIdx;	// index of rendering thread
	UINT	y1;			// first row rendered
	UINT	y2;			// last row rendered, plus one
	float	RenderTime;	// elapsed time, in seconds
	UINT	StripSize;	// size of strip, in bytes
} FRAP_STRIP_DONE_HDR;

typedef	struct tagFRAP_STRIP_DONE : public FRAP_STRIP_DONE_HDR {
	ESCTIME	Strip[1];	// variable-size strip of escape times
} FRAP_STRIP_DONE;

#endif
