// 01_imgui.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "01_imgui.h"
#include "TutorialApp.h"


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	TutorialApp App(hInstance);  // 생성자에서 아이콘,윈도우 이름만 바꾼다
	App.Initialize(1024, 768);
	App.Loop();
	return 1;
}
