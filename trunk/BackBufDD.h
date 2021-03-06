// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      10may05	initial version
		01		05may06	add CreateSurface wrapper
		02		02aug06	overload Blt for external surfaces
		03		23nov07	support Unicode
		04		30jan08	fix unmirrored exclusive in system memory case
		05		15feb09	use forward declarations
		06		16mar09	add GetDD

        DirectDraw back buffer for off-screen drawing
 
*/

#ifndef CBACKBUFDD_INCLUDED
#define CBACKBUFDD_INCLUDED

// forward declarations
struct IDirectDraw7;
struct IDirectDrawSurface7;
struct _DDSURFACEDESC2;
struct _DDBLTFX;
struct IDirectDrawClipper;

class CBackBufDD : public WObject {
public:
	enum {
		OPT_AUTO_MEMORY		= 0x01,	// automatically decide back buffer location;
									// otherwise OPT_USE_VIDEO_MEM sets location
		OPT_USE_VIDEO_MEM	= 0x02,	// if specified, create back buffer in video 
									// memory, else create it in system memory;
									// ignored if OPT_AUTO_MEMORY is specified
		OPT_MIRROR_PRECISE	= 0x04	// make mirroring more precise at the expense
									// of mirroring to an intermediate back buffer
	};
	CBackBufDD();
	~CBackBufDD();
	BOOL	Create(HWND Main, HWND View, GUID *Driver = NULL, bool Exclusive = FALSE);
	void	Destroy();
	void	DeleteSurface();
	bool	CreateSurface(int Width, int Height);
	IDirectDrawSurface7	*GetDrawBuf();
	bool	Blt();
	bool	Blt(LPRECT DestRect, IDirectDrawSurface7 *SrcSurface, 
		LPRECT SrcRect, DWORD Flags, _DDBLTFX *BltFx);
	HRESULT	GetLastError() const;
	LPCTSTR	GetLastErrorString() const;
	bool	IsCreated() const;
	void	SetMirror(bool Enable);
	bool	IsMirrored() const;
	void	SetOptions(int Options);
	int		GetOptions() const;
	bool	SetExclusive(HWND Main, HWND View, bool Enable);
	CSize	GetSize() const;
	bool	IsExclusive() const;
	bool	IsSurface() const;
	static	HMONITOR	GetFullScreenRect(HWND hWnd, CRect& rc);
	static	bool	GetMonitorGUID(HMONITOR hMon, GUID& MonGuid);
	static	LPCTSTR	GetErrorString(HRESULT hr);
	HRESULT	CreateSurface(_DDSURFACEDESC2 *SurfaceDesc, IDirectDrawSurface7 *FAR *Surface);
	IDirectDraw7	*GetDD();

private:
	typedef	struct tagERRTRAN {
		HRESULT	Code;
		LPCTSTR	Text;
	} ERRTRAN;
	typedef struct tagMONGUID {
		HMONITOR	Mon;
		GUID		Guid;
		BOOL		Valid;
	} MONGUID;
	static	const	ERRTRAN	m_ErrTran[];	// map DirectDraw error codes to text
	IDirectDraw7 *m_dd;			// pointer to DirectDraw instance
	IDirectDrawSurface7 *m_Front;	// pointer to front buffer
	IDirectDrawSurface7 *m_Back;	// pointer to back buffer
	IDirectDrawSurface7 *m_SysBack;	// pointer to back buffer in system memory
	IDirectDrawSurface7 *m_Unmirror;// pointer to unmirrored back buffer
	IDirectDrawSurface7 *m_Mirror;	// pointer to mirrored back buffer
	IDirectDrawSurface7 *m_DrawBuf;	// pointer to buffer we draw to
    IDirectDrawClipper	*m_Clipper;	// pointer to clipper
	HWND	m_Main;				// top-level owner
	HWND	m_View;				// window to clip to
	HRESULT	m_hr;				// most recent DirectDraw result code
	int		m_Width;			// width of window, in client coords
	int		m_Height;			// height of window, in client coords
	bool	m_IsMirrored;		// true if we're mirroring
	bool	m_IsExclusive;		// true if we're in full-screen exclusive mode
	int		m_Options;			// display options; see enum above
	WINDOWPLACEMENT	m_PreExcl;	// window placement before exclusive mode

	static	BOOL WINAPI DDEnumCallbackEx(GUID FAR *lpGUID, LPSTR lpDriverDescription,
		LPSTR lpDriverName, LPVOID lpContext, HMONITOR hm);
};

inline IDirectDrawSurface7 *CBackBufDD::GetDrawBuf()
{
	return(m_DrawBuf);
}

inline HRESULT CBackBufDD::GetLastError() const
{
	return(m_hr);
}

inline LPCTSTR CBackBufDD::GetLastErrorString() const
{
	return(GetErrorString(GetLastError()));
}

inline bool CBackBufDD::IsCreated() const
{
	return(m_dd != NULL);
}

inline bool CBackBufDD::IsMirrored() const
{
	return(m_IsMirrored);
}

inline int CBackBufDD::GetOptions() const
{
	return(m_Options);
}

inline CSize CBackBufDD::GetSize() const
{
	return(CSize(m_Width, m_Height));
}

inline bool CBackBufDD::IsExclusive() const
{
	return(m_IsExclusive);
}

inline bool CBackBufDD::IsSurface() const
{
	return(m_Back != NULL);
}

inline IDirectDraw7 *CBackBufDD::GetDD()
{
	return(m_dd);
}

#endif
