
// testGamepad.h : главный файл заголовка для приложения testGamepad
//
#pragma once

#ifndef __AFXWIN_H__
	#error "включить stdafx.h до включения этого файла в PCH"
#endif

#include "resource.h"       // основные символы


// CGamepadApp:
// О реализации данного класса см. testGamepad.cpp
//

class CGamepadApp : public CWinAppEx
{
public:
	CGamepadApp();


// Переопределение
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	Singlton<Helpers> _hlp;
	Singlton<Log> _lg;
	Singlton<Gamepad> _gp;

// Реализация

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
