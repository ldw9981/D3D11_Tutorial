// 01_imgui.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//
#include "TutorialApp.h"


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	TutorialApp App;
	return App.Run(hInstance);
}
