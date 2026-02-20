
// LibraryApp.h: основной файл заголовка для приложения LibraryApp
//
#pragma once

#ifndef __AFXWIN_H__
	#error "включить pch.h до включения этого файла в PCH"
#endif

#include "resource.h"       // основные символы


// CLibraryAppApp:
// Сведения о реализации этого класса: LibraryApp.cpp
//

class CLibraryAppApp : public CWinApp
{
public:
	CLibraryAppApp() noexcept;


// Переопределение
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Реализация
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CLibraryAppApp theApp;
