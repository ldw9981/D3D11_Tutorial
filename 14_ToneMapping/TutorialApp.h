#pragma once
#include <windows.h>
#include "../Common/GameApp.h"
#include <d3d11.h>
#include <directxtk/SimpleMath.h>
#include <dxgiformat.h>
#include <wrl/client.h>



using namespace Microsoft::WRL;
using namespace DirectX::SimpleMath;
using namespace DirectX;

class TutorialApp :
	public GameApp
{
public:
	// 렌더링 파이프라인을 구성하는 필수 객체의 인터페이스 
	ID3D11Device* m_pDevice = nullptr;						// 디바이스	
	ID3D11DeviceContext* m_pDeviceContext = nullptr;		// 즉시 디바이스 컨텍스트
	IDXGISwapChain* m_pSwapChain = nullptr;					// 스왑체인
	ID3D11RenderTargetView* m_pRenderTargetView = nullptr;	// 렌더링 타겟뷰
	ID3D11DepthStencilView* m_pDepthStencilView = nullptr;  // 깊이값 처리를 위한 뎊스스텐실 뷰
	ID3D11SamplerState* m_pSamplerLinear = nullptr;			// 선형 필터링 샘플러 상태 객체

	// HDR 렌더 타겟 관련 객체들.
	ID3D11Texture2D* m_pHdrRenderTarget = nullptr;	// 렌더 타겟 텍스처
	ID3D11RenderTargetView* m_pHdrRenderTargetView = nullptr; 
	ID3D11ShaderResourceView* m_pHdrShaderResourceView = nullptr;

	// Quad 렌더링에 필요한 객체들.
	ID3D11VertexShader* m_pQuadVertexShader = nullptr;

	ID3D11PixelShader* m_pPS_ToneMappingLDR = nullptr;
	ID3D11PixelShader* m_pPS_ToneMappingHDR = nullptr;

	ID3D11InputLayout* m_pQuadInputLayout = nullptr;
	ID3D11Buffer* m_pQuadVertexBuffer = nullptr;
	UINT m_QuadVertexBufferStride = 0;
	UINT m_QuadVertexBufferOffset = 0;
	ID3D11Buffer* m_pQuadIndexBuffer = nullptr;
	int m_nQuadIndices = 0;

	// Cube 렌더링에 필요한 객체들.
	ID3D11VertexShader* m_pCubeVertexShader = nullptr;
	ID3D11PixelShader* m_pCubePixelShader = nullptr;
	ID3D11PixelShader* m_pSolidPixelShader = nullptr;
	ID3D11InputLayout* m_pCubeInputLayout = nullptr;
	ID3D11Buffer* m_pCubeVertexBuffer = nullptr;
	UINT m_CubeVertexBufferStride = 0;
	UINT m_CubeVertexBufferOffset = 0;
	ID3D11Buffer* m_pCubeIndexBuffer = nullptr;
	int m_nCubeIndices = 0;
	bool m_forceLDR = false;
	bool m_forceP709 = false;
	bool m_UseACESFilm = true;


	DXGI_FORMAT			m_CurrFormat = DXGI_FORMAT_UNKNOWN;
	DXGI_COLOR_SPACE_TYPE m_CurrColorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;
	ID3D11Buffer* m_pLightConstantBuffer = nullptr;
	Matrix              m_World;				// 월드좌표계 공간으로 변환을 위한 행렬.
	Matrix				m_View;				// 뷰좌표계 공간으로 변환을 위한 행렬.
	Matrix              m_Projection;			// 단위장치좌표계( Normalized Device Coordinate) 공간으로 변환을 위한 행렬.


	XMFLOAT4 m_LightColors[2] =		// 라이트 색상
	{
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f)
	};

	XMFLOAT4 m_InitialLightDirs[2] =	// 초기 라이트 방향
	{
		XMFLOAT4(-0.577f, 0.577f, -0.577f, 1.0f),
		XMFLOAT4(0.0f, 0.0f, -1.0f, 1.0f),
	};	
	XMFLOAT4 m_LightDirsEvaluated[2] = {};		// 계산된 라이트 방향
	float m_rotationAngle = 0.0f;

	float m_LightIntensity[2] = { 1.0f, 5.0f };	// 라이트 세기 (배율, 1.0 = 기준 밝기)
	float m_ReferenceWhiteNit=400;
	float m_MonitorMaxNits=0.0f;
	float m_Exposure = 0.0f;
	bool m_isHDRSupported = false;
	bool m_isWideGamutSupported = false;	// 넓은 색역 지원 여부
	bool m_useWideGamut = false;		// 넓은 색역 사용 여부
	bool m_UseToneMapping = true;	// 톤매핑 적용 여부

	float m_RedPrimary[2] = { 0.0f, 0.0f };		// Red Primary CIE xy 좌표
	float m_GreenPrimary[2] = { 0.0f, 0.0f };	// Green Primary CIE xy 좌표
	float m_BluePrimary[2] = { 0.0f, 0.0f };	// Blue Primary CIE xy 좌표

	bool OnInitialize() override;
	void OnUninitialize() override;
	void OnUpdate() override;
	void OnRender() override;

	bool InitD3D();
	void UninitD3D();
	void CreateSwapChainAndBackBuffer(DXGI_FORMAT format);
	bool CheckHDRSupport(float& outMaxLuminance, DXGI_FORMAT& outFormat, DXGI_COLOR_SPACE_TYPE& outCS);
	void SelectColorSpace(DXGI_COLOR_SPACE_TYPE colorSpace);
	bool InitScene();		// 쉐이더,버텍스,인덱스
	void UninitScene();
	
	bool InitImGUI();
	void UninitImGUI();
	void RenderImGUI();

	void CreateQuad();
	void CreateCube();

	virtual LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

