
// socket.h : главный файл заголовка для приложения PROJECT_NAME
//

#pragma once

#ifndef __AFXWIN_H__
	#error "включить stdafx.h до включения этого файла в PCH"
#endif

#include "resource.h"		// основные символы


// CsocketApp:
// О реализации данного класса см. socket.cpp
//

class CsocketApp : public CWinApp
{
public:
	CsocketApp();

// Переопределение
public:
	virtual BOOL InitInstance();

// Реализация

	DECLARE_MESSAGE_MAP()
};

extern CsocketApp theApp;