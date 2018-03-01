// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda

		rev		date		comments
        00      14feb08	initial version
		01		06jan10	W64: in GetPendingCount, cast server array size to 32-bit
		02		09jan10	add optional capability flags

		derived packets server

*/

#ifndef CSERVER_INCLUDED
#define CSERVER_INCLUDED

#include "RefPtr.h"
#include "Packets.h"

class CServer : public CRefObj, public CPackets {
public:
// Construction
	CServer();

// Attributes
	LPCTSTR	GetHostName() const;
	LPCTSTR	GetIPAddress() const;
	UINT	GetBinIPAddr() const;
	int		GetPendingCount() const;
	UINT	GetPendingItem(int ItemIdx) const;
	double	GetRenderTime() const;
	void	SetRenderTime(double RenderTime);
	int		GetProgress() const;
	void	SetProgress(int Progress);
	UINT	GetCPUCount() const;
	UINT	GetThreadCount() const;
	int		GetRenderedCount() const;
	void	SetRenderedCount(int Count);
	void	AddToRenderedCount(int Delta);
	UINT	GetOptions() const;

// Operations
	void	AddPending(UINT Item);
	int		FindPending(UINT Item) const;
	bool	RemovePending(UINT Item);
	void	RemoveAllPending();
	void	ResetStatistics();

protected:
// Data members
	CString	m_HostName;		// server host name
	CString	m_IPAddress;	// server IP address
	UINT	m_BinIPAddr;	// server binary IP address, in host order
	CDWordArray	m_Pending;	// array of pending item numbers
	UINT	m_CPUCount;		// number of processors server has
	UINT	m_Threads;		// number of rendering threads server has
	double	m_RenderTime;	// most recent renderering time, in seconds
	int		m_Progress;		// percentage of current render completed
	int		m_Rendered;		// number of items rendered
	UINT	m_Options;		// optional capability flags

// Helpers
	friend class CServersDlg;
};

inline LPCTSTR CServer::GetHostName() const
{
	return(m_HostName);
}

inline LPCTSTR CServer::GetIPAddress() const
{
	return(m_IPAddress);
}

inline UINT CServer::GetBinIPAddr() const
{
	return(m_BinIPAddr);
}

inline int CServer::GetPendingCount() const
{
	return(INT64TO32(m_Pending.GetSize()));
}

inline UINT CServer::GetPendingItem(int ItemIdx) const
{
	return(m_Pending[ItemIdx]);
}

inline void CServer::AddPending(UINT Item)
{
	m_Pending.Add(Item);
}

inline void CServer::RemoveAllPending()
{
	m_Pending.RemoveAll();
}

inline double CServer::GetRenderTime() const
{
	return(m_RenderTime);
}

inline void CServer::SetRenderTime(double RenderTime)
{
	m_RenderTime = RenderTime;
}

inline int CServer::GetProgress() const
{
	return(m_Progress);
}

inline void CServer::SetProgress(int Progress)
{
	m_Progress = Progress;
}

inline UINT CServer::GetCPUCount() const
{
	return(m_CPUCount);
}

inline UINT CServer::GetThreadCount() const
{
	return(m_Threads);
}

inline int CServer::GetRenderedCount() const
{
	return(m_Rendered);
}

inline void	CServer::SetRenderedCount(int Count)
{
	m_Rendered = Count;
}

inline void	CServer::AddToRenderedCount(int Delta)
{
	m_Rendered += Delta;
}

inline UINT CServer::GetOptions() const
{
	return(m_Options);
}

#endif
