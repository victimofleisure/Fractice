// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      11jan09	initial version

        image size/resolution dialog with unit conversion
 
*/

#if !defined(AFX_IMAGESIZERES_H__1B16FC8C_CDB8_439A_B889_F3EC7F39DAED__INCLUDED_)
#define AFX_IMAGESIZERES_H__1B16FC8C_CDB8_439A_B889_F3EC7F39DAED__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ImageSizeResDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CImageSizeResDlg dialog

typedef struct tagIMAGE_SIZE_RES_INFO {
	float	m_Width;		// image width, in unit specifed by m_WidthUnit
	int		m_WidthUnit;	// which unit width is in; see size unit enum
	float	m_Height;		// image height, in unit specifed by m_HeightUnit
	int		m_HeightUnit;	// which unit height is in; see size unit enum
	float	m_Res;			// image resolution, in unit specified by m_ResUnit
	int		m_ResUnit;		// which unit resolution is in; see res unit enum
	SIZE	m_ImageSize;	// image size in pixels, rounded to nearest integer
} IMAGE_SIZE_RES_INFO;

class CImageSizeResDlg : public CDialog, protected IMAGE_SIZE_RES_INFO
{
	DECLARE_DYNAMIC(CImageSizeResDlg);
// Construction
public:
	CImageSizeResDlg(UINT nIDTemplate = IDD_IMAGE_SIZE_RES, CWnd* pParent = NULL);
	~CImageSizeResDlg();

// Constants
	enum {	// size units
		SU_PIXELS,	// size in pixels
		SU_INCHES,	// size in inches
		SU_CM,		// size in centimeters
	};
	enum {	// resolution units
		RU_PIXELS_INCH,	// resolution in pixels per inch
		RU_PIXELS_CM,	// resolution in pixels per centimeter
	};

// Attributes
	void	GetInfo(IMAGE_SIZE_RES_INFO& Info) const;
	void	SetInfo(const IMAGE_SIZE_RES_INFO& Info);
	CSize	GetImageSize() const;
	float	GetDPI() const;

// Operations

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImageSizeResDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CImageSizeResDlg)
	enum { IDD = IDD_IMAGE_SIZE_RES };
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CImageSizeResDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChangeResUnit();
	afx_msg void OnSelChangeSizeUnit();
	virtual void OnCancel();
	afx_msg void OnKillfocusWidthEdit();
	afx_msg void OnKillfocusHeightEdit();
	afx_msg void OnKillfocusResolutionEdit();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Constants
	static const float	CM_PER_INCH;	// centimeters per inch

// Data members
	IMAGE_SIZE_RES_INFO	m_PrevInfo;	// saved state to restore on cancel
	bool	m_Canceled;		// true if dialog was canceled

// Helpers
	void	UpdateImgSize();
	static	float	Norm(float Val, int SizeUnit, float DPI);
	static	float	Denorm(float Val, int SizeUnit, float DPI);
	static	float	NormRes(float Val, int ResUnit);
	static	float	DenormRes(float Val, int ResUnit);
};

inline void CImageSizeResDlg::GetInfo(IMAGE_SIZE_RES_INFO& Info) const
{
	Info = *this;
}

inline void CImageSizeResDlg::SetInfo(const IMAGE_SIZE_RES_INFO& Info)
{
	IMAGE_SIZE_RES_INFO&	OurInfo = *this;
	OurInfo = Info;
}

inline CSize CImageSizeResDlg::GetImageSize() const
{
	return(m_ImageSize);
}

inline float CImageSizeResDlg::GetDPI() const
{
	return(NormRes(m_Res, m_ResUnit));	// covert to pixels per inch
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMAGESIZERES_H__1B16FC8C_CDB8_439A_B889_F3EC7F39DAED__INCLUDED_)
