#include "TutorialApp.h"
#include "../Common/Helper.h"
#include <directxtk/simplemath.h>
#include <d3dcompiler.h>

#pragma comment (lib, "d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")

using namespace DirectX::SimpleMath;

TutorialApp::TutorialApp(HINSTANCE hInstance)
:GameApp(hInstance)
{

}

TutorialApp::~TutorialApp()
{
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
	pDeviceContext->ClearRenderTargetView(pRenderTargetView, color);

	// Render a triangle
	pDeviceContext->VSSetShader(vertexShader, nullptr, 0);
	pDeviceContext->PSSetShader(pixelShader, nullptr, 0);
	pDeviceContext->Draw(3, 0);

	// Present the information rendered to the back buffer to the front buffer (the screen)
	pSwapChain->Present(0, 0);
}

bool TutorialApp::InitD3D()
{
	// �����.
	HRESULT hr;

	// ����ü�� �Ӽ� ���� ����ü ����.
	DXGI_SWAP_CHAIN_DESC swapDesc;
	ZeroMemory(&swapDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	swapDesc.BufferCount = 1;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.OutputWindow = m_hWnd;	// ����ü�� ����� â �ڵ� ��.
	swapDesc.Windowed = true;		// â ��� ���� ����.
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
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

	// 1. ��ġ �� ����ü�� ����.
	hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, NULL, NULL,
		D3D11_SDK_VERSION, &swapDesc, &pSwapChain, &pDevice, NULL, &pDeviceContext);
	if (FAILED(hr)) {
		LOG_ERROR(L"%s", GetComErrorString(hr));
		return false;
	}
	
	// 2. ����Ÿ�ٺ� ����.
	// ����ü���� ������ ����۸� ����ϴ�. 
	ID3D11Texture2D* pBackBufferTexture;
	hr = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBufferTexture);
	if (FAILED(hr)) {
		LOG_ERROR(L"%s", GetComErrorString(hr));
		return false;
	}

	// ����ü���� ����۸� �̿��ϴ� ����Ÿ�ٺ並 �����մϴ�.
	hr = pDevice->CreateRenderTargetView(
		pBackBufferTexture, NULL, &pRenderTargetView);
	// ����Ÿ�ٺ並 ��������Ƿ� ����� �ؽ�ó �������̽��� ���̻� �ʿ����� �ʽ��ϴ�.
	SAFE_RELEASE(pBackBufferTexture);
	if (FAILED(hr)) {
		LOG_ERROR(L"%s", GetComErrorString(hr));
		return false;
	}

	//3. ���� Ÿ���� ���� ��� ���������ο� ���ε��մϴ�.
	pDeviceContext->OMSetRenderTargets(1, &pRenderTargetView, NULL);
	return true;
}

void TutorialApp::UninitD3D()
{
	// Cleanup DirectX
	SAFE_RELEASE(pDevice);
	SAFE_RELEASE(pDeviceContext);
	SAFE_RELEASE(pSwapChain);
	SAFE_RELEASE(pRenderTargetView);
}

bool TutorialApp::InitScene()
{	
	HRESULT hr; // �����.
	ID3D10Blob* errorMessage = nullptr;	 // ���� �޽����� ������ ����.
	
	//////////////////////////////////////////////////////////////////////////
	// ���� ���̴�	 
	// 1. ���� ���̴� �������ؼ� ���� ���̴� ���ۿ� ����.
	hr = D3DCompileFromFile(L"BasicVertexShader.hlsl",	// ���̴� ���� �̸�.
		NULL,NULL,
		"main",	// ���� �Լ� �̸�
		"vs_4_0", // ���� ���̴� ����.
		NULL,NULL,	
		&vertexShaderBuffer, // �����ϵ� ���̴� �ڵ尡 ����� ����.
		&errorMessage);	// ������ ���� �޽����� ����� ����.

	if (FAILED(hr))
	{
		MessageBoxA(m_hWnd,(char*)errorMessage->GetBufferPointer() ,"����.", MB_OK);		
		return false;
	}
	SAFE_RELEASE(errorMessage);	// ���� �޼��� ���̻� �ʿ����

	// 2. ���� ���̴� ����.
	hr = pDevice->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), NULL, &vertexShader);

	if (FAILED(hr)) {
		LOG_ERROR(L"%s", GetComErrorString(hr));
		return false;
	}

	// 3. ���� ���̴� �ܰ迡 ���ε�(����, ����)binding.
	pDeviceContext->VSSetShader(vertexShader, NULL, NULL);

	//////////////////////////////////////////////////////////////////////////
	// �ȼ� ���̴� 
	// 1. ������.
	hr = D3DCompileFromFile(L"BasicPixelShader.hlsl", // ���̴� ���� �̸�.
		NULL,NULL,
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
	// 2. �ȼ� ���̴� ����.
	hr = pDevice->CreatePixelShader(
		pixelShaderBuffer->GetBufferPointer(),
		pixelShaderBuffer->GetBufferSize(), NULL, &pixelShader);

	if (FAILED(hr)) {
		LOG_ERROR(L"%s", GetComErrorString(hr));
		return false;
	}

	//3. �ȼ� ���̴� ����.
	pDeviceContext->PSSetShader(pixelShader, NULL, NULL);	

	// ���� ������(�迭) ����.
	Vector3 vertices[] =
	{
		Vector3( 0.0f,  0.5f, 0.5f),
		Vector3( 0.5f, -0.5f, 0.5f),
		Vector3(-0.5f, -0.5f, 0.5f)
	};

	D3D11_BUFFER_DESC vbDesc;
	ZeroMemory(&vbDesc, sizeof(D3D11_BUFFER_DESC));
	// sizeof(vertices) / sizeof(Vertex).
	vbDesc.ByteWidth = sizeof(Vector3) * ARRAYSIZE(vertices);
	vbDesc.CPUAccessFlags = 0;
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.MiscFlags = 0;
	vbDesc.Usage = D3D11_USAGE_DEFAULT;

	// �迭 ������ �Ҵ�.
	D3D11_SUBRESOURCE_DATA vbData;
	ZeroMemory(&vbData, sizeof(vbData));
	vbData.pSysMem = vertices;

	// ���� ���� ����.
	hr = pDevice->CreateBuffer(&vbDesc, &vbData, &vertexBuffer);
	if (FAILED(hr))
	{
		MessageBox(m_hWnd, L"���� ���� ���� ����.", L"����.", MB_OK);
		return false;
	}

	UINT stride = sizeof(Vector3);
	UINT offset = 0;

	// ���� ���� ���ε�.
	pDeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

	// �Է� ���̾ƿ�.
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		/*LPCSTR SemanticName;
		UINT SemanticIndex;
		DXGI_FORMAT Format;
		UINT InputSlot;
		UINT AlignedByteOffset;
		D3D11_INPUT_CLASSIFICATION InputSlotClass;
		UINT InstanceDataStepRate;*/
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	// �Է� ���̾ƿ� ����.
	hr = pDevice->CreateInputLayout(layout, ARRAYSIZE(layout),
		vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &vertexInputLayout);

	if (FAILED(hr))
	{
		MessageBox(m_hWnd, L"�Է� ���̾ƿ� ���� ����.", L"����.", MB_OK);
		return false;
	}

	// �Է� ���̾ƿ� ���ε�.
	pDeviceContext->IASetInputLayout(vertexInputLayout);

	// ������ �̾ �׸� ��� ����.
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// ����Ʈ ����.	
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)m_ClientWidth;
	viewport.Height = (float)m_ClientHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	// ����Ʈ ����.
	pDeviceContext->RSSetViewports(1, &viewport);	
	return true;
}

void TutorialApp::UninitScene()
{
	SAFE_RELEASE(vertexBuffer);
	SAFE_RELEASE(vertexShader);
	SAFE_RELEASE(pixelShader);
	SAFE_RELEASE(vertexShaderBuffer);
	SAFE_RELEASE(pixelShaderBuffer);
	SAFE_RELEASE(vertexInputLayout);
}
