// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda
 
		rev		date	comments
		00		20jul07	initial version
		
		message box with custom button text
		  
*/

#include "stdafx.h"
#include "CustomMsgBox.h"

HHOOK	CCustomMsgBox::m_Hook;
const LPCTSTR *CCustomMsgBox::m_ButtonText;
int		CCustomMsgBox::m_Buttons;

LRESULT CALLBACK CCustomMsgBox::CBTProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode < 0)
		return CallNextHookEx(m_Hook, nCode, wParam, lParam);
	switch (nCode) {
	case HCBT_ACTIVATE:
		{
			HWND	dlg = (HWND)wParam;
			for (int i = 0; i < m_Buttons; i++) {
				if (m_ButtonText[i] != NULL) {
					int	id = IDOK + i;
					if (GetDlgItem(dlg, id) != NULL)
						SetDlgItemText(dlg, id, m_ButtonText[i]);
				}
			}
		}
		return 0;
	}
	return CallNextHookEx(m_Hook, nCode, wParam, lParam);
}

int CCustomMsgBox::MsgBox(LPCTSTR Text, UINT Type, const LPCTSTR *ButtonText, int Buttons)
{
	m_ButtonText = ButtonText;
	m_Buttons = Buttons;
	m_Hook = SetWindowsHookEx(WH_CBT, CBTProc, NULL, GetCurrentThreadId());
	int	retval = AfxMessageBox(Text, Type);
	UnhookWindowsHookEx(m_Hook);
	return retval;
}
