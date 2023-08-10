#include "TutorialApp.h"
#include "../Common/Helper.h"
#include <d3dcompiler.h>

#pragma comment (lib, "d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")


// ���� ����.
struct Vertex
{
	Vector3 position;		// ���� ��ġ ����.
	Vector4 color;			// ���� ���� ����.

	Vertex(float x, float y, float z) : position(x, y, z) { }
	Vertex(Vector3 position) : position(position) { }

	Vertex(Vector3 position, Vector4 color)
		: position(position), color(color) { }
};

struct ConstantBuffer
{
	Matrix mWorld;
	Matrix mView;
	Matrix mProjection;
};


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
	__super::Update();
	g_World = XMMatrixRotationY(GameTimer::m_Instance->TotalTime());
}

void TutorialApp::Render()
{
	float color[4] = { 0.0f, 0.5f, 0.5f, 1.0f };

	// ȭ�� ĥ�ϱ�.
	pDeviceContext->ClearRenderTargetView(pRenderTargetView, color);

	//
	// Update variables
	//
	ConstantBuffer cb;
	cb.mWorld = XMMatrixTranspose(g_World);
	cb.mView = XMMatrixTranspose(g_View);
	cb.mProjection = XMMatrixTranspose(g_Projection);
	pDeviceContext->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb, 0, 0);

	//
	// Renders a triangle
	//
	pDeviceContext->VSSetShader(vertexShader, nullptr, 0);
	pDeviceContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
	pDeviceContext->PSSetShader(pixelShader, nullptr, 0);

	// ���� �׸���. (Draw Call. ��ο� ��).
	//pDeviceContext->Draw(nVertices, 0);
	pDeviceContext->DrawIndexed(nIndices, 0, 0);        // 36 vertices needed for 12 triangles in a triangle list

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

	//4. ����Ʈ ����.	
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
		SAFE_RELEASE(errorMessage);	// ���� �޼��� ���̻� �ʿ����
		return false;
	}	

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


	//4. ���ؽ� ���� 
	// ���ؽ� ������(�迭) ����.
	// Local or Object or Model Space
	Vertex vertices[] =
	{
		{ Vector3(-1.0f, 1.0f, -1.0f),	Vector4(0.0f, 0.0f, 1.0f, 1.0f) },
		{ Vector3(1.0f, 1.0f, -1.0f),	Vector4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ Vector3(1.0f, 1.0f, 1.0f),	Vector4(0.0f, 1.0f, 1.0f, 1.0f) },
		{ Vector3(-1.0f, 1.0f, 1.0f),	Vector4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ Vector3(-1.0f, -1.0f, -1.0f), Vector4(1.0f, 0.0f, 1.0f, 1.0f) },
		{ Vector3(1.0f, -1.0f, -1.0f),	Vector4(1.0f, 1.0f, 0.0f, 1.0f) },
		{ Vector3(1.0f, -1.0f, 1.0f),	Vector4(1.0f, 1.0f, 1.0f, 1.0f) },
		{ Vector3(-1.0f, -1.0f, 1.0f),	Vector4(0.0f, 0.0f, 0.0f, 1.0f) },
	};

	D3D11_BUFFER_DESC bd = {};
	bd.ByteWidth = sizeof(Vertex) * ARRAYSIZE(vertices);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0;

	// �迭 ������ �Ҵ�.
	D3D11_SUBRESOURCE_DATA vbData;
	ZeroMemory(&vbData, sizeof(vbData));
	vbData.pSysMem = vertices;

	// ���ؽ� ���� ����.
	hr = pDevice->CreateBuffer(&bd, &vbData, &vertexBuffer);
	if (FAILED(hr))
	{
		MessageBox(m_hWnd, L"���� ���� ���� ����.", L"����.", MB_OK);
		return false;
	}

	// ���ؽ� ���� ���ε�.
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	pDeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);


	//5. �ε��� ����
	// �ε��� ���� ����.
	WORD indices[] =
	{
		3,1,0,
		2,1,3,

		0,5,4,
		1,5,0,

		3,4,7,
		0,4,3,

		1,6,5,
		2,6,1,

		2,7,6,
		3,7,2,

		6,4,5,
		7,4,6,
	};

	// �ε��� ���� ����.
	nIndices = ARRAYSIZE(indices);

	bd.ByteWidth = sizeof(DWORD) * ARRAYSIZE(indices);
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA ibData;
	ZeroMemory(&ibData, sizeof(D3D11_SUBRESOURCE_DATA));
	ibData.pSysMem = indices;

	// �ε��� ���� ����.
	hr = pDevice->CreateBuffer(&bd, &ibData, &g_pIndexBuffer);
	if (FAILED(hr))
	{
		MessageBox(m_hWnd, L"�ε��� ���� ���� ����.", L"����.", MB_OK);
		return false;
	}

	// �ε��� ���� ���ε�(binding).
	pDeviceContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

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
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
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

	// Create the constant buffer
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = pDevice->CreateBuffer(&bd, nullptr, &g_pConstantBuffer);
	if (FAILED(hr))
	{
		MessageBox(m_hWnd, L"������� ���� ����.", L"����.", MB_OK);
		return false;
	}

	// Initialize the world matrix
	g_World = XMMatrixIdentity();

	// Initialize the view matrix
	XMVECTOR Eye = XMVectorSet(0.0f, 1.0f, -5.0f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	g_View = XMMatrixLookAtLH(Eye, At, Up);

	// Initialize the projection matrix
	g_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, m_ClientWidth / (FLOAT)m_ClientHeight, 0.01f, 100.0f);


	//g_Projection = XMMatrixOrthographicLH(m_ClientWidth ,m_ClientHeight, 0.0f, 100);
	
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

	SAFE_RELEASE(g_pIndexBuffer);
}