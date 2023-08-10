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
	TutorialApp(HINSTANCE hInstance);
	~TutorialApp();

	// DirectX ����.
	ID3D11Device* pDevice = nullptr;
	ID3D11DeviceContext* pDeviceContext = nullptr;
	IDXGISwapChain* pSwapChain = nullptr;
	ID3D11RenderTargetView* pRenderTargetView = nullptr;

	ID3D11Buffer* vertexBuffer = nullptr;		// ���� ����.
	ID3D11VertexShader* vertexShader = nullptr;		// ���� ���̴�.
	ID3D11PixelShader* pixelShader = nullptr;		// �ȼ� ���̴�.
	ID3DBlob* vertexShaderBuffer = nullptr;		// ���� ���̴� ����.
	ID3DBlob* pixelShaderBuffer = nullptr;		// �ȼ� ���̴� ����.
	ID3D11InputLayout* vertexInputLayout = nullptr;	// �Է� ���̾ƿ�.

	int nVertices;							// ���� ����.

	ID3D11Buffer* g_pIndexBuffer = nullptr;
	int nIndices;							// �ε��� ����.

	virtual bool Initialize(UINT Width, UINT Height);
	virtual void Update();
	virtual void Render();

	bool InitD3D();			
	void UninitD3D();

	bool InitScene();		// ���̴�,���ؽ�,�ε���
	void UninitScene();		 
};

