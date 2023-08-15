#pragma once
#include <windows.h>
#include "../Common/GameApp.h"
#include <d3d11.h>
#include <directxtk/SimpleMath.h>


using namespace DirectX::SimpleMath;
using namespace DirectX;

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
	ID3D11DepthStencilView* m_pDepthStencilView = nullptr;	// 깊이/스텐실 뷰

	// 렌더링 파이프라인에 적용하는 리소스 객체의 인터페이스
	ID3D11Buffer* m_pVertexBuffer = nullptr;			// 버텍스 버퍼.
	ID3D11Buffer* m_pIndexBuffer = nullptr;				// 인덱스 버퍼.
	ID3D11VertexShader* m_pVertexShader = nullptr;		// 정점 셰이더.
	ID3D11PixelShader* m_pPixelShader = nullptr;		// 픽셀 셰이더.
	ID3D11PixelShader* m_pPixelShaderSolid = nullptr;	// 픽셀 셰이더.
	ID3D11InputLayout* m_pInputLayout = nullptr;		// 입력 레이아웃.
	ID3D11Buffer* m_pCBNeverChanges = nullptr;			// m_View 행렬을 위한 상수 버퍼.
	ID3D11Buffer* m_pCBChangeOnResize = nullptr;		// m_Projection 행렬을 위한 상수 버퍼.
	ID3D11Buffer* m_pCBChangesEveryFrame = nullptr;		// m_World 행렬,m_vMeshColor 위한 상수 버퍼.
	ID3D11ShaderResourceView* m_pTextureRV = nullptr;	// 텍스처 리소스 뷰
	ID3D11SamplerState* m_pSamplerLinear = nullptr;		// 텍스처 샘플러

	// 렌더링 파이프라인에 적용하는 정보
	UINT m_VertexBufferStride = 0;						// 버텍스 하나의 크기.
	UINT m_VertexBufferOffset = 0;						// 버텍스 버퍼의 오프셋.
	int m_nIndices = 0;							// 인덱스 개수.
	Matrix                m_World;				// 월드좌표계 공간으로 변환을 위한 행렬.
	Matrix                m_View;				// 뷰좌표계 공간으로 변환을 위한 행렬.
	Matrix                m_Projection;			// 단위장치좌표계( Normalized Device Coordinate) 공간으로 변환을 위한 행렬.
	Vector4              m_vMeshColor = {0.7f, 0.7f, 0.7f, 1.0f};

	virtual bool Initialize(UINT Width, UINT Height);
	virtual void Update();
	virtual void Render();

	bool InitD3D();			
	void UninitD3D();

	bool InitScene();		// 쉐이더,버텍스,인덱스
	void UninitScene();		 
};

