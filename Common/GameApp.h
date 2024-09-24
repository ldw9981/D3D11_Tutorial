#pragma once
#include <windows.h>
#include "TimeSystem.h"
#include "InputSystem.h"
#include "Camera.h"

#define MAX_LOADSTRING 100


class GameApp : public InputProcesser
{
public:
	GameApp(HINSTANCE hInstance);
	virtual ~GameApp();
	
	static HWND m_hWnd;		//�����ʿ��ϴ� ������ ���������� ���ϱ����� ��������� �������.
	static GameApp* m_pInstance;			// �����ڿ��� �ν��Ͻ� �����͸� �����Ѵ�.
	
public:
	HACCEL m_hAccelTable;
	MSG m_msg;
	HINSTANCE m_hInstance;                                // ���� �ν��Ͻ��Դϴ�.
	WCHAR m_szTitle[MAX_LOADSTRING];                  // ���� ǥ���� �ؽ�Ʈ�Դϴ�.
	WCHAR m_szWindowClass[MAX_LOADSTRING];            // �⺻ â Ŭ���� �̸��Դϴ�.
	WNDCLASSEXW m_wcex;
	float m_previousTime;
	float m_currentTime;
	int  m_nCmdShow;
	GameTimer m_Timer;
	InputSystem m_Input;
	UINT m_ClientWidth;
	UINT m_ClientHeight;
	Camera m_Camera;
public:
	// ������ ���� ���,����,���̱� �Ѵ�.
	virtual bool Initialize(UINT Width, UINT Height);
	virtual bool Run();
	virtual void Update(); // ��� ���� Ŭ�������� ����
	virtual void Render()=0; // ��� ���� Ŭ�������� ����
	virtual void OnInputProcess(const Keyboard::State& KeyState, const Keyboard::KeyboardStateTracker& KeyTracker,
		const Mouse::State& MouseState, const Mouse::ButtonStateTracker& MouseTracker);
	virtual LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);	
};

