#include "TutorialApp.h"
#include "../Common/Helper.h"
#include <d3dcompiler.h>

#pragma comment (lib, "d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")


// 정점 선언.
struct Vertex
{
	Vector3 position;		// 정점 위치 정보.
	Vector4 color;			// 정점 색상 정보.

	Vertex(float x, float y, float z) : position(x, y, z) { }
	Vertex(Vector3 position) : position(position) { }

	Vertex(Vector3 position, Vector4 color)
		: position(position), color(color) { }
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

}

void TutorialApp::Render()
{
	float color[4] = { 0.0f, 0.5f, 0.5f, 1.0f };

	// 화면 칠하기.
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, color);


	// 정점 그리기. (Draw Call. 드로우 콜).
	//pDeviceContext->Draw(nVertices, 0);
	m_pDeviceContext->DrawIndexed(nIndices, 0, 0);

	// Present the information rendered to the back buffer to the front buffer (the screen)
	m_pSwapChain->Present(0, 0);
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
		D3D11_SDK_VERSION, &swapDesc, &m_pSwapChain, &m_pDevice, NULL, &m_pDeviceContext);
	if (FAILED(hr)) {
		LOG_ERROR(L"%s", GetComErrorString(hr));
		return false;
	}
	
	// 2. 렌더타겟뷰 생성.
	// 스왑체인의 내부의 백버퍼를 얻습니다. 
	ID3D11Texture2D* pBackBufferTexture;
	hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBufferTexture);
	if (FAILED(hr)) {
		LOG_ERROR(L"%s", GetComErrorString(hr));
		return false;
	}

	// 스왑체인의 백버퍼를 이용하는 렌더타겟뷰를 생성합니다.
	hr = m_pDevice->CreateRenderTargetView(
		pBackBufferTexture, NULL, &m_pRenderTargetView);
	// 렌더타겟뷰를 만들었으므로 백버퍼 텍스처 인터페이스는 더이상 필요하지 않습니다.
	SAFE_RELEASE(pBackBufferTexture);
	if (FAILED(hr)) {
		LOG_ERROR(L"%s", GetComErrorString(hr));
		return false;
	}

	//3. 렌더 타겟을 최종 출력 파이프라인에 바인딩합니다.
	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, NULL);
	return true;
}

void TutorialApp::UninitD3D()
{
	// Cleanup DirectX
	SAFE_RELEASE(m_pDevice);
	SAFE_RELEASE(m_pDeviceContext);
	SAFE_RELEASE(m_pSwapChain);
	SAFE_RELEASE(m_pRenderTargetView);
}

bool TutorialApp::InitScene()
{	
	HRESULT hr; // 결과값.
	ID3D10Blob* errorMessage = nullptr;	 // 에러 메시지를 저장할 버퍼.
	
	//////////////////////////////////////////////////////////////////////////
	// 버텍스 셰이더	 
	// 1. 버텍스 셰이더 컴파일해서 버텍스 셰이더 버퍼에 저장.
	ID3D10Blob* vertexShaderBuffer = nullptr;
	hr = D3DCompileFromFile(L"BasicVertexShader.hlsl",	// 셰이더 파일 이름.
		NULL,NULL,
		"main",	// 시작 함수 이름
		"vs_4_0", // 정점 셰이더 버전.
		NULL,NULL,	
		&vertexShaderBuffer, // 컴파일된 셰이더 코드가 저장될 버퍼.
		&errorMessage);	// 컴파일 에러 메시지가 저장될 버퍼.

	if (FAILED(hr))
	{
		MessageBoxA(m_hWnd,(char*)errorMessage->GetBufferPointer() ,"오류.", MB_OK);		
		SAFE_RELEASE(errorMessage);	// 에러 메세지 더이상 필요없음
		return false;
	}

	// 2. 버텍스 셰이더 리소스 생성.
	hr = m_pDevice->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), NULL, &m_pVertexShader);

	if (FAILED(hr)) {
		LOG_ERROR(L"%s", GetComErrorString(hr));
		return false;
	}

	// 3. 정점 셰이더 단계에 바인딩(설정, 연결)binding.
	m_pDeviceContext->VSSetShader(m_pVertexShader, NULL, NULL);

	//////////////////////////////////////////////////////////////////////////
	// 픽셀 셰이더 
	// 1. 컴파일.
	ID3D10Blob* pixelShaderBuffer = nullptr;
	hr = D3DCompileFromFile(L"BasicPixelShader.hlsl", // 셰이더 파일 이름.
		NULL,NULL,
		"main",		// 시작 함수 이름
		"ps_4_0",	// 정점 셰이더 버전.
		NULL,NULL,
		&pixelShaderBuffer, // 컴파일된 셰이더 코드가 저장될 버퍼.
		&errorMessage);		// 컴파일 에러 메시지가 저장될 버퍼.
	
	if (FAILED(hr))
	{
		MessageBoxA(m_hWnd, (char*)errorMessage->GetBufferPointer(), "오류.", MB_OK);
		SAFE_RELEASE(errorMessage);	// 에러 메세지 더이상 필요없음
		return false;
	}
	// 2. 픽셀 셰이더 생성.
	hr = m_pDevice->CreatePixelShader(
		pixelShaderBuffer->GetBufferPointer(),
		pixelShaderBuffer->GetBufferSize(), NULL, &m_pPixelShader);
	SAFE_RELEASE(pixelShaderBuffer);	// 픽셀 셰이더 버퍼 더이상 필요없음.

	if (FAILED(hr)) {
		LOG_ERROR(L"%s", GetComErrorString(hr));
		return false;
	}

	//3. 픽셀 셰이더 설정.
	m_pDeviceContext->PSSetShader(m_pPixelShader, NULL, NULL);	


	//4. 버텍스 버퍼 
	// 버텍스 데이터(배열) 생성.
	// Normalized Device Coordinate
	//   0-----1
	//   |    /|
	//   |  /  |                중앙이 (0,0)  왼쪽이 (-1,0) 오른쪽이 (1,0) , 위쪽이 (0,1) 아래쪽이 (0,-1)
	//   |/    |
	//	 2-----3
	Vertex vertices[] =
	{
		Vertex(Vector3(-0.5f,  0.5f, 0.5f), Vector4(1.0f, 0.0f, 0.0f, 1.0f)),  // 
		Vertex(Vector3( 0.5f,  0.5f, 0.5f), Vector4(0.0f, 1.0f, 0.0f, 1.0f)),
		Vertex(Vector3(-0.5f, -0.5f, 0.5f), Vector4(0.0f, 0.0f, 1.0f, 1.0f)),
		Vertex(Vector3( 0.5f, -0.5f, 0.5f), Vector4(0.0f, 0.0f, 1.0f, 1.0f))
	};

	D3D11_BUFFER_DESC vbDesc;
	ZeroMemory(&vbDesc, sizeof(D3D11_BUFFER_DESC));
	// sizeof(vertices) / sizeof(Vertex).
	vbDesc.ByteWidth = sizeof(Vertex) * ARRAYSIZE(vertices);
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.Usage = D3D11_USAGE_DEFAULT;

	// 배열 데이터 할당.
	D3D11_SUBRESOURCE_DATA vbData;
	ZeroMemory(&vbData, sizeof(vbData));
	vbData.pSysMem = vertices;

	// 버텍스 버퍼 생성.
	hr = m_pDevice->CreateBuffer(&vbDesc, &vbData, &m_pVertexBuffer);
	if (FAILED(hr))
	{
		MessageBox(m_hWnd, L"정점 버퍼 생성 실패.", L"오류.", MB_OK);
		return false;
	}

	// 버텍스 버퍼 바인딩.
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);


	//5. 인덱스 버퍼
	// 인덱스 버퍼 설정.
	DWORD indices[] =
	{
		0, 1, 2,
		2, 1, 3
	};

	// 인덱스 개수 저장.
	nIndices = ARRAYSIZE(indices);

	D3D11_BUFFER_DESC ibDesc;
	ZeroMemory(&ibDesc, sizeof(D3D11_BUFFER_DESC));
	ibDesc.ByteWidth = sizeof(DWORD) * ARRAYSIZE(indices);
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibDesc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA ibData;
	ZeroMemory(&ibData, sizeof(D3D11_SUBRESOURCE_DATA));
	ibData.pSysMem = indices;

	// 인덱스 버퍼 생성.
	hr = m_pDevice->CreateBuffer(&ibDesc, &ibData, &m_pIndexBuffer);
	if (FAILED(hr))
	{
		MessageBox(m_hWnd, L"인덱스 버퍼 생성 실패.", L"오류.", MB_OK);
		return false;
	}

	// 인덱스 버퍼 바인딩(binding).
	m_pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

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
	hr = m_pDevice->CreateInputLayout(layout, ARRAYSIZE(layout),
		vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_pInputLayout);
	SAFE_RELEASE(vertexShaderBuffer);	// 버퍼 해제.

	if (FAILED(hr))
	{
		MessageBox(m_hWnd, L"입력 레이아웃 생성 실패.", L"오류.", MB_OK);
		return false;
	}

	// 입력 레이아웃 바인딩.
	m_pDeviceContext->IASetInputLayout(m_pInputLayout);


	// 정점을 이어서 그릴 방식 설정.
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 뷰포트 설정.	
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)m_ClientWidth;
	viewport.Height = (float)m_ClientHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	// 뷰포트 설정.
	m_pDeviceContext->RSSetViewports(1, &viewport);
	return true;
}

void TutorialApp::UninitScene()
{
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pIndexBuffer);
	SAFE_RELEASE(m_pInputLayout);
	SAFE_RELEASE(m_pVertexShader);
	SAFE_RELEASE(m_pPixelShader);
}
