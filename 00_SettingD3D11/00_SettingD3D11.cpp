// 00_SettingD3D11.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "00_SettingD3D11.h"
#include <d3d11.h>

#pragma comment (lib, "d3d11.lib")

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.
HWND hWnd;


// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int, UINT width, UINT height);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


// DirectX 변수.
ID3D11Device* pDevice;
ID3D11DeviceContext* pDeviceContext;
IDXGISwapChain* pSwapChain;
ID3D11RenderTargetView* pRenderTargetView;

bool InitD3D(HWND hwnd,UINT clientWidth,UINT clientHeight)
{
    // 결과값.
    HRESULT hr;

    // 스왑체인 속성 설정 구조체 생성.
    DXGI_SWAP_CHAIN_DESC swapDesc;
    ZeroMemory(&swapDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
    swapDesc.BufferCount = 1;
    swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapDesc.OutputWindow = hwnd;	// 스왑체인 출력할 창 핸들 값.
    swapDesc.Windowed = true;		// 창 모드 여부 설정.
    swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    // 백버퍼(텍스처)의 가로/세로 크기 설정.
    swapDesc.BufferDesc.Width = clientWidth;
    swapDesc.BufferDesc.Height = clientHeight;
    // 화면 주사율 설정.
    swapDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapDesc.BufferDesc.RefreshRate.Denominator = 1;
    // 샘플링 관련 설정.
    swapDesc.SampleDesc.Count = 1;
    swapDesc.SampleDesc.Quality = 0;

    // 장치 및 스왑체인 생성.
    hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, NULL, NULL,
        D3D11_SDK_VERSION, &swapDesc, &pSwapChain, &pDevice,
        NULL, &pDeviceContext);

    if (FAILED(hr))
    {
        MessageBox(NULL, L"장치 생성 실패.", L"오류.", MB_OK);
        return false;
    }

    // 백버퍼(텍스처).
    ID3D11Texture2D* pBackBufferTexture;
    hr = pSwapChain->GetBuffer(NULL,
        __uuidof(ID3D11Texture2D),
        (void**)&pBackBufferTexture);

    if (FAILED(hr))
    {
        MessageBox(NULL, L"백버퍼 생성 실패.", L"오류.", MB_OK);
        return false;
    }

    // 렌더 타겟 생성.
    hr = pDevice->CreateRenderTargetView(
        pBackBufferTexture, NULL, &pRenderTargetView);

    if (FAILED(hr))
    {
        MessageBox(NULL, L"렌더 타겟 생성 실패.", L"오류.", MB_OK);
        return false;
    }

    // 렌더 타겟 설정.
    pDeviceContext->OMSetRenderTargets(1, &pRenderTargetView, NULL);

    // 백버퍼 텍스처 해제.
    if (pBackBufferTexture)
    {
        pBackBufferTexture->Release();
        pBackBufferTexture = NULL;
    }
    return true;
}

void UninitD3D()
{
    if (pDevice)
    {
        pDevice->Release();
        pDevice = NULL;
    }

    if (pDeviceContext)
    {
        pDeviceContext->Release();
        pDeviceContext = NULL;
    }

    if (pSwapChain)
    {
        pSwapChain->Release();
        pSwapChain = NULL;
    }

    if (pRenderTargetView)
    {
        pRenderTargetView->Release();
        pRenderTargetView = NULL;
    }
}

void Update()
{

}
void Render()
{
    float color[4] = { 0.0f, 0.5f, 0.5f, 1.0f };

    // 화면 칠하기.
    pDeviceContext->ClearRenderTargetView(pRenderTargetView, color);


    

    // 스왑체인 교체.
    pSwapChain->Present(0, 0);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MY00SETTINGD3D11, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    SIZE ClientSize = { 800,600 };
    if (!InitInstance (hInstance, nCmdShow, ClientSize.cx, ClientSize.cy))
    {
        return FALSE;
    }

    if (!InitD3D(hWnd, ClientSize.cx, ClientSize.cy))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MY00SETTINGD3D11));

    MSG msg = { 0 };

    // 루프 실행.
    while (msg.message != WM_QUIT)
    {
        // 메시지 매핑.
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            // 업데이트.
            Update();

            // 화면 그리기.
            Render();
        }
    }

    UninitD3D();
    return (int) msg.wParam;
}



//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MY00SETTINGD3D11));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    // 메뉴는 사용하지 않는다.
    //wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_MY00SETTINGD3D11);   
    wcex.lpszMenuName = 0;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow ,UINT width,UINT height)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   // 원하는 크기가 조정되어 리턴
   RECT rcClient = { 0, 0, (LONG)width, (LONG)height };
   AdjustWindowRect(&rcClient, WS_OVERLAPPEDWINDOW, FALSE);

   //생성
   hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
       100, 100,	// 시작 위치
       rcClient.right - rcClient.left, rcClient.bottom - rcClient.top,
       nullptr, nullptr, hInstance, nullptr);


   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 메뉴 선택을 구문 분석합니다:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
