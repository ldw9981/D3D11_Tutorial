#pragma once
#include <d3d11.h>
#include "../Common/GameApp.h"

class TutorialApp :
	public GameApp
{
public:
	TutorialApp(HINSTANCE hInstance);
	~TutorialApp();

	// 렌더링 파이프라인을 구성하는 필수 객체의 인터페이스

	ID3D11Device* m_pDevice = nullptr;			// 디바이스
	ID3D11DeviceContext* m_pDeviceContext = nullptr;	// 즉시 디바이스 컨텍스트
	IDXGISwapChain* m_pSwapChain = nullptr;		// 스왑체인
	ID3D11RenderTargetView* m_pRenderTargetView = nullptr;	// 렌더링 타겟뷰

	
	// 렌더링 파이프라인에 적용하는 리소스 객체의 인터페이스

	ID3D11Buffer* m_pVertexBuffer = nullptr;		// 정점 버퍼.
	ID3D11VertexShader* m_pVertexShader = nullptr;		// 정점 셰이더.
	ID3D11PixelShader* m_pPixelShader = nullptr;		// 픽셀 셰이더.
	ID3D11InputLayout* m_pInputLayout = nullptr;	// 입력 레이아웃.


	virtual bool Initialize(UINT Width, UINT Height);
	virtual void Update();
	virtual void Render();

	bool InitD3D();
	void UninitD3D();

	bool InitScene();		// 쉐이더,버텍스,인덱스
	void UninitScene();
};

