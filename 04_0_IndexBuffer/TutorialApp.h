#pragma once
#include <windows.h>
#include "../Common/GameApp.h"
#include <d3d11.h>
#include <directxtk/SimpleMath.h>
using namespace DirectX::SimpleMath;


class TutorialApp :
    public GameApp
{
public:
	// 렌더링 파이프라인을 구성하는 필수 객체의 인터페이스 (  뎊스 스텐실 뷰도 있지만 아직 사용하지 않는다.)
	ID3D11Device* m_pDevice = nullptr;						// 디바이스	
	ID3D11DeviceContext* m_pDeviceContext = nullptr;		// 즉시 디바이스 컨텍스트
	IDXGISwapChain* m_pSwapChain = nullptr;					// 스왑체인
	ID3D11RenderTargetView* m_pRenderTargetView = nullptr;	// 렌더링 타겟뷰

	// Quad 렌더링에 필요한 객체들.
	ID3D11VertexShader* m_pQuadVertexShader = nullptr;	// 정점 셰이더.
	ID3D11PixelShader* m_pQuadPixelShader = nullptr;	// 픽셀 셰이더.	
	ID3D11InputLayout* m_pQuadInputLayout = nullptr;	// 입력 레이아웃.
	ID3D11Buffer* m_pQuadVertexBuffer = nullptr;		// 버텍스 버퍼.
	UINT m_QuadVertextBufferStride = 0;					// 버텍스 하나의 크기.
	UINT m_QuadVertextBufferOffset = 0;					// 버텍스 버퍼의 오프셋.
	ID3D11Buffer* m_pQuadIndexBuffer = nullptr;			// 버텍스 버퍼.
	int m_nQuadIndices=0;								// 인덱스 개수.

	bool OnInitialize() override;
	void OnUninitialize() override;
	void OnUpdate() override;
	void OnRender() override;

	bool InitD3D();			
	void UninitD3D();

	bool InitScene();		// 쉐이더,버텍스,인덱스
	void UninitScene();		 
};

