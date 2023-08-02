#pragma once

#define MAX_LOADSTRING 100


class D2DRenderer;
class GameApp
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

	int  m_nCmdShow;
	SIZE m_ClientSize;
public:

	virtual bool Initialize(UINT Width, UINT Height);
	virtual void Loop();
	virtual void Update();
	virtual void Render();

	const SIZE& GetClientSize() { return m_ClientSize; 	}


	virtual LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);	
};

