// 01_imgui.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//
#include "TutorialApp.h"


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	TutorialApp App;
	int argc;
	// 1. 명령줄 문자열을 인자 배열(argv)로 분리
	LPWSTR* argv = CommandLineToArgvW(lpCmdLine, &argc);
	if (argv != NULL)
	{
		for (int i = 0; i < argc; i++)
		{
			if (_wcsicmp(argv[i], L"-LDR") == 0)
			{
				App.m_forceLDR = true;
			}

			if (_wcsicmp(argv[i], L"-P709") == 0)
			{
				App.m_forceP709 = true;
			}
		}

		// 3. 메모리 해제
		LocalFree(argv);
	}
	
	return App.Run(hInstance);
}
