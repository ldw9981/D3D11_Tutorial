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

	// DirectX 변수.
	ID3D11Device* pDevice = nullptr;
	ID3D11DeviceContext* pDeviceContext = nullptr;
	IDXGISwapChain* pSwapChain = nullptr;
	ID3D11RenderTargetView* pRenderTargetView = nullptr;

	ID3D11Buffer* vertexBuffer = nullptr;		// 정점 버퍼.
	ID3D11VertexShader* vertexShader = nullptr;		// 정점 셰이더.
	ID3D11PixelShader* pixelShader = nullptr;		// 픽셀 셰이더.
	ID3DBlob* vertexShaderBuffer = nullptr;		// 정점 셰이더 버퍼.
	ID3DBlob* pixelShaderBuffer = nullptr;		// 픽셀 셰이더 버퍼.
	ID3D11InputLayout* vertexInputLayout = nullptr;	// 입력 레이아웃.

	int nIndices=0;							// 인덱스 개수.

	ID3D11Buffer* g_pIndexBuffer = nullptr;	
	ID3D11Buffer* g_pConstantBuffer = nullptr;	// 상수 버퍼.

	Matrix                g_World;
	Matrix                g_View;
	Matrix                g_Projection;

	virtual bool Initialize(UINT Width, UINT Height);
	virtual void Update();
	virtual void Render();

	bool InitD3D();			
	void UninitD3D();

	bool InitScene();		// 쉐이더,버텍스,인덱스
	void UninitScene();		 
};

