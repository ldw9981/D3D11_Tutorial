#include "TutorialApp.h"
#include "../Common/Helper.h"
#include <directxtk/simplemath.h>
#include <d3dcompiler.h>

#pragma comment (lib, "d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")

using namespace DirectX::SimpleMath;

// ���� ����.
struct Vertex
{
	Vector3 position;		// ��ġ ����.
};

TutorialApp::TutorialApp(HINSTANCE hInstance)
:GameApp(hInstance)
{

}

TutorialApp::~TutorialApp()
{
	UninitScene();
	UninitD3D();	
}

bool TutorialApp::Initialize(UINT Width, UINT Height)
{
	__super::Initialize(Width, Height);

	if(!InitD3D())
		return false;

	if (!InitScene())
		return false;

	return true;
}

void TutorialApp::Update()
{

}

void TutorialApp::Render()
{
	float color[4] = { 0.0f, 0.5f, 0.5f, 1.0f };

	// ȭ�� ĥ�ϱ�.
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, color);

	// Draw�迭 �Լ��� ȣ���ϱ����� ������ ���������ο� �ʼ� �������� ������ �ؾ��Ѵ�.	
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // ������ �̾ �׸� ��� ����.
	m_pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_VertextBufferStride, &m_VertextBufferOffset);
	m_pDeviceContext->IASetInputLayout(m_pInputLayout);
	m_pDeviceContext->VSSetShader(m_pVertexShader, nullptr, 0);
	m_pDeviceContext->PSSetShader(m_pPixelShader, nullptr, 0);
	
	// Render a triangle	
	m_pDeviceContext->Draw(3, 0);

	// Present the information rendered to the back buffer to the front buffer (the screen)
	m_pSwapChain->Present(0, 0);
}

bool TutorialApp::InitD3D()
{
	// �����.
	HRESULT hr=0;

	// ����ü�� �Ӽ� ���� ����ü ����.
	DXGI_SWAP_CHAIN_DESC swapDesc = {};
	swapDesc.BufferCount = 1;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.OutputWindow = m_hWnd;	// ����ü�� ����� â �ڵ� ��.
	swapDesc.Windowed = true;		// â ��� ���� ����.
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	// �����(�ؽ�ó)�� ����/���� ũ�� ����.
	swapDesc.BufferDesc.Width = m_ClientWidth;
	swapDesc.BufferDesc.Height = m_ClientHeight;
	// ȭ�� �ֻ��� ����.
	swapDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapDesc.BufferDesc.RefreshRate.Denominator = 1;
	// ���ø� ���� ����.
	swapDesc.SampleDesc.Count = 1;
	swapDesc.SampleDesc.Quality = 0;

	UINT creationFlags = 0;
#ifdef _DEBUG
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	// 1. ��ġ ����.   2.����ü�� ����. 3.��ġ ���ؽ�Ʈ ����.
	HR_T(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, creationFlags, NULL, NULL,
		D3D11_SDK_VERSION, &swapDesc, &m_pSwapChain, &m_pDevice, NULL, &m_pDeviceContext));

	// 4. ����Ÿ�ٺ� ����.  (����۸� �̿��ϴ� ����Ÿ�ٺ�)	
	ID3D11Texture2D* pBackBufferTexture = nullptr;
	HR_T(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBufferTexture));
	HR_T(m_pDevice->CreateRenderTargetView(pBackBufferTexture, NULL, &m_pRenderTargetView));  // �ؽ�ó�� ���� ���� ����
	SAFE_RELEASE(pBackBufferTexture);	//�ܺ� ���� ī��Ʈ�� ���ҽ�Ų��.
	// ���� Ÿ���� ���� ��� ���������ο� ���ε��մϴ�.
	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, NULL);

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
	Vertex vertices[] =
	{
		Vector3(0.0f,  0.5f, 0.5f),
		Vector3(0.5f, -0.5f, 0.5f),
		Vector3(-0.5f, -0.5f, 0.5f)
	};

	D3D11_BUFFER_DESC vbDesc = {};
	// sizeof(vertices) / sizeof(Vertex).
	vbDesc.ByteWidth = sizeof(Vertex) * ARRAYSIZE(vertices);
	vbDesc.CPUAccessFlags = 0;
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.MiscFlags = 0;
	vbDesc.Usage = D3D11_USAGE_DEFAULT;

	// ���� ���� ����.
	D3D11_SUBRESOURCE_DATA vbData = {};
	vbData.pSysMem = vertices;
	HR_T(hr = m_pDevice->CreateBuffer(&vbDesc, &vbData, &m_pVertexBuffer));

	// ���ؽ� ���� ���� 
	m_VertextBufferStride = sizeof(Vertex);
	m_VertextBufferOffset = 0;


	// 2. Render() ���� ���������ο� ���ε��� InputLayout ���� 	
	D3D11_INPUT_ELEMENT_DESC layout[] =  // ��ǲ ���̾ƿ��� ���ؽ� ���̴��� �Է¹��� �������� ������ �����Ѵ�.
	{	
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	
	ID3DBlob* vertexShaderBuffer = nullptr;
	hr = D3DCompileFromFile(L"BasicVertexShader.hlsl",	// ���ؽ� ������ ������
		NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",	// ���� �Լ� �̸�
		"vs_4_0", //  ���̴� ����.
		NULL,NULL,	
		&vertexShaderBuffer, // �����ϵ� ���̴� �ڵ尡 ����� ����.
		&errorMessage);	

	if (FAILED(hr))
	{
		MessageBoxA(m_hWnd,(char*)errorMessage->GetBufferPointer() ,"����.", MB_OK);		
		SAFE_RELEASE(errorMessage);	// ������ ���� �޼��� ���̻� �ʿ����
		return false;
	}
	HR_T(hr = m_pDevice->CreateInputLayout(layout, ARRAYSIZE(layout),
		vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_pInputLayout));	


	// 3. Render���� ���������ο� ���ε���  ���ؽ� ���̴� ����
	HR_T(m_pDevice->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), NULL, &m_pVertexShader));
	SAFE_RELEASE(vertexShaderBuffer);


	// 4. Render���� ���������ο� ���ε��� �ȼ� ���̴� ����
	ID3DBlob* pixelShaderBuffer = nullptr;
	hr = D3DCompileFromFile(L"BasicPixelShader.hlsl", // ���̴� ���� �̸�.
		NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",		// ���� �Լ� �̸�
		"ps_4_0",	// ���� ���̴� ����.
		NULL,NULL,
		&pixelShaderBuffer, // �����ϵ� ���̴� �ڵ尡 ����� ����.
		&errorMessage);		// ������ ���� �޽����� ����� ����.
	
	if (FAILED(hr))
	{
		MessageBoxA(m_hWnd, (char*)errorMessage->GetBufferPointer(), "����.", MB_OK);
		SAFE_RELEASE(errorMessage);
		return false;
	}

	HR_T( m_pDevice->CreatePixelShader(
		pixelShaderBuffer->GetBufferPointer(),
		pixelShaderBuffer->GetBufferSize(), NULL, &m_pPixelShader));
	SAFE_RELEASE(pixelShaderBuffer);
	
	return true;
}

void TutorialApp::UninitScene()
{
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pInputLayout);
	SAFE_RELEASE(m_pVertexShader);
	SAFE_RELEASE(m_pPixelShader);
}
