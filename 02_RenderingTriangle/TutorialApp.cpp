#include "TutorialApp.h"
#include "../Common/Helper.h"
#include <directxtk/simplemath.h>
#include <dxgidebug.h>
#include <dxgi1_3.h>
#include <d3dcompiler.h>
#include <wrl/client.h>

#pragma comment (lib, "d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"dxgi.lib")

using namespace DirectX::SimpleMath;
using namespace Microsoft::WRL;

// ���� ����.
struct Vertex
{
	Vector3 position;		// ��ġ ����.
};

bool TutorialApp::OnInitialize()
{
	if (!InitD3D())
		return false;

	if (!InitScene())
		return false;

	return true;
}

void TutorialApp::OnUninitialize()
{
	UninitScene();
	UninitD3D();
}

void TutorialApp::OnUpdate()
{

}

void TutorialApp::OnRender()
{
	float color[4] = { 0.0f, 0.5f, 0.5f, 1.0f };

	// Flip��忡���� �������� ���� �ʿ�
	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, NULL);

	// ȭ�� ĥ�ϱ�.
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, color);

	// Draw�迭 �Լ��� ȣ���ϱ����� ������ ���������ο� �ʼ� �������� ������ �ؾ��Ѵ�.	
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // ������ �̾ �׸� ��� ����.
	m_pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_VertextBufferStride, &m_VertextBufferOffset);
	m_pDeviceContext->IASetInputLayout(m_pInputLayout);
	m_pDeviceContext->VSSetShader(m_pVertexShader, nullptr, 0);
	m_pDeviceContext->PSSetShader(m_pPixelShader, nullptr, 0);
	
	// Render a triangle	
	m_pDeviceContext->Draw(m_VertexCount, 0);

	// Present the information rendered to the back buffer to the front buffer (the screen)
	m_pSwapChain->Present(0, 0);
}

bool TutorialApp::InitD3D()
{
	HRESULT hr = 0;

	// 1. D3D11 Device,DeviceContext ����
	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	// �׷��� ī�� �ϵ������ �������� ȣȯ�Ǵ� ���� ���� DirectX ��ɷ����� �����Ͽ� ����̹��� �۵� �Ѵ�.
	// �������̽��� Direc3D11 ������ GPU����̹��� D3D12 ����̹��� �۵��Ҽ��� �ִ�.
	D3D_FEATURE_LEVEL featureLevels[] = { // index 0���� ������� �õ��Ѵ�.
		D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0,D3D_FEATURE_LEVEL_11_1,D3D_FEATURE_LEVEL_11_0
	};
	D3D_FEATURE_LEVEL actualFeatureLevel; // ���� ��ó ������ ������ ����

	HR_T(D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		0,
		creationFlags,
		featureLevels,
		ARRAYSIZE(featureLevels),
		D3D11_SDK_VERSION,
		&m_pDevice,
		&actualFeatureLevel,
		&m_pDeviceContext
	));

	// 2. ����ü�� ������ ���� DXGI Factory ����
	UINT dxgiFactoryFlags = 0;
#ifdef _DEBUG
	dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	ComPtr<IDXGIFactory2> pFactory;
	HR_T(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&pFactory)));

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = 2;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	swapChainDesc.Width = m_ClientWidth;
	swapChainDesc.Height = m_ClientHeight;
	// �ϳ��� �ȼ��� ä�� RGBA �� 8��Ʈ �������� ǥ���Ǹ� 
	// Unsigned Normalized Integer 8��Ʈ ����(0~255)�ܰ踦 �ε��Ҽ������� ����ȭ�� 0.0~1.0���� �����Ͽ� ǥ���Ѵ�.
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // ���� ü���� �� ���۰� ������ ������������ ���� ��� ������� ���
	swapChainDesc.SampleDesc.Count = 1;  // ��Ƽ���ø� ��� ����
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE; // Recommended for flip models
	swapChainDesc.Stereo = FALSE;  // ���׷��� 3D �������� ��Ȱ��ȭ
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // ��ü ȭ�� ��ȯ�� ���
	swapChainDesc.Scaling = DXGI_SCALING_NONE; //  â�� ũ��� �� ������ ũ�Ⱑ �ٸ� ��. ����� ũ�⿡ �°� �����ϸ� ���� �ʴ´�.

	HR_T(pFactory->CreateSwapChainForHwnd(
		m_pDevice,
		m_hWnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		&m_pSwapChain
	));

	// 3. ����Ÿ�� �� ����.  ���� Ÿ�� ��� "����ٰ� �׸��� �׷���"��� GPU���� �˷��ִ� ������ �ϴ� ��ü.
	// �ؽ�ó�� ������ ����Ǵ� ��ü�̴�. 
	ComPtr<ID3D11Texture2D> pBackBufferTexture;
	HR_T(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBufferTexture));
	HR_T(m_pDevice->CreateRenderTargetView(pBackBufferTexture.Get(), nullptr, &m_pRenderTargetView));

	// ����Ʈ ����.	
	D3D11_VIEWPORT viewport={};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)m_ClientWidth;
	viewport.Height = (float)m_ClientHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	m_pDeviceContext->RSSetViewports(1, &viewport);
	return true;
}

void TutorialApp::UninitD3D()
{
	SAFE_RELEASE(m_pRenderTargetView);
	SAFE_RELEASE(m_pSwapChain);
	SAFE_RELEASE(m_pDeviceContext);
	SAFE_RELEASE(m_pDevice);
}

bool TutorialApp::InitScene()
{	
	HRESULT hr=0; // �����.
	ID3D10Blob* errorMessage = nullptr;	 // ������ ���� �޽����� ����� ����.	

	//1. Render() ���� ���������ο� ���ε��� ���ؽ� ���۹� ���� ���� �غ�
	// ������ VertexShader�� World, View, Projection ��ȯ�� ������� �����Ƿ� 
	// ���� Normalized Device Coordinate(��ǥ��)�� ��ġ�� �����Ѵ�.
	//      /---------------------(1,1,1)   z���� ���̰�
	//     /                      / |   
	// (-1,1,0)----------------(1,1,0)        
	//   |         v1           |   |
	//   |        /   `         |   |       �߾��� (0,0,0)  
	//   |       /  +   `       |   |
	//   |     /         `      |   |
	//	 |   v0-----------v2    |  /
	// (-1,-1,0)-------------(1,-1,0)
	Vertex vertices[] =
	{
		Vector3(-0.5,-0.5,0.5), // v0    
		Vector3(0,0.5,0.5),		// v1    
		Vector3(0.5,-0.5,0.5),	// v2		
	};

	D3D11_BUFFER_DESC vbDesc = {};
	m_VertexCount = ARRAYSIZE(vertices);	// ������ ��
	vbDesc.ByteWidth = sizeof(Vertex) * m_VertexCount; // ���ؽ� ������ ũ��(Byte).
	vbDesc.CPUAccessFlags = 0;
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER; // ���� ���۷� ���.
	vbDesc.MiscFlags = 0;
	vbDesc.Usage = D3D11_USAGE_DEFAULT;	// CPU�� ���ٺҰ� ,  GPU���� �б�/���� ������ ���۷� ����.

	// ���� ���� ����.
	D3D11_SUBRESOURCE_DATA vbData = {};
	vbData.pSysMem = vertices;	// ���۸� �����Ҷ� ������ �������� �ּ� ���� 
	HR_T(hr = m_pDevice->CreateBuffer(&vbDesc, &vbData, &m_pVertexBuffer));

	// ���ؽ� ���� ���� 
	m_VertextBufferStride = sizeof(Vertex); // ���ؽ� �ϳ��� ũ��
	m_VertextBufferOffset = 0;	// ���ؽ� ���� �ּҿ��� ���� ������ �ּ�
	
	// 2. Render���� ���������ο� ���ε���  ���ؽ� ���̴� ����
	ID3DBlob* vertexShaderBuffer = nullptr; // ���ؽ� ���̴� HLSL�� �����ϵ� ���(����Ʈ�ڵ�)�� ������ �ִ� ���� ��ü
	HR_T(CompileShaderFromFile(L"BasicVertexShader.hlsl", "main", "vs_4_0", &vertexShaderBuffer));
	HR_T(m_pDevice->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), // �ʿ��� �����͸� �����ϸ� ��ü ���� 
		vertexShaderBuffer->GetBufferSize(), NULL, &m_pVertexShader));
	
	// 3. Render() ���� ���������ο� ���ε��� InputLayout ���� 	
	D3D11_INPUT_ELEMENT_DESC layout[] =  // ��ǲ ���̾ƿ��� ���ؽ� ���̴��� �Է¹��� �������� ������ �����Ѵ�.
	{// SemanticName , SemanticIndex , Format , InputSlot , AlignedByteOffset , InputSlotClass , InstanceDataStepRate		
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	// ���ؽ� ���̴��� Input�� ������ ����� ������ �����ϸ鼭 InputLayout�� �����Ѵ�.
	HR_T(hr = m_pDevice->CreateInputLayout(layout, ARRAYSIZE(layout),
		vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_pInputLayout));	

	SAFE_RELEASE(vertexShaderBuffer); // ���������� ���۴� ���� ����


	// 4. Render���� ���������ο� ���ε��� �ȼ� ���̴� ����
	ID3DBlob* pixelShaderBuffer = nullptr; // �ȼ� ���̴� HLSL�� �����ϵ� ���(����Ʈ�ڵ�)�� ������ �ִ� ���� ��ü
	HR_T(CompileShaderFromFile(L"BasicPixelShader.hlsl", "main", "ps_4_0", &pixelShaderBuffer));	
	HR_T( m_pDevice->CreatePixelShader(	  // �ʿ��� �����͸� �����ϸ� ��ü ���� 
		pixelShaderBuffer->GetBufferPointer(),
		pixelShaderBuffer->GetBufferSize(), NULL, &m_pPixelShader));
	SAFE_RELEASE(pixelShaderBuffer); // ���������� ���۴� ���� ����
	
	return true;
}

void TutorialApp::UninitScene()
{
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pInputLayout);
	SAFE_RELEASE(m_pVertexShader);
	SAFE_RELEASE(m_pPixelShader);
}
