#include "TutorialApp.h"
#include "../Common/Helper.h"
#include <d3dcompiler.h>


#pragma comment (lib, "d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")


// 정점 선언.
struct Vertex
{
	Vector3 Pos;		// 정점 위치 정보.
	Vector3 Normal;
};

struct ConstantBuffer
{
	Matrix mWorld;
	Matrix mView;
	Matrix mProjection;

	Vector4 vLightDir[2];
	Vector4 vLightColor[2];
	Vector4 vOutputColor;
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

	if (!InitD3D())
		return false;

	if (!InitScene())
		return false;

	return true;
}

void TutorialApp::Update()
{
	__super::Update();

	float t = GameTimer::m_Instance->TotalTime();

	// 1st Cube: Rotate around the origin
	g_World1 = XMMatrixRotationY(t);

	// 2nd Cube:  Rotate around origin
	XMMATRIX mSpin = XMMatrixRotationZ(-t);
	XMMATRIX mOrbit = XMMatrixRotationY(-t * 2.0f);
	XMMATRIX mTranslate = XMMatrixTranslation(-4.0f, 0.0f, 0.0f);
	XMMATRIX mScale = XMMatrixScaling(0.3f, 0.3f, 0.3f);

	g_World2 = mScale * mSpin * mTranslate * mOrbit;
}

void TutorialApp::Render()
{
	float color[4] = { 0.0f, 0.5f, 0.5f, 1.0f };

	// 화면 칠하기.
	pDeviceContext->ClearRenderTargetView(pRenderTargetView, color);

	//
	// Clear the depth buffer to 1.0 (max depth)
	//
	pDeviceContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// Update variables for the first cube
	//
	ConstantBuffer cb1;
	cb1.mWorld = XMMatrixTranspose(g_World1);
	cb1.mView = XMMatrixTranspose(g_View);
	cb1.mProjection = XMMatrixTranspose(g_Projection);
	pDeviceContext->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb1, 0, 0);

	//
	// Render the first cube
	//
	pDeviceContext->VSSetShader(vertexShader, nullptr, 0);
	pDeviceContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
	pDeviceContext->PSSetShader(pixelShader, nullptr, 0);
	pDeviceContext->DrawIndexed(nIndices, 0, 0);

	//
	// Update variables for the second cube
	//
	ConstantBuffer cb2;
	cb2.mWorld = XMMatrixTranspose(g_World2);
	cb2.mView = XMMatrixTranspose(g_View);
	cb2.mProjection = XMMatrixTranspose(g_Projection);
	pDeviceContext->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb2, 0, 0);

	//
	// Render the second cube
	//
	pDeviceContext->DrawIndexed(nIndices, 0, 0);

	// Present the information rendered to the back buffer to the front buffer (the screen)
	pSwapChain->Present(0, 0);
}

bool TutorialApp::InitD3D()
{
	// 결과값.
	HRESULT hr;

	// 스왑체인 속성 설정 구조체 생성.
	DXGI_SWAP_CHAIN_DESC swapDesc;
	ZeroMemory(&swapDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	swapDesc.BufferCount = 1;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.OutputWindow = m_hWnd;	// 스왑체인 출력할 창 핸들 값.
	swapDesc.Windowed = true;		// 창 모드 여부 설정.
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	// 백버퍼(텍스처)의 가로/세로 크기 설정.
	swapDesc.BufferDesc.Width = m_ClientWidth;
	swapDesc.BufferDesc.Height = m_ClientHeight;
	// 화면 주사율 설정.
	swapDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapDesc.BufferDesc.RefreshRate.Denominator = 1;
	// 샘플링 관련 설정.
	swapDesc.SampleDesc.Count = 1;
	swapDesc.SampleDesc.Quality = 0;

	// 1. 장치 와 스왑체인 생성.
	hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, NULL, NULL,
		D3D11_SDK_VERSION, &swapDesc, &pSwapChain, &pDevice, NULL, &pDeviceContext);
	if (FAILED(hr)) {
		LOG_ERROR(L"%s", GetComErrorString(hr));
		return false;
	}

	// 2. 렌더타겟뷰 생성.
	// 스왑체인의 내부의 백버퍼를 얻습니다. 
	ID3D11Texture2D* pBackBufferTexture;
	hr = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBufferTexture);
	if (FAILED(hr)) {
		LOG_ERROR(L"%s", GetComErrorString(hr));
		return false;
	}

	// 스왑체인의 백버퍼를 이용하는 렌더타겟뷰를 생성합니다.
	hr = pDevice->CreateRenderTargetView(
		pBackBufferTexture, NULL, &pRenderTargetView);
	// 렌더타겟뷰를 만들었으므로 백버퍼 텍스처 인터페이스는 더이상 필요하지 않습니다.
	SAFE_RELEASE(pBackBufferTexture);
	if (FAILED(hr)) {
		LOG_ERROR(L"%s", GetComErrorString(hr));
		return false;
	}

	// Create depth stencil texture
	D3D11_TEXTURE2D_DESC descDepth = {};
	descDepth.Width = m_ClientWidth;
	descDepth.Height = m_ClientHeight;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	hr = pDevice->CreateTexture2D(&descDepth, nullptr, &g_pDepthStencil);
	if (FAILED(hr)) {
		LOG_ERROR(L"%s", GetComErrorString(hr));
		return false;
	}

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	hr = pDevice->CreateDepthStencilView(g_pDepthStencil, &descDSV, &g_pDepthStencilView);
	if (FAILED(hr)) {
		LOG_ERROR(L"%s", GetComErrorString(hr));
		return false;
	}

	pDeviceContext->OMSetRenderTargets(1, &pRenderTargetView, g_pDepthStencilView);

	//4. 뷰포트 설정.	
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)m_ClientWidth;
	viewport.Height = (float)m_ClientHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	// 뷰포트 설정.
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
	HRESULT hr; // 결과값.
	ID3D10Blob* errorMessage = nullptr;	 // 에러 메시지를 저장할 버퍼.

	//////////////////////////////////////////////////////////////////////////
	// 정점 셰이더	 
	// 1. 정점 셰이더 컴파일해서 정점 셰이더 버퍼에 저장.
	hr = D3DCompileFromFile(L"BasicVertexShader.hlsl",	// 셰이더 파일 이름.
		NULL, NULL,
		"VS",	// 시작 함수 이름
		"vs_4_0", // 정점 셰이더 버전.
		NULL, NULL,
		&vertexShaderBuffer, // 컴파일된 셰이더 코드가 저장될 버퍼.
		&errorMessage);	// 컴파일 에러 메시지가 저장될 버퍼.

	if (FAILED(hr))
	{
		MessageBoxA(m_hWnd, (char*)errorMessage->GetBufferPointer(), "오류.", MB_OK);
		SAFE_RELEASE(errorMessage);	// 에러 메세지 더이상 필요없음
		return false;
	}

	// 2. 정점 셰이더 생성.
	hr = pDevice->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), NULL, &vertexShader);

	if (FAILED(hr)) {
		LOG_ERROR(L"%s", GetComErrorString(hr));
		return false;
	}

	// 3. 정점 셰이더 단계에 바인딩(설정, 연결)binding.
	pDeviceContext->VSSetShader(vertexShader, NULL, NULL);

	//////////////////////////////////////////////////////////////////////////
	// 픽셀 셰이더 
	// 1. 컴파일.
	hr = D3DCompileFromFile(L"BasicPixelShader.hlsl", // 셰이더 파일 이름.
		NULL, NULL,
		"PS",		// 시작 함수 이름
		"ps_4_0",	// 정점 셰이더 버전.
		NULL, NULL,
		&pixelShaderBuffer, // 컴파일된 셰이더 코드가 저장될 버퍼.
		&errorMessage);		// 컴파일 에러 메시지가 저장될 버퍼.

	if (FAILED(hr))
	{
		MessageBoxA(m_hWnd, (char*)errorMessage->GetBufferPointer(), "오류.", MB_OK);
		SAFE_RELEASE(errorMessage);
		return false;
	}
	// 2. 픽셀 셰이더 생성.
	hr = pDevice->CreatePixelShader(
		pixelShaderBuffer->GetBufferPointer(),
		pixelShaderBuffer->GetBufferSize(), NULL, &pixelShader);

	if (FAILED(hr)) {
		LOG_ERROR(L"%s", GetComErrorString(hr));
		return false;
	}

	//3. 픽셀 셰이더 설정.
	pDeviceContext->PSSetShader(pixelShader, NULL, NULL);

	//////////////////////////////////////////////////////////////////////////
	// Solid 픽셀 셰이더 
	// 1. 컴파일.
	hr = D3DCompileFromFile(L"SolidPixelShader.hlsl", // 셰이더 파일 이름.
		NULL, NULL,
		"PSSolid",		// 시작 함수 이름
		"ps_4_0",	// 정점 셰이더 버전.
		NULL, NULL,
		&pixelShaderBuffer, // 컴파일된 셰이더 코드가 저장될 버퍼.
		&errorMessage);		// 컴파일 에러 메시지가 저장될 버퍼.

	if (FAILED(hr))
	{
		MessageBoxA(m_hWnd, (char*)errorMessage->GetBufferPointer(), "오류.", MB_OK);
		SAFE_RELEASE(errorMessage);
		return false;
	}
	// 2. 픽셀 셰이더 생성.
	hr = pDevice->CreatePixelShader(
		pixelShaderBuffer->GetBufferPointer(),
		pixelShaderBuffer->GetBufferSize(), NULL, &pixelShader);

	if (FAILED(hr)) {
		LOG_ERROR(L"%s", GetComErrorString(hr));
		return false;
	}

	//3. 픽셀 셰이더 설정.
	pDeviceContext->PSSetShader(pixelShader, NULL, NULL);





	//4. 버텍스 버퍼 
	// 버텍스 데이터(배열) 생성.
	// Local or Object or Model Space
	Vertex vertices[] =
	{
		{ Vector3(-1.0f, 1.0f, -1.0f),	Vector3(0.0f, 1.0f, 0.0f) },
		{ Vector3(1.0f, 1.0f, -1.0f),	Vector3(0.0f, 1.0f, 0.0f) },
		{ Vector3(1.0f, 1.0f, 1.0f),	Vector3(0.0f, 1.0f, 0.0f) },
		{ Vector3(-1.0f, 1.0f, 1.0f),	Vector3(0.0f, 1.0f, 0.0f) },

		{ Vector3(-1.0f, -1.0f, -1.0f), Vector3(0.0f, -1.0f, 0.0f) },
		{ Vector3(1.0f, -1.0f, -1.0f),	Vector3(0.0f, -1.0f, 0.0f) },
		{ Vector3(1.0f, -1.0f, 1.0f),	Vector3(0.0f, -1.0f, 0.0f) },
		{ Vector3(-1.0f, -1.0f, 1.0f),	Vector3(0.0f, -1.0f, 0.0f) },

		{ Vector3(-1.0f, -1.0f, 1.0f),	Vector3(-1.0f, 0.0f, 0.0f) },
		{ Vector3(-1.0f, -1.0f, -1.0f), Vector3(-1.0f, 0.0f, 0.0f) },
		{ Vector3(-1.0f, 1.0f, -1.0f),	Vector3(-1.0f, 0.0f, 0.0f) },
		{ Vector3(-1.0f, 1.0f, 1.0f),	Vector3(-1.0f, 0.0f, 0.0f) },

		{ Vector3(1.0f, -1.0f, 1.0f),	Vector3(1.0f, 0.0f, 0.0f) },
		{ Vector3(1.0f, -1.0f, -1.0f),	Vector3(1.0f, 0.0f, 0.0f) },
		{ Vector3(1.0f, 1.0f, -1.0f),	Vector3(1.0f, 0.0f, 0.0f) },
		{ Vector3(1.0f, 1.0f, 1.0f),	Vector3(1.0f, 0.0f, 0.0f) },

		{ Vector3(-1.0f, -1.0f, -1.0f), Vector3(0.0f, 0.0f, -1.0f) },
		{ Vector3(1.0f, -1.0f, -1.0f),	Vector3(0.0f, 0.0f, -1.0f) },
		{ Vector3(1.0f, 1.0f, -1.0f),	Vector3(0.0f, 0.0f, -1.0f) },
		{ Vector3(-1.0f, 1.0f, -1.0f),	Vector3(0.0f, 0.0f, -1.0f) },

		{ Vector3(-1.0f, -1.0f, 1.0f),	Vector3(0.0f, 0.0f, 1.0f) },
		{ Vector3(1.0f, -1.0f, 1.0f),	Vector3(0.0f, 0.0f, 1.0f) },
		{ Vector3(1.0f, 1.0f, 1.0f),	Vector3(0.0f, 0.0f, 1.0f) },
		{ Vector3(-1.0f, 1.0f, 1.0f),	Vector3(0.0f, 0.0f, 1.0f) },
	};

	D3D11_BUFFER_DESC bd = {};
	bd.ByteWidth = sizeof(Vertex) * ARRAYSIZE(vertices);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0;

	// 배열 데이터 할당.
	D3D11_SUBRESOURCE_DATA vbData;
	ZeroMemory(&vbData, sizeof(vbData));
	vbData.pSysMem = vertices;

	// 버텍스 버퍼 생성.
	hr = pDevice->CreateBuffer(&bd, &vbData, &vertexBuffer);
	if (FAILED(hr))
	{
		MessageBox(m_hWnd, L"정점 버퍼 생성 실패.", L"오류.", MB_OK);
		return false;
	}

	// 버텍스 버퍼 바인딩.
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	pDeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);


	//5. 인덱스 버퍼
	// 인덱스 버퍼 설정.
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

	// 인덱스 개수 저장.
	nIndices = ARRAYSIZE(indices);

	bd.ByteWidth = sizeof(DWORD) * ARRAYSIZE(indices);
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA ibData;
	ZeroMemory(&ibData, sizeof(D3D11_SUBRESOURCE_DATA));
	ibData.pSysMem = indices;

	// 인덱스 버퍼 생성.
	hr = pDevice->CreateBuffer(&bd, &ibData, &g_pIndexBuffer);
	if (FAILED(hr))
	{
		MessageBox(m_hWnd, L"인덱스 버퍼 생성 실패.", L"오류.", MB_OK);
		return false;
	}

	// 인덱스 버퍼 바인딩(binding).
	pDeviceContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	// 입력 레이아웃.
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

	// 입력 레이아웃 생성.
	hr = pDevice->CreateInputLayout(layout, ARRAYSIZE(layout),
		vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &vertexInputLayout);

	if (FAILED(hr))
	{
		MessageBox(m_hWnd, L"입력 레이아웃 생성 실패.", L"오류.", MB_OK);
		return false;
	}

	// 입력 레이아웃 바인딩.
	pDeviceContext->IASetInputLayout(vertexInputLayout);


	// 정점을 이어서 그릴 방식 설정.
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Create the constant buffer
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = pDevice->CreateBuffer(&bd, nullptr, &g_pConstantBuffer);
	if (FAILED(hr))
	{
		MessageBox(m_hWnd, L"상수버퍼 생성 실패.", L"오류.", MB_OK);
		return false;
	}

	// Initialize the world matrix
	g_World1 = XMMatrixIdentity();
	g_World2 = XMMatrixIdentity();

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
	SAFE_RELEASE(g_pDepthStencil);
	SAFE_RELEASE(g_pDepthStencilView);
}
