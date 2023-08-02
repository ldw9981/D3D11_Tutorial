#pragma once

#define MAX_LOADSTRING 100


class D2DRenderer;
class GameApp
{
public:
	GameApp(HINSTANCE hInstance);
	virtual ~GameApp();
	
	static HWND m_hWnd;		//자주필요하니 포인터 간접접근을 피하기위해 정적멤버로 만들었다.
	static GameApp* m_pInstance;			// 생성자에서 인스턴스 포인터를 보관한다.
	
public:
	HACCEL m_hAccelTable;
	MSG m_msg;
	HINSTANCE m_hInstance;                                // 현재 인스턴스입니다.
	WCHAR m_szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
	WCHAR m_szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.
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

