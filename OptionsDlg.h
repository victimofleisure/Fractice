// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
        01      08feb09	convert from dialog to property sheet
		02		12feb09	add network adapter selection
		03		16feb09	add display options page
		04		01mar09	add scroll delta
		05		03mar09	add MIDI page
		06		31mar09	make UpdateDisplayList public
		07		06jan10	W64: DoModal return value is 64-bit

        options property sheet
 
*/

#if !defined(AFX_OPTIONSDLG_H__2EF17119_9713_40D8_8BA0_8D5A182DEA75__INCLUDED_)
#define AFX_OPTIONSDLG_H__2EF17119_9713_40D8_8BA0_8D5A182DEA75__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptionsDlg

#include "OptionsInfo.h"
#include "OptsGeneralDlg.h"
#include "OptsDocumentDlg.h"
#include "OptsNavigateDlg.h"
#include "OptsPaletteDlg.h"
#include "OptsEngineDlg.h"
#include "OptsNetworkDlg.h"
#include "OptsDisplayDlg.h"
#include "OptsMidiDlg.h"

class CDDEnumObj;

class COptionsDlg : public CPropertySheet, protected COptionsInfo
{
	DECLARE_DYNAMIC(COptionsDlg)
// Construction
public:
	COptionsDlg(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

// Attributes
	void	GetInfo(COptionsInfo& Info) const;
	void	SetInfo(const COptionsInfo& Info);
	CString	GetDefSnapshot() const;
	CSize	GetThumbSize() const;
	int		GetDefDocFmt() const;
	UINT	GetThreadCount() const;
	int		GetUndoLevels() const;
	bool	GetSaveChgsWarn() const;
	bool	GetUseSSE2() const;
	WORD	GetNetworkPort() const;
	double	GetZoomStep() const;
	double	GetContZoomStep() const;
	int		GetScrollDelta() const;
	COLORREF	GetGridColor() const;
	bool	GetListenServers() const;
	bool	GetCacheImages() const;
	bool	GetSaveEscTimes() const;
	UINT	GetPalImpExpFlags() const;
	int		GetPalExportSize() const;
	int		GetHueRotation() const;
	UINT	GetMaxMRUParams() const;
	UINT	GetMaxMRUPalettes() const;
	CString	GetNicDescription() const;
	CString	GetNicIPAddress() const;
	LPCTSTR	GetNicIPAddressOrNull() const;
	bool	GetDisplayInfo(CDDEnumObj& DDObj) const;
	UINT	GetFrameRate() const;
	int		GetMidiDevice() const;

// Operations
	void	SetDefaults();
	void	UpdateDisplayList();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptionsDlg)
	public:
	virtual BOOL OnInitDialog();
	virtual W64INT DoModal();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~COptionsDlg();

// Generated message map functions
protected:
	//{{AFX_MSG(COptionsDlg)
	afx_msg void OnDestroy();
	afx_msg void OnResetAll();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Constants
	static const BASE_OPTIONS_INFO	m_DefaultState;

// Member data
	COptsGeneralDlg		m_GeneralDlg;
	COptsDocumentDlg	m_DocumentDlg;
	COptsNavigateDlg	m_NavigateDlg;
	COptsEngineDlg		m_EngineDlg;
	COptsPaletteDlg		m_PaletteDlg;
	COptsNetworkDlg		m_NetworkDlg;
	COptsDisplayDlg		m_DisplayDlg;
	COptsMidiDlg		m_MidiDlg;

	CButton	m_ResetAll;		// reset all button
	int		m_CurPage;		// index of current page
	COptionsInfo	m_Save;	// backup of state, restored on cancel

// Helpers
	void	InitState();
	void	ReadState();
	void	WriteState();
	void	CreateResetAllButton();
};

inline void COptionsDlg::GetInfo(COptionsInfo& Info) const
{
	Info = *this;
}

inline void COptionsDlg::SetInfo(const COptionsInfo& Info)
{
	COptionsInfo&	oi = *this;	// downcast to base class
	oi = Info;
}

inline CString COptionsDlg::GetDefSnapshot() const
{
	return(m_DefSnapshot);
}

inline CSize COptionsDlg::GetThumbSize() const
{
	return(m_ThumbSize);
}

inline int COptionsDlg::GetDefDocFmt() const
{
	return(m_DefDocFmt);
}

inline double COptionsDlg::GetZoomStep() const
{
	return(m_ZoomStep);
}

inline double COptionsDlg::GetContZoomStep() const
{
	return(m_ContZoomStep);
}

inline int COptionsDlg::GetScrollDelta() const
{
	return(m_ScrollDelta);
}

inline UINT COptionsDlg::GetThreadCount() const
{
	return(m_ThreadCount);
}

inline int COptionsDlg::GetUndoLevels() const
{
	return(m_UndoLevels);
}

inline bool COptionsDlg::GetSaveChgsWarn() const
{
	return(m_SaveChgsWarn != 0);
}

inline bool COptionsDlg::GetUseSSE2() const
{
	return(m_UseSSE2 != 0);
}

inline WORD COptionsDlg::GetNetworkPort() const
{
	return(WORD(m_NetworkPort));
}

inline COLORREF COptionsDlg::GetGridColor() const
{
	return(m_GridColor);
}

inline bool COptionsDlg::GetListenServers() const
{
	return(m_ListenServers != 0);
}

inline bool COptionsDlg::GetCacheImages() const
{
	return(m_CacheImages != 0);
}

inline bool COptionsDlg::GetSaveEscTimes() const
{
	return(m_SaveEscTimes != 0);
}

inline int COptionsDlg::GetPalExportSize() const
{
	return(m_PalExportSize);
}

inline int COptionsDlg::GetHueRotation() const
{
	return(m_HueRotation);
}

inline UINT COptionsDlg::GetMaxMRUParams() const
{
	return(m_MaxMRUParams);
}

inline UINT COptionsDlg::GetMaxMRUPalettes() const
{
	return(m_MaxMRUPalettes);
}

inline CString COptionsDlg::GetNicDescription() const
{
	return(m_NetworkDlg.GetNicDescription());
}

inline CString COptionsDlg::GetNicIPAddress() const
{
	return(m_NetworkDlg.GetNicIPAddress());
}

inline LPCTSTR COptionsDlg::GetNicIPAddressOrNull() const
{
	return(GetNicIPAddress().IsEmpty() ? LPCTSTR(NULL) : GetNicIPAddress());
}

inline bool COptionsDlg::GetDisplayInfo(CDDEnumObj& DDObj) const
{
	return(m_DisplayDlg.GetDisplayInfo(DDObj));
}

inline UINT COptionsDlg::GetFrameRate() const
{
	return(m_FrameRate);
}

inline int COptionsDlg::GetMidiDevice() const
{
	return(m_MidiDlg.GetMidiDevice());
}

inline void COptionsDlg::UpdateDisplayList()
{
	m_DisplayDlg.UpdateDisplayList();
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTIONSDLG_H__2EF17119_9713_40D8_8BA0_8D5A182DEA75__INCLUDED_)
