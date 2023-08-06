#pragma once
#include "../Common/GameApp.h"
#include <d3d11.h>

class TutorialApp :
    public GameApp
{
public:
	TutorialApp(HINSTANCE hInstance);
	~TutorialApp();

	// DirectX 변수.
	ID3D11Device* pDevice = nullptr;						// 디바이스	
	ID3D11DeviceContext* pDeviceContext = nullptr;			// 디바이스 컨텍스트
	IDXGISwapChain* pSwapChain = nullptr;					// 스왑체인
	ID3D11RenderTargetView* pRenderTargetView = nullptr;	// 렌더링 타겟뷰

	virtual bool Initialize(UINT Width, UINT Height);	// 윈도우 정보는 게임 마다 다를수 있으므로 등록,생성,보이기만 한다.
	virtual void Update();
	virtual void Render();

	bool InitD3D();
	void UninitD3D();
};

