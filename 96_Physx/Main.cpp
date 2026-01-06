#include <windows.h>
#include "TutorialApp.h"

int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd)
{
	TutorialApp app;
	app.SetClientSize(1024, 768);

	
	int retValue = app.Run(hInstance) ? 0 : -1;

	
	return retValue;
}
