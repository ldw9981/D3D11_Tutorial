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

// 정점 선언.
struct Vertex
{
	Vector3 position;		// 위치 정보.
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

	// Flip모드에서는 매프레임 설정 필요
	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, NULL);

	// 화면 칠하기.
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, color);

	// Draw계열 함수를 호출하기전에 렌더링 파이프라인에 필수 스테이지 설정을 해야한다.	
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // 정점을 이어서 그릴 방식 설정.
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

	// 1. D3D11 Device,DeviceContext 생성
	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	// 그래픽 카드 하드웨어의 스펙으로 호환되는 가장 높은 DirectX 기능레벨로 생성하여 드라이버가 작동 한다.
	// 인터페이스는 Direc3D11 이지만 GPU드라이버는 D3D12 드라이버가 작동할수도 있다.
	D3D_FEATURE_LEVEL featureLevels[] = { // index 0부터 순서대로 시도한다.
		D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0,D3D_FEATURE_LEVEL_11_1,D3D_FEATURE_LEVEL_11_0
	};
	D3D_FEATURE_LEVEL actualFeatureLevel; // 최종 피처 레벨을 저장할 변수

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

	// 2. 스왑체인 생성을 위한 DXGI Factory 생성
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
	// 하나의 픽셀이 채널 RGBA 각 8비트 형식으로 표현되며 
	// Unsigned Normalized Integer 8비트 정수(0~255)단계를 부동소수점으로 정규화한 0.0~1.0으로 매핑하여 표현한다.
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // 스왑 체인의 백 버퍼가 렌더링 파이프라인의 최종 출력 대상으로 사용
	swapChainDesc.SampleDesc.Count = 1;  // 멀티샘플링 사용 안함
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE; // Recommended for flip models
	swapChainDesc.Stereo = FALSE;  // 스테레오 3D 렌더링을 비활성화
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // 전체 화면 전환을 허용
	swapChainDesc.Scaling = DXGI_SCALING_NONE; //  창의 크기와 백 버퍼의 크기가 다를 때. 백버퍼 크기에 맞게 스케일링 하지 않는다.

	HR_T(pFactory->CreateSwapChainForHwnd(
		m_pDevice,
		m_hWnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		&m_pSwapChain
	));

	// 3. 렌더타겟 뷰 생성.  렌더 타겟 뷰는 "여기다가 그림을 그려라"라고 GPU에게 알려주는 역할을 하는 객체.
	// 텍스처와 영구적 연결되는 객체이다. 
	ComPtr<ID3D11Texture2D> pBackBufferTexture;
	HR_T(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBufferTexture));
	HR_T(m_pDevice->CreateRenderTargetView(pBackBufferTexture.Get(), nullptr, &m_pRenderTargetView));

	// 뷰포트 설정.	
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
	HRESULT hr=0; // 결과값.
	ID3D10Blob* errorMessage = nullptr;	 // 컴파일 에러 메시지가 저장될 버퍼.	

	//1. Render() 에서 파이프라인에 바인딩할 버텍스 버퍼및 버퍼 정보 준비
	// 아직은 VertexShader의 World, View, Projection 변환을 사용하지 않으므로 
	// 직접 Normalized Device Coordinate(좌표계)의 위치로 설정한다.
	//      /---------------------(1,1,1)   z값은 깊이값
	//     /                      / |   
	// (-1,1,0)----------------(1,1,0)        
	//   |         v1           |   |
	//   |        /   `         |   |       중앙이 (0,0,0)  
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
	m_VertexCount = ARRAYSIZE(vertices);	// 정점의 수
	vbDesc.ByteWidth = sizeof(Vertex) * m_VertexCount; // 버텍스 버퍼의 크기(Byte).
	vbDesc.CPUAccessFlags = 0;
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER; // 정점 버퍼로 사용.
	vbDesc.MiscFlags = 0;
	vbDesc.Usage = D3D11_USAGE_DEFAULT;	// CPU는 접근불가 ,  GPU에서 읽기/쓰기 가능한 버퍼로 생성.

	// 정점 버퍼 생성.
	D3D11_SUBRESOURCE_DATA vbData = {};
	vbData.pSysMem = vertices;	// 버퍼를 생성할때 복사할 데이터의 주소 설정 
	HR_T(hr = m_pDevice->CreateBuffer(&vbDesc, &vbData, &m_pVertexBuffer));

	// 버텍스 버퍼 정보 
	m_VertextBufferStride = sizeof(Vertex); // 버텍스 하나의 크기
	m_VertextBufferOffset = 0;	// 버텍스 시작 주소에서 더할 오프셋 주소
	
	// 2. Render에서 파이프라인에 바인딩할  버텍스 셰이더 생성
	ID3DBlob* vertexShaderBuffer = nullptr; // 버텍스 세이더 HLSL의 컴파일된 결과(바이트코드)를 담을수 있는 버퍼 객체
	HR_T(CompileShaderFromFile(L"BasicVertexShader.hlsl", "main", "vs_4_0", &vertexShaderBuffer));
	HR_T(m_pDevice->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), // 필요한 데이터를 복사하며 객체 생성 
		vertexShaderBuffer->GetBufferSize(), NULL, &m_pVertexShader));
	
	// 3. Render() 에서 파이프라인에 바인딩할 InputLayout 생성 	
	D3D11_INPUT_ELEMENT_DESC layout[] =  // 인풋 레이아웃은 버텍스 쉐이더가 입력받을 데이터의 형식을 지정한다.
	{// SemanticName , SemanticIndex , Format , InputSlot , AlignedByteOffset , InputSlotClass , InstanceDataStepRate		
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	// 버텍스 셰이더의 Input에 지정된 내용과 같은지 검증하면서 InputLayout을 생성한다.
	HR_T(hr = m_pDevice->CreateInputLayout(layout, ARRAYSIZE(layout),
		vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_pInputLayout));	

	SAFE_RELEASE(vertexShaderBuffer); // 복사했으니 버퍼는 해제 가능


	// 4. Render에서 파이프라인에 바인딩할 픽셀 셰이더 생성
	ID3DBlob* pixelShaderBuffer = nullptr; // 픽셀 세이더 HLSL의 컴파일된 결과(바이트코드)를 담을수 있는 버퍼 객체
	HR_T(CompileShaderFromFile(L"BasicPixelShader.hlsl", "main", "ps_4_0", &pixelShaderBuffer));	
	HR_T( m_pDevice->CreatePixelShader(	  // 필요한 데이터를 복사하며 객체 생성 
		pixelShaderBuffer->GetBufferPointer(),
		pixelShaderBuffer->GetBufferSize(), NULL, &m_pPixelShader));
	SAFE_RELEASE(pixelShaderBuffer); // 복사했으니 버퍼는 해제 가능
	
	return true;
}

void TutorialApp::UninitScene()
{
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pInputLayout);
	SAFE_RELEASE(m_pVertexShader);
	SAFE_RELEASE(m_pPixelShader);
}
