#include "TutorialApp.h"
#include "../Common/Helper.h"
#include <d3dcompiler.h>
#include <Directxtk/DDSTextureLoader.h>

#pragma comment (lib, "d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <dxgidebug.h>

// 정점 선언.
struct Vertex
{
	Vector3 Pos;		// 정점 위치 정보.
	Vector2 Tex;
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
	UninitScene();
	UninitImGUI();
	UninitD3D();
}

bool TutorialApp::Initialize(UINT Width, UINT Height)
{
	__super::Initialize(Width, Height);

	if (!InitD3D())
		return false;

	if (!InitImGUI())
		return false;

	if (!InitScene())
		return false;

	return true;
}

void TutorialApp::Update()
{
	__super::Update();

	float t = GameTimer::m_Instance->TotalTime();
	// Rotate cube around the origin
	m_World = XMMatrixRotationY(t);

	// Modify the color
	m_vMeshColor.x = (sinf(t * 1.0f) + 1.0f) * 0.5f;
	m_vMeshColor.y = (cosf(t * 3.0f) + 1.0f) * 0.5f;
	m_vMeshColor.z = (sinf(t * 5.0f) + 1.0f) * 0.5f;

	m_Camera.GetViewMatrix(m_View);

	Vector3 Pos = m_Camera.m_Position + m_Camera.GetForward() * 10;
	m_World2 = m_Camera.m_World;
	m_World2.Translation(Pos);
}

void TutorialApp::Render()
{
	float color[4] = { 0.0f, 0.5f, 0.5f, 1.0f };
	ConstantBuffer cb1;
	cb1.mView = XMMatrixTranspose(m_View);
	cb1.mProjection = XMMatrixTranspose(m_Projection);
	
	// Clear the back buffer
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, color);
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	if (m_bTestStencilBuffer)
	{
		// Write Stencil Buffer
		m_pDeviceContext->OMSetDepthStencilState(m_pDepthStencilStateWrite, 1);
		m_pDeviceContext->OMSetRenderTargets(0, nullptr, m_pDepthStencilView);

		m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_VertexBufferStride, &m_VertexBufferOffset);
		m_pDeviceContext->IASetInputLayout(m_pInputLayout);
		m_pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
		m_pDeviceContext->VSSetShader(m_pVertexShader, nullptr, 0);
		m_pDeviceContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
		m_pDeviceContext->PSSetShader(nullptr, nullptr, 0);

		cb1.mWorld = XMMatrixTranspose(m_World2);
		m_pDeviceContext->UpdateSubresource(m_pConstantBuffer, 0, nullptr, &cb1, 0, 0);
		m_pDeviceContext->DrawIndexed(m_nIndices, 0, 0);


		// Read Stencil Buffer
		m_pDeviceContext->OMSetDepthStencilState(m_pDepthStencilStateRead, 1);
		m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);
		//
		m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_VertexBufferStride, &m_VertexBufferOffset);
		m_pDeviceContext->IASetInputLayout(m_pInputLayout);
		m_pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
		m_pDeviceContext->VSSetShader(m_pVertexShader, nullptr, 0);
		m_pDeviceContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
		m_pDeviceContext->PSSetShader(m_pPixelShader, nullptr, 0);
		m_pDeviceContext->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);
		m_pDeviceContext->PSSetShaderResources(0, 1, &m_pTextureRV);
		m_pDeviceContext->PSSetSamplers(0, 1, &m_pSamplerLinear);


		cb1.mWorld = XMMatrixTranspose(m_World);
		m_pDeviceContext->UpdateSubresource(m_pConstantBuffer, 0, nullptr, &cb1, 0, 0);
		m_pDeviceContext->DrawIndexed(m_nIndices, 0, 0);
	}
	else
	{
		m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);
		m_pDeviceContext->OMSetDepthStencilState(nullptr, 1);

		m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_VertexBufferStride, &m_VertexBufferOffset);
		m_pDeviceContext->IASetInputLayout(m_pInputLayout);
		m_pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
		m_pDeviceContext->VSSetShader(m_pVertexShader, nullptr, 0);
		m_pDeviceContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
		m_pDeviceContext->PSSetShader(m_pPixelShader, nullptr, 0);
		m_pDeviceContext->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);
		m_pDeviceContext->PSSetShaderResources(0, 1, &m_pTextureRV);
		m_pDeviceContext->PSSetSamplers(0, 1, &m_pSamplerLinear);

		cb1.mWorld = XMMatrixTranspose(m_World2);
		m_pDeviceContext->UpdateSubresource(m_pConstantBuffer, 0, nullptr, &cb1, 0, 0);
		m_pDeviceContext->DrawIndexed(m_nIndices, 0, 0);

		cb1.mWorld = XMMatrixTranspose(m_World);
		m_pDeviceContext->UpdateSubresource(m_pConstantBuffer, 0, nullptr, &cb1, 0, 0);
		m_pDeviceContext->DrawIndexed(m_nIndices, 0, 0);
	}

	RenderImGUI();
	m_pSwapChain->Present(0, 0);	// Present our back buffer to our front buffer
}

bool TutorialApp::InitD3D()
{
	HRESULT hr = 0;	// 결과값.

	// 스왑체인 속성 설정 구조체 생성.
	DXGI_SWAP_CHAIN_DESC swapDesc = {};
	swapDesc.BufferCount = 1;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.OutputWindow = m_hWnd;	// 스왑체인 출력할 창 핸들 값.
	swapDesc.Windowed = true;		// 창 모드 여부 설정.
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

	UINT creationFlags = 0;
#ifdef _DEBUG
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	// 1. 장치 생성.   2.스왑체인 생성. 3.장치 컨텍스트 생성.
	HR_T(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, creationFlags, NULL, NULL,
		D3D11_SDK_VERSION, &swapDesc, &m_pSwapChain, &m_pDevice, NULL, &m_pDeviceContext));

	// 4. 렌더타겟뷰 생성.  (백버퍼를 이용하는 렌더타겟뷰)	
	ID3D11Texture2D* pBackBufferTexture = nullptr;
	HR_T(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBufferTexture));
	HR_T(m_pDevice->CreateRenderTargetView(pBackBufferTexture, NULL, &m_pRenderTargetView));  // 텍스처는 내부 참조 증가
	SAFE_RELEASE(pBackBufferTexture);	//외부 참조 카운트를 감소시킨다.
	// 렌더 타겟을 최종 출력 파이프라인에 바인딩합니다.
	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, NULL);

	//5. 뷰포트 설정.	
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)m_ClientWidth;
	viewport.Height = (float)m_ClientHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	m_pDeviceContext->RSSetViewports(1, &viewport);

	//6. 뎊스&스텐실 뷰 생성
	D3D11_TEXTURE2D_DESC descDepth = {};
	descDepth.Width = m_ClientWidth;
	descDepth.Height = m_ClientHeight;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_R24G8_TYPELESS;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	HR_T(m_pDevice->CreateTexture2D(&descDepth, nullptr, &m_pTextureDepthStencil));

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
	descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // Depth 24-bit + Stencil 8-bit;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	HR_T(m_pDevice->CreateDepthStencilView(m_pTextureDepthStencil, &descDSV, &m_pDepthStencilView));

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS; // 뎁스 값만 읽음
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	HR_T(m_pDevice->CreateShaderResourceView(m_pTextureDepthStencil, &srvDesc, &m_pDepthSRV));

	srvDesc.Format = DXGI_FORMAT_X24_TYPELESS_G8_UINT; // 스텐실 값만 읽음
	HR_T(m_pDevice->CreateShaderResourceView(m_pTextureDepthStencil, &srvDesc, &m_pStencilSRV));



	// 7. 스텐실 쓰기를 위한 상태 설정
	D3D11_DEPTH_STENCIL_DESC stencilDescWrite = {};
	stencilDescWrite.DepthEnable = TRUE; // 깊이 버퍼 사용
	stencilDescWrite.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	stencilDescWrite.DepthFunc = D3D11_COMPARISON_ALWAYS;

	stencilDescWrite.StencilEnable = TRUE; // 스텐실 사용
	stencilDescWrite.StencilWriteMask = 0xFF; // 스텐실의 모든 비트 쓰기 가능
	stencilDescWrite.StencilReadMask = 0xFF;  // 스텐실의 모든 비트 읽기 가능

	// 전면 페이스에서의 스텐실 연산
	stencilDescWrite.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP; // 스텐실 테스트 실패 시 유지
	stencilDescWrite.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP; // 깊이 테스트 실패 시 유지
	stencilDescWrite.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE; // 테스트 통과 시 교체
	stencilDescWrite.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS; // 항상 통과

	// 후면 페이스도 동일하게 설정
	stencilDescWrite.BackFace = stencilDescWrite.FrontFace;
	m_pDevice->CreateDepthStencilState(&stencilDescWrite, &m_pDepthStencilStateWrite);

	// 7. 스텐실 읽기(테스트)를 위한 상태 설정
	D3D11_DEPTH_STENCIL_DESC stencilDescRead = {};
	stencilDescRead.DepthEnable = TRUE;
	stencilDescRead.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; // 깊이 쓰기 비활성화
	stencilDescRead.DepthFunc = D3D11_COMPARISON_LESS;

	stencilDescRead.StencilEnable = TRUE;
	stencilDescRead.StencilReadMask = 0xFF; // 읽기 마스크
	stencilDescRead.StencilWriteMask = 0x00; // 쓰기 비활성화

	// 스텐실 값이 1인 경우만 통과
	stencilDescRead.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
	stencilDescRead.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP; // 유지
	stencilDescRead.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP; // 실패 시 유지
	stencilDescRead.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP; // 깊이 실패 시 유지
	stencilDescRead.BackFace = stencilDescRead.FrontFace;
	m_pDevice->CreateDepthStencilState(&stencilDescRead, &m_pDepthStencilStateRead);
	return true;
}

void TutorialApp::UninitD3D()
{
	// Cleanup DirectX	
	SAFE_RELEASE(m_pDepthSRV);
	SAFE_RELEASE(m_pStencilSRV);
	SAFE_RELEASE(m_pDepthStencilStateWrite);
	SAFE_RELEASE(m_pDepthStencilStateRead);
	SAFE_RELEASE(m_pDevice);
	SAFE_RELEASE(m_pDeviceContext);
	SAFE_RELEASE(m_pSwapChain);
	SAFE_RELEASE(m_pRenderTargetView);
}

// 1. Render() 에서 파이프라인에 바인딩할 버텍스 버퍼및 버퍼 정보 준비
// 2. Render() 에서 파이프라인에 바인딩할 InputLayout 생성 	
// 3. Render() 에서 파이프라인에 바인딩할  버텍스 셰이더 생성
// 4. Render() 에서 파이프라인에 바인딩할 인덱스 버퍼 생성
// 5. Render() 에서 파이프라인에 바인딩할 픽셀 셰이더 생성
// 6. Render() 에서 파이프라인에 바인딩할 상수 버퍼 생성
bool TutorialApp::InitScene()
{
	HRESULT hr=0; // 결과값.
	// 1. Render() 에서 파이프라인에 바인딩할 버텍스 버퍼및 버퍼 정보 준비
	// Local or Object or Model Space
	Vertex vertices[] =
	{
		{ Vector3(-1.0f, 1.0f, -1.0f), Vector2(1.0f, 0.0f) },
		{ Vector3(1.0f, 1.0f, -1.0f), Vector2(0.0f, 0.0f) },
		{ Vector3(1.0f, 1.0f, 1.0f), Vector2(0.0f, 1.0f) },
		{ Vector3(-1.0f, 1.0f, 1.0f), Vector2(1.0f, 1.0f) },

		{ Vector3(-1.0f, -1.0f, -1.0f), Vector2(0.0f, 0.0f) },
		{ Vector3(1.0f, -1.0f, -1.0f), Vector2(1.0f, 0.0f) },
		{ Vector3(1.0f, -1.0f, 1.0f), Vector2(1.0f, 1.0f) },
		{ Vector3(-1.0f, -1.0f, 1.0f), Vector2(0.0f, 1.0f) },

		{ Vector3(-1.0f, -1.0f, 1.0f), Vector2(0.0f, 1.0f) },
		{ Vector3(-1.0f, -1.0f, -1.0f), Vector2(1.0f, 1.0f) },
		{ Vector3(-1.0f, 1.0f, -1.0f), Vector2(1.0f, 0.0f) },
		{ Vector3(-1.0f, 1.0f, 1.0f), Vector2(0.0f, 0.0f) },

		{ Vector3(1.0f, -1.0f, 1.0f), Vector2(1.0f, 1.0f) },
		{ Vector3(1.0f, -1.0f, -1.0f), Vector2(0.0f, 1.0f) },
		{ Vector3(1.0f, 1.0f, -1.0f), Vector2(0.0f, 0.0f) },
		{ Vector3(1.0f, 1.0f, 1.0f), Vector2(1.0f, 0.0f) },

		{ Vector3(-1.0f, -1.0f, -1.0f), Vector2(0.0f, 1.0f) },
		{ Vector3(1.0f, -1.0f, -1.0f), Vector2(1.0f, 1.0f) },
		{ Vector3(1.0f, 1.0f, -1.0f), Vector2(1.0f, 0.0f) },
		{ Vector3(-1.0f, 1.0f, -1.0f), Vector2(0.0f, 0.0f) },

		{ Vector3(-1.0f, -1.0f, 1.0f), Vector2(1.0f, 1.0f) },
		{ Vector3(1.0f, -1.0f, 1.0f), Vector2(0.0f, 1.0f) },
		{ Vector3(1.0f, 1.0f, 1.0f), Vector2(0.0f, 0.0f) },
		{ Vector3(-1.0f, 1.0f, 1.0f), Vector2(1.0f, 0.0f) },
	};

	D3D11_BUFFER_DESC bd = {};
	bd.ByteWidth = sizeof(Vertex) * ARRAYSIZE(vertices);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA vbData = {};
	vbData.pSysMem = vertices;
	HR_T( m_pDevice->CreateBuffer(&bd, &vbData, &m_pVertexBuffer));	

	// 버텍스 버퍼 정보
	m_VertexBufferStride = sizeof(Vertex);
	m_VertexBufferOffset = 0;


	// 2. Render() 에서 파이프라인에 바인딩할 InputLayout 생성 	
	ID3D10Blob* vertexShaderBuffer = nullptr;
	HR_T(CompileShaderFromFile(L"BasicVertexShader.hlsl", "main", "vs_4_0", &vertexShaderBuffer));

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	hr = m_pDevice->CreateInputLayout(layout, ARRAYSIZE(layout),
		vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_pInputLayout);

	// 3. Render() 에서 파이프라인에 바인딩할  버텍스 셰이더 생성
	HR_T(m_pDevice->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), NULL, &m_pVertexShader));
	SAFE_RELEASE(vertexShaderBuffer);

	// 4. Render() 에서 파이프라인에 바인딩할 인덱스 버퍼 생성
	WORD indices[] =
	{
		3,1,0, 2,1,3,
		6,4,5, 7,4,6,
		11,9,8, 10,9,11,
		14,12,13, 15,12,14,
		19,17,16, 18,17,19,
		22,20,21, 23,20,22
	};

	// 인덱스 개수 저장.
	m_nIndices = ARRAYSIZE(indices);

	bd = {};
	bd.ByteWidth = sizeof(WORD) * ARRAYSIZE(indices);
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA ibData = {};
	ibData.pSysMem = indices;
	HR_T(m_pDevice->CreateBuffer(&bd, &ibData, &m_pIndexBuffer));


	// 5. Render() 에서 파이프라인에 바인딩할 픽셀 셰이더 생성
	ID3D10Blob* pixelShaderBuffer = nullptr;
	HR_T(CompileShaderFromFile(L"BasicPixelShader.hlsl", "main", "ps_4_0", &pixelShaderBuffer));
	HR_T(m_pDevice->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
		pixelShaderBuffer->GetBufferSize(), NULL, &m_pPixelShader));
	SAFE_RELEASE(pixelShaderBuffer);


	// 6. Render() 에서 파이프라인에 바인딩할 상수 버퍼 생성
	// Create the constant buffer
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	HR_T( m_pDevice->CreateBuffer(&bd, nullptr, &m_pConstantBuffer));

	// Load the Texture
	HR_T( CreateDDSTextureFromFile(m_pDevice, L"seafloor.dds", nullptr, &m_pTextureRV));

	// Create the sample state
	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	HR_T( m_pDevice->CreateSamplerState(&sampDesc, &m_pSamplerLinear));


	// Initialize the world matrix
	m_World = XMMatrixIdentity();


	// Initialize the view matrix
	XMVECTOR Eye = XMVectorSet(0.0f, 3.0f, -6.0f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	m_View = XMMatrixLookAtLH(Eye, At, Up);

	// Initialize the projection matrix
	m_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, m_ClientWidth / (FLOAT)m_ClientHeight, 0.01f, 100.0f);

	return true;
}

void TutorialApp::UninitScene()
{
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pVertexShader);
	SAFE_RELEASE(m_pPixelShader);
	SAFE_RELEASE(m_pInputLayout);
	SAFE_RELEASE(m_pIndexBuffer);
	SAFE_RELEASE(m_pDepthStencilView);
}


bool TutorialApp::InitImGUI()
{
	/*
		ImGui 초기화.
	*/
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();


	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(m_hWnd);
	ImGui_ImplDX11_Init(this->m_pDevice, this->m_pDeviceContext);

	//
	return true;
}

void TutorialApp::UninitImGUI()
{
	// Cleanup
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK TutorialApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
		return true;

	return __super::WndProc(hWnd, message, wParam, lParam);
}



void TutorialApp::RenderImGUI()
{
	/////////////////
	//아래부터는 ImGUI
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Test");                       
	ImGui::Checkbox("Test Stencil",&m_bTestStencilBuffer);
	ImGui::Image((void*)m_pDepthSRV, ImVec2(128, 128));
	ImGui::Image((void*)m_pStencilSRV, ImVec2(128, 128));
	ImGui::End();

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}