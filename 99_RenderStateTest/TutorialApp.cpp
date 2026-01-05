#include "TutorialApp.h"
#include "../Common/Helper.h"
#include <d3dcompiler.h>

#pragma comment (lib, "d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")


// 정점 구조.
struct CubeVertex
{
	Vector3 Pos;
	Vector3 Normal;
};


struct CBNeverChanges
{
	// 비워둔 구조체 (사용하지 않지만 레지스터 유지를 위해)
	float padding[4];
};

static_assert((sizeof(CBNeverChanges) % 16) == 0,
	"Constant Buffer size must be 16-byte aligned");

struct CBChangeOnResize
{
	Matrix mProjection;
};

struct CBChangesEveryFrame
{
	Matrix mWorld;
	Matrix mView;
	Vector4 vMeshColor;
	Vector3 vLightDir;
	float padding1;
	Vector3 vLightColor;
	float padding2;
};

bool TutorialApp::OnInitialize()
{
	std::wstring title = L"" PROJECT_NAME;
	SetWindowTitle(title.c_str());

	if (!InitD3D())
		return false;

	if (!InitScene())
		return false;

	if (!InitRenderStates())
		return false;

	if (!InitImGUI())
		return false;

	return true;
}

void TutorialApp::OnUninitialize()
{
	UninitImGUI();
	UninitRenderStates();
	UninitScene();
	UninitD3D();
}

void TutorialApp::OnUpdate()
{	
	float t = GameTimer::m_Instance->TotalTime();
	float dt = GameTimer::m_Instance->DeltaTime();
	
	// Update Camera
	m_Camera.Update(dt);
	m_Camera.GetViewMatrix(m_View);
	
	// Rotate cube around the origin
	if (m_bRotateAnimation)
	{
		m_World1 = XMMatrixRotationY(t);
		m_World2 = XMMatrixRotationY(t) * XMMatrixTranslation(0.0f, 0.0f, 10.0f);
		m_World3 = XMMatrixRotationY(t) * XMMatrixTranslation(0.0f, 0.0f, 20.0f);
	}
	else
	{
		m_World2 = XMMatrixTranslation(0.0f, 0.0f, 10.0f);
		m_World3 = XMMatrixTranslation(0.0f, 0.0f, 20.0f);
	}
}

void TutorialApp::OnRender()
{	
	// 렌더링 설정
	ID3D11RenderTargetView* rtvs[] = { m_pRenderTargetView.Get() };
	m_pDeviceContext->OMSetRenderTargets(1, rtvs, m_pDepthStencilView.Get());

	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView.Get(), (float*)&m_vClearColor);
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	// Set Render States
	m_pDeviceContext->RSSetState(m_pRasterizerState.Get());
	m_pDeviceContext->OMSetDepthStencilState(m_pDepthStencilState.Get(), 1);
	m_pDeviceContext->OMSetBlendState(m_pBlendState.Get(), nullptr, 0xffffffff);

	// Render the cube
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	ID3D11Buffer* vbs[] = { m_pVertexBuffer.Get() };
	m_pDeviceContext->IASetVertexBuffers(0, 1, vbs, &m_VertexBufferStride, &m_VertexBufferOffset);
	m_pDeviceContext->IASetInputLayout(m_pInputLayout.Get());
	m_pDeviceContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	m_pDeviceContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0);
	ID3D11Buffer* cbs0[] = { m_pCBNeverChanges.Get() };
	m_pDeviceContext->VSSetConstantBuffers(0, 1, cbs0);
	ID3D11Buffer* cbs1[] = { m_pCBChangeOnResize.Get() };
	m_pDeviceContext->VSSetConstantBuffers(1, 1, cbs1);
	ID3D11Buffer* cbs2[] = { m_pCBChangesEveryFrame.Get() };
	m_pDeviceContext->VSSetConstantBuffers(2, 1, cbs2);
	m_pDeviceContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0);
	m_pDeviceContext->PSSetConstantBuffers(2, 1, cbs2);

	// Draw cubes in the order specified by m_DrawOrder
	CBChangesEveryFrame cb;
	cb.mView = XMMatrixTranspose(m_View);
	cb.vLightDir = m_vLightDir;
	cb.vLightColor = m_vLightColor;

	for (int i = 0; i < 3; i++)
	{
		int cubeIndex = m_DrawOrder[i];
		
		switch(cubeIndex)
		{
		case 0: // Cube 1
			cb.mWorld = XMMatrixTranspose(m_World1);
			cb.vMeshColor = m_vMeshColor1;
			break;
		case 1: // Cube 2
			cb.mWorld = XMMatrixTranspose(m_World2);
			cb.vMeshColor = m_vMeshColor2;
			break;
		case 2: // Cube 3
			cb.mWorld = XMMatrixTranspose(m_World3);
			cb.vMeshColor = m_vMeshColor3;
			break;
		}
		
		m_pDeviceContext->UpdateSubresource(m_pCBChangesEveryFrame.Get(), 0, nullptr, &cb, 0, 0);
		m_pDeviceContext->DrawIndexed(m_nIndexCount, 0, 0);
	}

	// Depth 텍스처를 ImGui에서 SRV로 샘플링하려면, 여기서 DSV 바인딩을 먼저 해제해야 합니다.
	// (같은 리소스를 DSV + SRV로 동시에 바인딩하는 것은 D3D11에서 금지)
	m_pDeviceContext->OMSetRenderTargets(1, rtvs, nullptr);

	// Render ImGUI
	RenderImGUI();

	// 다음 프레임에 depth 텍스처를 DSV로 다시 바인딩할 수 있도록 SRV 바인딩도 해제
	ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
	m_pDeviceContext->PSSetShaderResources(0, 1, nullSRV);

	// Present our back buffer to our front buffer
	m_pSwapChain->Present(0, 0);
}

bool TutorialApp::InitD3D()
{
	HRESULT hr = 0;

	// 스왑체인 속성을 담는 구조체 생성.
	DXGI_SWAP_CHAIN_DESC swapDesc = {};
	swapDesc.BufferCount = 2;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.OutputWindow = m_hWnd;
	swapDesc.Windowed = true;
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapDesc.BufferDesc.Width = m_ClientWidth;
	swapDesc.BufferDesc.Height = m_ClientHeight;
	swapDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapDesc.SampleDesc.Count = 1;
	swapDesc.SampleDesc.Quality = 0;
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	UINT creationFlags = 0;
#ifdef _DEBUG
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	// 1. 디바이스 생성.   2.스왑체인 생성. 3.디바이스 컨텍스트 생성.
	HR_T(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, creationFlags, NULL, NULL,
		D3D11_SDK_VERSION, &swapDesc, m_pSwapChain.GetAddressOf(), m_pDevice.GetAddressOf(), NULL, m_pDeviceContext.GetAddressOf()));

	// 4. 렌더타겟뷰 생성.  (백버퍼를 이용하는 렌더타겟뷰)	
	ComPtr<ID3D11Texture2D> pBackBufferTexture;
	HR_T(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)pBackBufferTexture.GetAddressOf()));
	HR_T(m_pDevice->CreateRenderTargetView(pBackBufferTexture.Get(), NULL, m_pRenderTargetView.GetAddressOf()));
	// 렌더 타겟을 최종 출력 파이프라인에 바인딩합니다.
	ID3D11RenderTargetView* rtvs[] = { m_pRenderTargetView.Get() };
	m_pDeviceContext->OMSetRenderTargets(1, rtvs, NULL);

	//5. 뷰포트 설정.	
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)m_ClientWidth;
	viewport.Height = (float)m_ClientHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	m_pDeviceContext->RSSetViewports(1, &viewport);

	//6. 깊이&스텐실 뷰 생성
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

	m_pDepthStencilTexture.Reset();
	HR_T(m_pDevice->CreateTexture2D(&descDepth, nullptr, m_pDepthStencilTexture.GetAddressOf()));

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
	descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	HR_T(m_pDevice->CreateDepthStencilView(m_pDepthStencilTexture.Get(), &descDSV, m_pDepthStencilView.GetAddressOf()));

	// Create SRV for visualization
	D3D11_SHADER_RESOURCE_VIEW_DESC descSRV = {};
	descSRV.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	descSRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	descSRV.Texture2D.MostDetailedMip = 0;
	descSRV.Texture2D.MipLevels = 1;
	m_pDepthStencilSRV.Reset();
	HR_T(m_pDevice->CreateShaderResourceView(m_pDepthStencilTexture.Get(), &descSRV, m_pDepthStencilSRV.GetAddressOf()));

	
	return true;
}

void TutorialApp::UninitD3D()
{
	// ComPtr이 자동으로 메모리 해제를 처리하므로 명시적 해제 불필요
}

bool TutorialApp::InitScene()
{
	HRESULT hr=0;
	
	// 큐브 정점 생성 (각 면마다 다른 색상)
	CubeVertex vertices[] =
	{
		// Top face
		{ Vector3(-1.0f, 1.0f, -1.0f), Vector3(0.0f, 1.0f, 0.0f) },
		{ Vector3(1.0f, 1.0f, -1.0f), Vector3(0.0f, 1.0f, 0.0f) },
		{ Vector3(1.0f, 1.0f, 1.0f), Vector3(0.0f, 1.0f, 0.0f) },
		{ Vector3(-1.0f, 1.0f, 1.0f), Vector3(0.0f, 1.0f, 0.0f) },

		// Bottom face
		{ Vector3(-1.0f, -1.0f, -1.0f), Vector3(0.0f, -1.0f, 0.0f) },
		{ Vector3(1.0f, -1.0f, -1.0f), Vector3(0.0f, -1.0f, 0.0f) },
		{ Vector3(1.0f, -1.0f, 1.0f), Vector3(0.0f, -1.0f, 0.0f) },
		{ Vector3(-1.0f, -1.0f, 1.0f), Vector3(0.0f, -1.0f, 0.0f) },

		// Left face
		{ Vector3(-1.0f, -1.0f, 1.0f), Vector3(-1.0f, 0.0f, 0.0f) },
		{ Vector3(-1.0f, -1.0f, -1.0f), Vector3(-1.0f, 0.0f, 0.0f) },
		{ Vector3(-1.0f, 1.0f, -1.0f), Vector3(-1.0f, 0.0f, 0.0f) },
		{ Vector3(-1.0f, 1.0f, 1.0f), Vector3(-1.0f, 0.0f, 0.0f) },

		// Right face
		{ Vector3(1.0f, -1.0f, 1.0f), Vector3(1.0f, 0.0f, 0.0f) },
		{ Vector3(1.0f, -1.0f, -1.0f), Vector3(1.0f, 0.0f, 0.0f) },
		{ Vector3(1.0f, 1.0f, -1.0f), Vector3(1.0f, 0.0f, 0.0f) },
		{ Vector3(1.0f, 1.0f, 1.0f), Vector3(1.0f, 0.0f, 0.0f) },

		// Back face
		{ Vector3(-1.0f, -1.0f, -1.0f), Vector3(0.0f, 0.0f, -1.0f) },
		{ Vector3(1.0f, -1.0f, -1.0f), Vector3(0.0f, 0.0f, -1.0f) },
		{ Vector3(1.0f, 1.0f, -1.0f), Vector3(0.0f, 0.0f, -1.0f) },
		{ Vector3(-1.0f, 1.0f, -1.0f), Vector3(0.0f, 0.0f, -1.0f) },

		// Front face
		{ Vector3(-1.0f, -1.0f, 1.0f), Vector3(0.0f, 0.0f, 1.0f) },
		{ Vector3(1.0f, -1.0f, 1.0f), Vector3(0.0f, 0.0f, 1.0f) },
		{ Vector3(1.0f, 1.0f, 1.0f), Vector3(0.0f, 0.0f, 1.0f) },
		{ Vector3(-1.0f, 1.0f, 1.0f), Vector3(0.0f, 0.0f, 1.0f) },
	};
	D3D11_BUFFER_DESC bd = {};
	bd.ByteWidth = sizeof(CubeVertex) * ARRAYSIZE(vertices);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA vbData = {};
	vbData.pSysMem = vertices;
	HR_T( m_pDevice->CreateBuffer(&bd, &vbData, m_pVertexBuffer.GetAddressOf()));
		
	m_VertexBufferStride = sizeof(CubeVertex);
	m_VertexBufferOffset = 0;

	// 인덱스 버퍼 생성
	WORD indices[] =
	{
		3,1,0, 2,1,3,
		6,4,5, 7,4,6,
		11,9,8, 10,9,11,
		14,12,13, 15,12,14,
		19,17,16, 18,17,19,
		22,20,21, 23,20,22
	};	
	m_nIndexCount = ARRAYSIZE(indices);
	bd = {};
	bd.ByteWidth = sizeof(WORD) * ARRAYSIZE(indices);
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA ibData = {};
	ibData.pSysMem = indices;
	HR_T(m_pDevice->CreateBuffer(&bd, &ibData, m_pIndexBuffer.GetAddressOf()));

	// 입력 레이아웃 생성
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	ComPtr<ID3DBlob> vertexShaderBuffer;
	HR_T(CompileShaderFromFile(L"../shaders/99_BasicVertexShader.hlsl", "main", "vs_4_0", vertexShaderBuffer.GetAddressOf()));
	HR_T(m_pDevice->CreateInputLayout(layout, ARRAYSIZE(layout),
		vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), m_pInputLayout.GetAddressOf()));

	// 버텍스 셰이더 생성
	HR_T(m_pDevice->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), NULL, m_pVertexShader.GetAddressOf()));

	// 픽셀 셰이더 생성
	ComPtr<ID3D10Blob> pixelShaderBuffer;
	HR_T(CompileShaderFromFile(L"../shaders/99_BasicPixelShader.hlsl", "main", "ps_4_0", (ID3DBlob**)pixelShaderBuffer.GetAddressOf()));
	HR_T(m_pDevice->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
		pixelShaderBuffer->GetBufferSize(), NULL, m_pPixelShader.GetAddressOf()));

	// 상수 버퍼 생성
	bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(CBNeverChanges);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	HR_T( m_pDevice->CreateBuffer(&bd, nullptr, m_pCBNeverChanges.GetAddressOf()));

	bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(CBChangeOnResize);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	HR_T(m_pDevice->CreateBuffer(&bd, nullptr, m_pCBChangeOnResize.GetAddressOf()));

	bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(CBChangesEveryFrame);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	HR_T(m_pDevice->CreateBuffer(&bd, nullptr, m_pCBChangesEveryFrame.GetAddressOf()));

	// World,View,Projection 초기값 설정
	m_World1 = XMMatrixIdentity();
	m_World2 = XMMatrixTranslation(0.0f, 0.0f, 10.0f);
	m_World3 = XMMatrixTranslation(0.0f, 0.0f, 20.0f);

	// 카메라 초기화
	m_Camera.m_PositionInitial = Vector3(0.0f, 3.0f, -20.0f);
	m_Camera.Reset();  
	m_Camera.SetSpeed(10.0f);
	m_Camera.GetViewMatrix(m_View);
	
	m_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, m_ClientWidth / (FLOAT)m_ClientHeight, 0.1f, 1000.0f);
	
	// Constant Buffer 초기화
	CBChangeOnResize cbChangesOnResize;
	cbChangesOnResize.mProjection = XMMatrixTranspose(m_Projection);
	m_pDeviceContext->UpdateSubresource(m_pCBChangeOnResize.Get(), 0, nullptr, &cbChangesOnResize, 0, 0);
	
	return true;
}

void TutorialApp::UninitScene()
{
	// ComPtr이 자동으로 메모리 해제를 처리하므로 명시적 해제 불필요
}

bool TutorialApp::InitRenderStates()
{
	UpdateRasterizerState();
	UpdateDepthStencilState();
	UpdateBlendState();
	return true;
}

void TutorialApp::UninitRenderStates()
{
	// ComPtr이 자동으로 메모리 해제를 처리하므로 명시적 해제 불필요
}

void TutorialApp::UpdateRasterizerState()
{
	D3D11_RASTERIZER_DESC desc = {};
	desc.FillMode = (m_FillMode == 0) ? D3D11_FILL_SOLID : D3D11_FILL_WIREFRAME;
	desc.CullMode = (m_CullMode == 0) ? D3D11_CULL_NONE : (m_CullMode == 1) ? D3D11_CULL_BACK : D3D11_CULL_FRONT;
	desc.FrontCounterClockwise = m_FrontCounterClockwise;
	desc.DepthClipEnable = m_DepthClipEnable;
	desc.ScissorEnable = m_ScissorEnable;
	desc.MultisampleEnable = false;
	desc.AntialiasedLineEnable = false;
	
	m_pRasterizerState.Reset();
	m_pDevice->CreateRasterizerState(&desc, m_pRasterizerState.GetAddressOf());
}

void TutorialApp::UpdateDepthStencilState()
{
	D3D11_DEPTH_STENCIL_DESC desc = {};
	desc.DepthEnable = (m_DepthTestEnable || m_DepthWriteEnable);
	desc.DepthWriteMask = m_DepthWriteEnable ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
	
	// DepthFunc mapping
	D3D11_COMPARISON_FUNC funcs[] = {
		D3D11_COMPARISON_NEVER,
		D3D11_COMPARISON_LESS,
		D3D11_COMPARISON_EQUAL,
		D3D11_COMPARISON_LESS_EQUAL,
		D3D11_COMPARISON_GREATER,
		D3D11_COMPARISON_NOT_EQUAL,
		D3D11_COMPARISON_GREATER_EQUAL,
		D3D11_COMPARISON_ALWAYS
	};
	if (m_DepthFunc < 0) m_DepthFunc = 0;
	if (m_DepthFunc >= (int)_countof(funcs)) m_DepthFunc = (int)_countof(funcs) - 1;
	desc.DepthFunc = m_DepthTestEnable ? funcs[m_DepthFunc] : D3D11_COMPARISON_ALWAYS;
	desc.StencilEnable = m_StencilEnable;
	desc.StencilReadMask = 0xFF;
	desc.StencilWriteMask = 0xFF;
	
	// Stencil operations for front faces
	desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	
	// Stencil operations for back faces
	desc.BackFace = desc.FrontFace;
	
	m_pDepthStencilState.Reset();
	m_pDevice->CreateDepthStencilState(&desc, m_pDepthStencilState.GetAddressOf());
}

void TutorialApp::UpdateBlendState()
{
	D3D11_BLEND_DESC desc = {};
	desc.AlphaToCoverageEnable = m_AlphaToCoverageEnable;
	desc.IndependentBlendEnable = false;
	
	D3D11_BLEND blendTypes[] = {
		D3D11_BLEND_ZERO,
		D3D11_BLEND_ONE,
		D3D11_BLEND_SRC_COLOR,
		D3D11_BLEND_INV_SRC_COLOR,
		D3D11_BLEND_SRC_ALPHA,
		D3D11_BLEND_INV_SRC_ALPHA,
		D3D11_BLEND_DEST_ALPHA,
		D3D11_BLEND_INV_DEST_ALPHA,
		D3D11_BLEND_DEST_COLOR,
		D3D11_BLEND_INV_DEST_COLOR
	};
	
	D3D11_BLEND_OP blendOps[] = {
		D3D11_BLEND_OP_ADD,
		D3D11_BLEND_OP_ADD,
		D3D11_BLEND_OP_SUBTRACT,
		D3D11_BLEND_OP_REV_SUBTRACT,
		D3D11_BLEND_OP_MIN,
		D3D11_BLEND_OP_MAX
	};
	
	desc.RenderTarget[0].BlendEnable = m_BlendEnable;
	desc.RenderTarget[0].SrcBlend = blendTypes[m_SrcBlend];
	desc.RenderTarget[0].DestBlend = blendTypes[m_DestBlend];
	desc.RenderTarget[0].BlendOp = blendOps[m_BlendOp];
	desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	
	m_pBlendState.Reset();
	m_pDevice->CreateBlendState(&desc, m_pBlendState.GetAddressOf());
}

bool TutorialApp::InitImGUI()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(m_hWnd);
	ImGui_ImplDX11_Init(m_pDevice.Get(), m_pDeviceContext.Get());

	return true;
}

void TutorialApp::UninitImGUI()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void TutorialApp::RenderImGUI()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if ((m_DepthTestEnable || m_DepthWriteEnable) && m_pDepthStencilSRV)
	{
		ImGuiIO& io = ImGui::GetIO();
		const float margin = 10.0f;
		const ImVec2 size(220.0f, 220.0f);
		ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - size.x - margin, margin), ImGuiCond_Always);
		ImGui::SetNextWindowSize(size, ImGuiCond_Always);
		ImGui::SetNextWindowBgAlpha(0.75f);
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
		ImGui::Begin("Depth##Preview", nullptr, flags);
		ImGui::TextUnformatted("Depth");
		ImGui::Image((ImTextureID)m_pDepthStencilSRV.Get(), ImVec2(size.x - 20.0f, size.y - 40.0f));
		ImGui::End();
	}

	ImGui::Begin("Render Settings");
	
	// Frame Info
	float fps = 1.0f / GameTimer::m_Instance->DeltaTime();
	float frameTime = GameTimer::m_Instance->DeltaTime() * 1000.0f; // ms
	ImGui::Text("FPS: %.1f (%.2f ms)", fps, frameTime);
	ImGui::Separator();
	
	ImGui::Text("Camera Info");
	ImGui::Text("Position: (%.2f, %.2f, %.2f)", m_Camera.m_Position.x, m_Camera.m_Position.y, m_Camera.m_Position.z);
	ImGui::Text("Rotation: (%.2f, %.2f, %.2f)", m_Camera.m_Rotation.x, m_Camera.m_Rotation.y, m_Camera.m_Rotation.z);
	ImGui::DragFloat("Move Speed", &m_Camera.m_MoveSpeed, 0.5f, 1.0f, 100.0f, "%.1f");
	if (ImGui::Button("Reset Camera"))
	{
		m_Camera.Reset();
		m_Camera.m_Position = Vector3(0.0f, 3.0f, -20.0f);
	}
	
	ImGui::Separator();
	ImGui::Text("Light Settings");
	ImGui::ColorEdit3("Light Color", (float*)&m_vLightColor);
	ImGui::SliderFloat3("Light Direction", (float*)&m_vLightDir, -1.0f, 1.0f);
	
	ImGui::Separator();
	ImGui::Checkbox("Rotate Animation", &m_bRotateAnimation);
	ImGui::ColorEdit4("Clear Color", (float*)&m_vClearColor, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_Float);
	ImGui::ColorEdit4("Cube 1 Color", (float*)&m_vMeshColor1, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreview| ImGuiColorEditFlags_Float);
	ImGui::ColorEdit4("Cube 2 Color", (float*)&m_vMeshColor2, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_Float);
	ImGui::ColorEdit4("Cube 3 Color", (float*)&m_vMeshColor3, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_Float);
	
	ImGui::Separator();
	ImGui::Text("Draw Order Control");
	ImGui::Text("(Change drawing order to test depth/blend)");
	
	const char* cubeNames[] = { "Cube (Red)", "Cube (Green)", "Cube (Blue)" };
	for (int i = 0; i < 3; i++)
	{
		ImGui::Text("  %d. %s", i + 1, cubeNames[m_DrawOrder[i]]);
		ImGui::SameLine(200);
		
		if (i > 0)
		{
			ImGui::PushID(i * 10);
			if (ImGui::ArrowButton("##up", ImGuiDir_Up))
			{
				std::swap(m_DrawOrder[i], m_DrawOrder[i - 1]);
			}
			ImGui::PopID();
			ImGui::SameLine();
		}
		else
		{
			ImGui::Dummy(ImVec2(24, 0));
			ImGui::SameLine();
		}
		
		if (i < 2)
		{
			ImGui::PushID(i * 10 + 1);
			if (ImGui::ArrowButton("##down", ImGuiDir_Down))
			{
				std::swap(m_DrawOrder[i], m_DrawOrder[i + 1]);
			}
			ImGui::PopID();
		}
	}
	
	if (ImGui::Button("Reset Draw Order"))
	{
		m_DrawOrder[0] = 0;
		m_DrawOrder[1] = 1;
		m_DrawOrder[2] = 2;
	}
	
	ImGui::Text("---------------------------------");
	ImGui::Text("Rasterizer State");
	ImGui::Text("---------------------------------");
	
	const char* fillModes[] = { "Solid", "Wireframe" };
	if (ImGui::Combo("Fill Mode", &m_FillMode, fillModes, 2))
		UpdateRasterizerState();
	
	const char* cullModes[] = { "None", "Back", "Front" };
	if (ImGui::Combo("Cull Mode", &m_CullMode, cullModes, 3))
		UpdateRasterizerState();
	
	if (ImGui::Checkbox("Front Counter Clockwise", &m_FrontCounterClockwise))
		UpdateRasterizerState();
	
	if (ImGui::Checkbox("Depth Clip Enable", &m_DepthClipEnable))
		UpdateRasterizerState();
	
	if (ImGui::Checkbox("Scissor Enable", &m_ScissorEnable))
		UpdateRasterizerState();
	
	ImGui::Text("---------------------------------");
	ImGui::Text("Depth Stencil State");
	ImGui::Text("---------------------------------");
	
	if (ImGui::Checkbox("Depth Test", &m_DepthTestEnable))
		UpdateDepthStencilState();

	ImGui::SameLine();
	if (ImGui::Checkbox("Depth Write", &m_DepthWriteEnable))
		UpdateDepthStencilState();
	
	const char* depthFuncs[] = { "Never", "Less", "Equal", "LessEqual", "Greater", "NotEqual", "GreaterEqual", "Always" };
	if (ImGui::Combo("Depth Func", &m_DepthFunc, depthFuncs, 8))
		UpdateDepthStencilState();
	
	//if (ImGui::Checkbox("Stencil Enable", &m_StencilEnable))
	//	UpdateDepthStencilState();
	
	ImGui::Text("---------------------------------");
	ImGui::Text("Blend State");
	ImGui::Text("---------------------------------");
	
	if (ImGui::Checkbox("Blend Enable", &m_BlendEnable))
		UpdateBlendState();
	
	const char* blendTypes[] = { "Zero", "One", "SrcColor", "InvSrcColor", "SrcAlpha", "InvSrcAlpha", "DestAlpha", "InvDestAlpha", "DestColor", "InvDestColor" };
	if (ImGui::Combo("Src Blend", &m_SrcBlend, blendTypes, 10))
		UpdateBlendState();
	
	if (ImGui::Combo("Dest Blend", &m_DestBlend, blendTypes, 10))
		UpdateBlendState();
	
	const char* blendOps[] = {"Add", "Subtract", "RevSubtract", "Min", "Max" };
	if (ImGui::Combo("Blend Op", &m_BlendOp, blendOps, 5))
		UpdateBlendState();
	
	if (ImGui::Checkbox("Alpha To Coverage", &m_AlphaToCoverageEnable))
		UpdateBlendState();
	
	ImGui::End();

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK TutorialApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
		return true;

	return __super::WndProc(hWnd, message, wParam, lParam);
}
