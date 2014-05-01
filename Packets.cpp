// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		06jan10	W64: in WriteMessage, cast args to 32-bit
		02		09jan10	add extended capabilities

		app-specific packet methods
 
*/

#include "stdafx.h"
#include "Packets.h"
#include "VersionInfo.h"

// avoid .NET compiler error due to richedit.h #define wReserved, M$ bug 100766
#undef wReserved

bool CPackets::WriteMessage(UINT Message, WPARAM wParam, LPARAM lParam)
{
	FRAP_MESSAGE	pkt;
	InitHdr(pkt, Message, sizeof(pkt));
	pkt.wParam = INT64TO32(wParam);
	pkt.lParam = INT64TO32(lParam);
	return(Write(pkt, sizeof(pkt)));
}

bool CPackets::WriteCapsQuery()
{
	FRAP_CAPS_QUERY	pkt;
	InitHdr(pkt, PMID_CAPS_QUERY, sizeof(pkt));
	pkt.Signature = FRACTICE_NET_SIGNATURE;
	pkt.Version = FRACTICE_NET_VERSION;
	return(Write(pkt, sizeof(pkt)));
}

bool CPackets::WriteCapsReply(UINT CPUCount, UINT ThreadCount)
{
	FRAP_CAPS_REPLY	pkt;
	InitHdr(pkt, PMID_CAPS_REPLY, sizeof(pkt));
	pkt.Signature = FRACTICE_NET_SIGNATURE;
	pkt.Version = FRACTICE_NET_VERSION;
	pkt.CPUCount = CPUCount;
	pkt.Threads = ThreadCount;
	pkt.SnapVersion = CSnapshot::ARCHIVE_VERSION;
	pkt.Options = FRAP_OCF_HAS_EX_CAPS;
	return(Write(pkt, sizeof(pkt)));
}

void CPackets::SetOSInfo(FRAP_EX_CAPS_REPLY::OSINFO& OSInfo, const OSVERSIONINFOEX& vi, const SYSTEM_INFO& si)
{
	OSInfo.MajorVersion = vi.dwMajorVersion;
	OSInfo.MinorVersion = vi.dwMinorVersion;
	OSInfo.BuildNumber = vi.dwBuildNumber;
	OSInfo.PlatformId = vi.dwPlatformId;
	OSInfo.ServicePackMajor = vi.wServicePackMajor;
	OSInfo.ServicePackMinor = vi.wServicePackMinor;
#if _MFC_VER > 0x0600
	OSInfo.SuiteMask = vi.wSuiteMask;
	OSInfo.ProductType = vi.wProductType;
	OSInfo.Reserved = vi.wReserved;
#endif
	OSInfo.ProcessorArchitecture = si.wProcessorArchitecture;
	OSInfo.Reserved2 = si.wReserved;
	OSInfo.ProcessorType = si.dwProcessorType;
	OSInfo.ProcessorLevel = si.wProcessorLevel;
	OSInfo.ProcessorRevision = si.wProcessorRevision;
}

void CPackets::GetOSInfo(const FRAP_EX_CAPS_REPLY::OSINFO& OSInfo, OSVERSIONINFOEX& vi, SYSTEM_INFO& si)
{
	ZeroMemory(&vi, sizeof(vi));
	vi.dwMajorVersion = OSInfo.MajorVersion;
	vi.dwMinorVersion = OSInfo.MinorVersion;
	vi.dwBuildNumber = OSInfo.BuildNumber;
	vi.dwPlatformId = OSInfo.PlatformId;
	vi.wServicePackMajor = OSInfo.ServicePackMajor;
	vi.wServicePackMinor = OSInfo.ServicePackMinor;
#if _MFC_VER > 0x0600
	vi.wSuiteMask = OSInfo.SuiteMask;
	vi.wProductType = OSInfo.ProductType;
	vi.wReserved = OSInfo.Reserved;
#endif
	ZeroMemory(&si, sizeof(si));
	si.wProcessorArchitecture = OSInfo.ProcessorArchitecture;
	si.wReserved = OSInfo.Reserved2;
	si.dwProcessorType = OSInfo.ProcessorType;
	si.wProcessorLevel = OSInfo.ProcessorLevel;
	si.wProcessorRevision = OSInfo.ProcessorRevision;
}

bool CPackets::WriteExCapsReply()
{
	FRAP_EX_CAPS_REPLY	pkt;
	ZeroMemory(&pkt, sizeof(pkt));
	InitHdr(pkt, PMID_EX_CAPS_REPLY, sizeof(pkt));
	VS_FIXEDFILEINFO	AppInfo;
	CVersionInfo::GetFileInfo(AppInfo, NULL);
	pkt.ServerVersionMS = AppInfo.dwFileVersionMS;
	pkt.ServerVersionLS = AppInfo.dwFileVersionLS;
	pkt.BigNum.Version = BigNum::GetVersion();
	pkt.BigNum.VersionMinor = BigNum::GetVersionMinor();
	pkt.BigNum.PatchLevel = BigNum::GetPatchLevel();
	pkt.BigNum.LibType = BigNum::GetLibType();
	pkt.BigNum.OSType = BigNum::GetOSType();
	pkt.BigNum.CPUType = BigNum::GetCPUType();
	OSVERSIONINFOEX	vi;
	ZeroMemory(&vi, sizeof(vi));
	vi.dwOSVersionInfoSize = sizeof(vi);
	GetVersionEx((OSVERSIONINFO *)&vi);
	SYSTEM_INFO	si;
	GetSystemInfo(&si);
	SetOSInfo(pkt.OSInfo, vi, si);
	return(Write(pkt, sizeof(pkt)));
}

bool CPackets::WriteHostNameReply()
{
	static const int MAX_HOST_NAME = 256;
	DWORD	PktLen = sizeof(PACKET_HDR) + MAX_HOST_NAME;
	CByteArray	pkt;
	pkt.SetSize(PktLen);
	FRAP_HOST_NAME_REPLY	*pp = (FRAP_HOST_NAME_REPLY *)pkt.GetData();
	if (gethostname(pp->HostName, MAX_HOST_NAME) == SOCKET_ERROR)
		return(FALSE);
	InitHdr(*pp, PMID_HOST_NAME_REPLY, PktLen);
	return(Write(*pp, PktLen));
}

bool CPackets::WriteSnapshot(UINT JobID, UINT FrameID, CSnapshot& Snap)
{
	CMemFile	mf;
	FRAP_SNAPSHOT_HDR	hdr;
	ZeroMemory(&hdr, sizeof(hdr));
	mf.Write(&hdr, sizeof(hdr));	// reserve space for header
	{
		CArchive	ar(&mf, CArchive::store);
		Snap.Serialize(ar);
	}
	DWORD	PktLen = static_cast<DWORD>(mf.GetLength());
	DWORD	SnapLen = PktLen - sizeof(FRAP_SNAPSHOT_HDR);
	FRAP_SNAPSHOT	*pp = (FRAP_SNAPSHOT *)mf.Detach();
	InitHdr(*pp, PMID_SNAPSHOT, PktLen);
	pp->JobID = JobID;
	pp->FrameID = FrameID;
	pp->SnapLen = SnapLen;
	bool	retc = Write(*pp, PktLen);
	delete pp;	// buffer detached from CMemFile
	return(retc);
}

bool CPackets::WriteDib(UINT JobID, UINT FrameID, float RenderTime, PVOID DibBits, const DIB_INFO& DibInfo)
{
	UINT	DibLen = DibInfo.Length;
	UINT	PktLen = sizeof(FRAP_DIB_HDR) + DibLen;
	CByteArray	pkt;
	pkt.SetSize(PktLen);
	FRAP_DIB	*pp = (FRAP_DIB *)pkt.GetData();
	InitHdr(*pp, PMID_DIB, PktLen);
	pp->JobID = JobID;
	pp->FrameID = FrameID;
	pp->RenderTime = RenderTime;
	pp->Width = DibInfo.Size.cx;
	pp->Height = DibInfo.Size.cy;
	pp->BitCount = DibInfo.BitCount;
	pp->DibLen = DibLen;
	memcpy(pp->DibBits, DibBits, DibLen);
	return(Write(*pp, PktLen));
}

bool CPackets::WriteStripSnapshot(UINT JobID, UINT StripRows, CSnapshot& Snap)
{
	CMemFile	mf;
	FRAP_STRIP_SNAPSHOT_HDR	hdr;
	ZeroMemory(&hdr, sizeof(hdr));
	mf.Write(&hdr, sizeof(hdr));	// reserve space for header
	{
		CArchive	ar(&mf, CArchive::store);
		Snap.Serialize(ar);
	}
	DWORD	PktLen = static_cast<DWORD>(mf.GetLength());
	DWORD	SnapLen = PktLen - sizeof(FRAP_STRIP_SNAPSHOT_HDR);
	FRAP_STRIP_SNAPSHOT	*pp = (FRAP_STRIP_SNAPSHOT *)mf.Detach();
	InitHdr(*pp, PMID_STRIP_SNAPSHOT, PktLen);
	pp->JobID = JobID;
	pp->StripRows = StripRows;
	pp->SnapLen = SnapLen;
	bool	retc = Write(*pp, PktLen);
	delete pp;	// buffer detached from CMemFile
	return(retc);
}

bool CPackets::WriteStripRequest(UINT JobID, UINT ThreadIdx, UINT y1, UINT y2)
{
	FRAP_STRIP_REQUEST	pkt;
	InitHdr(pkt, PMID_STRIP_REQUEST, sizeof(pkt));
	pkt.JobID = JobID;
	pkt.ThreadIdx = ThreadIdx;
	pkt.y1 = y1;
	pkt.y2 = y2;
	return(Write(pkt, sizeof(pkt)));
}

bool CPackets::WriteStripDone(UINT JobID, UINT ThreadIdx, float RenderTime, UINT y1, UINT y2, const ESCTIME *Strip, UINT StripSize)
{
	UINT	PktLen = sizeof(FRAP_STRIP_DONE_HDR) + StripSize;
	CByteArray	pkt;
	pkt.SetSize(PktLen);
	FRAP_STRIP_DONE	*pp = (FRAP_STRIP_DONE *)pkt.GetData();
	InitHdr(*pp, PMID_STRIP_DONE, PktLen);
	pp->JobID = JobID;
	pp->ThreadIdx = ThreadIdx;
	pp->RenderTime = RenderTime;
	pp->y1 = y1;
	pp->y2 = y2;
	pp->StripSize = StripSize;
	memcpy(pp->Strip, Strip, StripSize);
	return(Write(*pp, PktLen));
}

