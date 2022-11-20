// WFPExplorer.cpp : main source file for WFPExplorer.exe
//

#include "pch.h"
#include "resource.h"
#include "MainFrm.h"
#include <ThemeHelper.h>
#include "AppSettings.h"

#pragma comment(lib, "Fwpuclnt.lib")
#pragma comment(lib, "Shlwapi.lib")

AppSettings g_Settings;
CAppModule _Module;

int Run(LPTSTR /*lpstrCmdLine*/ = nullptr, int nCmdShow = SW_SHOWDEFAULT) {
	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	CMainFrame wndMain;

	if (wndMain.CreateEx() == nullptr) {
		ATLTRACE(_T("Main window creation failed!\n"));
		return 0;
	}

	wndMain.ShowWindow(nCmdShow);

	int nRet = theLoop.Run();

	_Module.RemoveMessageLoop();
	return nRet;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow) {
	HRESULT hRes = ::CoInitialize(nullptr);
	ATLASSERT(SUCCEEDED(hRes));

	AtlInitCommonControls(ICC_BAR_CLASSES);	// add flags to support other controls
	AppSettings::Get().LoadFromKey(L"SOFTWARE\\ScorpioSoftware\\WFPExplorer");

	hRes = _Module.Init(nullptr, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	ThemeHelper::Init();

	int nRet = Run(lpstrCmdLine, nCmdShow);
	AppSettings::Get().Save();

	_Module.Term();
	::CoUninitialize();

	return nRet;
}
