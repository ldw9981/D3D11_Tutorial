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

	// 화면 칠하기.
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

	//3. 렌더 타겟을 최종 출력 파이프라인에 바인딩합니다.
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
	HRESULT hr; // 결과값.
	ID3D10Blob* errorMessage = nullptr;	 // 에러 메시지를 저장할 버퍼.
	
	//////////////////////////////////////////////////////////////////////////
	// 정점 셰이더	 
	// 1. 정점 셰이더 컴파일해서 정점 셰이더 버퍼에 저장.
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
		return false;
	}
	SAFE_RELEASE(errorMessage);	// 에러 메세지 더이상 필요없음

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
		NULL,NULL,
		"main",		// 시작 함수 이름
		"ps_4_0",	// 정점 셰이더 버전.
		NULL,NULL,
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

	// 정점 데이터(배열) 생성.
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

	// 배열 데이터 할당.
	D3D11_SUBRESOURCE_DATA vbData;
	ZeroMemory(&vbData, sizeof(vbData));
	vbData.pSysMem = vertices;

	// 정점 버퍼 생성.
	hr = pDevice->CreateBuffer(&vbDesc, &vbData, &vertexBuffer);
	if (FAILED(hr))
	{
		MessageBox(m_hWnd, L"정점 버퍼 생성 실패.", L"오류.", MB_OK);
		return false;
	}

	UINT stride = sizeof(Vector3);
	UINT offset = 0;

	// 정점 버퍼 바인딩.
	pDeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

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
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
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
