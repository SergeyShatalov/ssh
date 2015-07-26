
// testGamepad.h : ������� ���� ��������� ��� ���������� testGamepad
//
#pragma once

#ifndef __AFXWIN_H__
	#error "�������� stdafx.h �� ��������� ����� ����� � PCH"
#endif

#include "resource.h"       // �������� �������


// CGamepadApp:
// � ���������� ������� ������ ��. testGamepad.cpp
//

class CGamepadApp : public CWinAppEx
{
public:
	CGamepadApp();


// ���������������
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	Singlton<Helpers> _hlp;
	Singlton<Log> _lg;
	Singlton<Gamepad> _gp;

// ����������

public:
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CGamepadApp theApp;
