#pragma once
#include <d3d11.h>
#include "../Common/GameApp.h"
#include <wrl/client.h>
#include <directxtk/simplemath.h>
#include <dxgidebug.h>
#include <dxgi1_3.h>
#include <d3dcompiler.h>

#pragma comment (lib, "d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"dxgi.lib")

using namespace DirectX::SimpleMath;
using namespace Microsoft::WRL;


class TutorialApp :
	public GameApp
{
public:

	// 렌더링 파이프라인을 구성하는 필수 객체의 인터페이스 (  뎊스 스텐실 뷰도 있지만 아직 사용하지 않는다.)
	ComPtr<ID3D11Device> m_pDevice = nullptr;						// 디바이스	
	ComPtr<ID3D11DeviceContext> m_pDeviceContext = nullptr;		// 즉시 디바이스 컨텍스트
	ComPtr<IDXGISwapChain1> m_pSwapChain = nullptr;					// 스왑체인
	ComPtr<ID3D11RenderTargetView> m_pRenderTargetView = nullptr;	// 렌더링 타겟뷰
	
	// 렌더링 파이프라인에 적용하는  객체와 정보
	ComPtr<ID3D11VertexShader> m_pVertexShader = nullptr;	// 정점 셰이더.
	ComPtr<ID3D11PixelShader> m_pPixelShader = nullptr;	// 픽셀 셰이더.	
	ComPtr<ID3D11InputLayout> m_pInputLayout = nullptr;	// 입력 레이아웃.
	ComPtr<ID3D11Buffer> m_pVertexBuffer = nullptr;		// 버텍스 버퍼.
	UINT m_QuadVertextBufferStride = 0;					// 버텍스 하나의 크기.
	UINT m_QuadVertextBufferOffset = 0;					// 버텍스 버퍼의 오프셋.
	UINT m_VertexCount = 0;							// 버텍스 개수.

	bool OnInitialize() override;
	void OnUninitialize() override;
	void OnUpdate() override;
	void OnRender() override;

	bool InitD3D();
	void UninitD3D();

	bool InitScene();		// 쉐이더,버텍스,인덱스
	void UninitScene();
};

