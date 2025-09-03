#pragma once
#include <d3d11.h>

#include "../Common/GameApp.h"

class TutorialApp :
	public GameApp
{
public:
	// 렌더링 파이프라인을 구성하는 필수 객체의 인터페이스 ( 뎊스 스텐실 뷰도 있지만 아직 사용하지 않는다.)
	ID3D11Device* m_pDevice = nullptr;						// 디바이스	
	ID3D11DeviceContext* m_pDeviceContext = nullptr;		// 즉시 디바이스 컨텍스트
	IDXGISwapChain1* m_pSwapChain = nullptr;					// 스왑체인
    ID3D11RenderTargetView* m_pRenderTargetView = nullptr;	// 렌더링 타겟뷰v
	
	bool OnInitialize() override;	
	void OnUninitialize() override;
	void OnUpdate() override;
	void OnRender() override;

	bool InitD3D();
	void UninitD3D();
};

