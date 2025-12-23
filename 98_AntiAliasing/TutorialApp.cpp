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

	if (!InitMSAA())
		return false;

	if (!InitFXAA())
		return false;

	if (!InitImGUI())
		return false;

	return true;
}

void TutorialApp::OnUninitialize()
{
	UninitImGUI();
	UninitFXAA();
	UninitMSAA();
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
	// Select render target based on AA mode
	ID3D11RenderTargetView* rtv = nullptr;
	ID3D11DepthStencilView* dsv = nullptr;
	
	if (m_AAMode == AA_MSAA)
	{
		rtv = m_pMSAARenderTargetView.Get();
		dsv = m_pMSAADepthStencilView.Get();
	}
	else if (m_AAMode == AA_FXAA)
	{
		rtv = m_pSceneRenderTargetView.Get();
		dsv = m_pDepthStencilView.Get();
	}
	else // AA_NONE
	{
		rtv = m_pRenderTargetView.Get();
		dsv = m_pDepthStencilView.Get();
	}
	
	// 렌더링 설정
	ID3D11RenderTargetView* rtvs[] = { rtv };
	m_pDeviceContext->OMSetRenderTargets(1, rtvs, dsv);

	m_pDeviceContext->ClearRenderTargetView(rtv, (float*)&m_vClearColor);
	m_pDeviceContext->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);

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

	// Draw cubes
	CBChangesEveryFrame cb;
	cb.mView = XMMatrixTranspose(m_View);
	cb.vLightDir = m_vLightDir;
	cb.vLightColor = m_vLightColor;

	// Cube 1 (Red)
	cb.mWorld = XMMatrixTranspose(m_World1);
	cb.vMeshColor = m_vMeshColor1;
	m_pDeviceContext->UpdateSubresource(m_pCBChangesEveryFrame.Get(), 0, nullptr, &cb, 0, 0);
	m_pDeviceContext->DrawIndexed(m_nIndexCount, 0, 0);
	
	// Cube 2 (Green)
	cb.mWorld = XMMatrixTranspose(m_World2);
	cb.vMeshColor = m_vMeshColor2;
	m_pDeviceContext->UpdateSubresource(m_pCBChangesEveryFrame.Get(), 0, nullptr, &cb, 0, 0);
	m_pDeviceContext->DrawIndexed(m_nIndexCount, 0, 0);
	
	// Cube 3 (Blue)
	cb.mWorld = XMMatrixTranspose(m_World3);
	cb.vMeshColor = m_vMeshColor3;
	m_pDeviceContext->UpdateSubresource(m_pCBChangesEveryFrame.Get(), 0, nullptr, &cb, 0, 0);
	m_pDeviceContext->DrawIndexed(m_nIndexCount, 0, 0);

	// Post-processing based on AA mode
	if (m_AAMode == AA_MSAA && m_pMSAATexture)
	{
		// Resolve MSAA to backbuffer
		ComPtr<ID3D11Texture2D> backBuffer;
		m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)backBuffer.GetAddressOf());
		m_pDeviceContext->ResolveSubresource(backBuffer.Get(), 0, m_pMSAATexture.Get(), 0, DXGI_FORMAT_R8G8B8A8_UNORM);
		
		// Switch to backbuffer for ImGUI
		ID3D11RenderTargetView* backBufferRTV[] = { m_pRenderTargetView.Get() };
		m_pDeviceContext->OMSetRenderTargets(1, backBufferRTV, nullptr);
	}
	else if (m_AAMode == AA_FXAA && m_pSceneTexture)
	{
		// Apply FXAA pass to backbuffer
		ID3D11RenderTargetView* backBufferRTV[] = { m_pRenderTargetView.Get() };
		m_pDeviceContext->OMSetRenderTargets(1, backBufferRTV, nullptr);
		// Don't clear - we want to apply FXAA to the rendered scene
		
		// Update FXAA constant buffer
		float fxaaParams[4] = { m_FXAAQualitySubpix, m_FXAAQualityEdgeThreshold, m_FXAAQualityEdgeThresholdMin, 0.0f };
		m_pDeviceContext->UpdateSubresource(m_pFXAAConstantBuffer.Get(), 0, nullptr, fxaaParams, 0, 0);
		
		// Setup FXAA rendering
		m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_pDeviceContext->IASetInputLayout(nullptr);
		m_pDeviceContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
		m_pDeviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
		m_pDeviceContext->VSSetShader(m_pFullscreenVS.Get(), nullptr, 0);
		m_pDeviceContext->PSSetShader(m_pFXAAPS.Get(), nullptr, 0);
		
		ID3D11ShaderResourceView* srvs[] = { m_pSceneShaderResourceView.Get() };
		m_pDeviceContext->PSSetShaderResources(0, 1, srvs);
		ID3D11SamplerState* samplers[] = { m_pLinearSampler.Get() };
		m_pDeviceContext->PSSetSamplers(0, 1, samplers);
		ID3D11Buffer* cbs[] = { m_pFXAAConstantBuffer.Get() };
		m_pDeviceContext->PSSetConstantBuffers(0, 1, cbs);
		
		// Disable depth test for fullscreen quad
		m_pDeviceContext->OMSetDepthStencilState(nullptr, 0);
		m_pDeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
		m_pDeviceContext->RSSetState(nullptr);
		
		// Draw fullscreen triangle
		m_pDeviceContext->Draw(3, 0);
		
		// Clear shader resources
		ID3D11ShaderResourceView* nullSRV[] = { nullptr };
		m_pDeviceContext->PSSetShaderResources(0, 1, nullSRV);
	}

	// Render ImGUI
	RenderImGUI();

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
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;

	ComPtr<ID3D11Texture2D> textureDepthStencil;
	HR_T(m_pDevice->CreateTexture2D(&descDepth, nullptr, textureDepthStencil.GetAddressOf()));

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	HR_T(m_pDevice->CreateDepthStencilView(textureDepthStencil.Get(), &descDSV, m_pDepthStencilView.GetAddressOf()));

	
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
	HR_T(CompileShaderFromFile(L"../shaders/98_BasicVertexShader.hlsl", "main", "vs_4_0", vertexShaderBuffer.GetAddressOf()));
	HR_T(m_pDevice->CreateInputLayout(layout, ARRAYSIZE(layout),
		vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), m_pInputLayout.GetAddressOf()));

	// 버텍스 셰이더 생성
	HR_T(m_pDevice->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), NULL, m_pVertexShader.GetAddressOf()));

	// 픽셀 셰이더 생성
	ComPtr<ID3D10Blob> pixelShaderBuffer;
	HR_T(CompileShaderFromFile(L"../shaders/98_BasicPixelShader.hlsl", "main", "ps_4_0", (ID3DBlob**)pixelShaderBuffer.GetAddressOf()));
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
	
	m_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, m_ClientWidth / (FLOAT)m_ClientHeight, 1.0f, 1000.0f);
	
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

bool TutorialApp::InitMSAA()
{
	if (m_AAMode != AA_MSAA)
		return true;

	HRESULT hr = S_OK;

	// Create MSAA render target texture
	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = m_ClientWidth;
	texDesc.Height = m_ClientHeight;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = m_MSAASampleCount;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	HR_T(m_pDevice->CreateTexture2D(&texDesc, nullptr, m_pMSAATexture.GetAddressOf()));

	// Create MSAA render target view
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = texDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;

	HR_T(m_pDevice->CreateRenderTargetView(m_pMSAATexture.Get(), &rtvDesc, m_pMSAARenderTargetView.GetAddressOf()));

	// Create MSAA depth stencil texture
	D3D11_TEXTURE2D_DESC depthDesc = {};
	depthDesc.Width = m_ClientWidth;
	depthDesc.Height = m_ClientHeight;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.SampleDesc.Count = m_MSAASampleCount;
	depthDesc.SampleDesc.Quality = 0;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthDesc.CPUAccessFlags = 0;
	depthDesc.MiscFlags = 0;

	HR_T(m_pDevice->CreateTexture2D(&depthDesc, nullptr, m_pMSAADepthStencil.GetAddressOf()));

	// Create MSAA depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = depthDesc.Format;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;

	HR_T(m_pDevice->CreateDepthStencilView(m_pMSAADepthStencil.Get(), &dsvDesc, m_pMSAADepthStencilView.GetAddressOf()));

	return true;
}

void TutorialApp::UninitMSAA()
{
	// ComPtr이 자동으로 메모리 해제를 처리하므로 명시적 해제 불필요
}

bool TutorialApp::InitFXAA()
{
	if (m_AAMode != AA_FXAA)
		return true;

	HRESULT hr = S_OK;

	// Create scene render target texture (with SRV for FXAA input)
	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = m_ClientWidth;
	texDesc.Height = m_ClientHeight;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	HR_T(m_pDevice->CreateTexture2D(&texDesc, nullptr, m_pSceneTexture.GetAddressOf()));

	// Create render target view
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = texDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;

	HR_T(m_pDevice->CreateRenderTargetView(m_pSceneTexture.Get(), &rtvDesc, m_pSceneRenderTargetView.GetAddressOf()));

	// Create shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	HR_T(m_pDevice->CreateShaderResourceView(m_pSceneTexture.Get(), &srvDesc, m_pSceneShaderResourceView.GetAddressOf()));

	// Create linear sampler
	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	HR_T(m_pDevice->CreateSamplerState(&sampDesc, m_pLinearSampler.GetAddressOf()));

	// Compile fullscreen vertex shader
	ComPtr<ID3DBlob> vsBlob;
	HR_T(CompileShaderFromFile(L"../Shaders/98_FullscreenVS.hlsl", "main", "vs_4_0", vsBlob.GetAddressOf()));
	HR_T(m_pDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, m_pFullscreenVS.GetAddressOf()));

	// Compile FXAA pixel shader
	ComPtr<ID3DBlob> psBlob;
	HR_T(CompileShaderFromFile(L"../Shaders/98_FXAA_PS.hlsl", "main", "ps_4_0", psBlob.GetAddressOf()));
	HR_T(m_pDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, m_pFXAAPS.GetAddressOf()));

	// Create FXAA constant buffer
	D3D11_BUFFER_DESC cbDesc = {};
	cbDesc.ByteWidth = sizeof(float) * 4; // 3 floats + 1 padding
	cbDesc.Usage = D3D11_USAGE_DEFAULT;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = 0;
	HR_T(m_pDevice->CreateBuffer(&cbDesc, nullptr, m_pFXAAConstantBuffer.GetAddressOf()));

	return true;
}

void TutorialApp::UninitFXAA()
{
	// ComPtr이 자동으로 메모리 해제를 처리하므로 명시적 해제 불필요
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
	ImGui::ColorEdit4("Clear Color", (float*)&m_vClearColor, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreview);
	ImGui::ColorEdit4("Cube 1 Color", (float*)&m_vMeshColor1, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreview);
	ImGui::ColorEdit4("Cube 2 Color", (float*)&m_vMeshColor2, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreview);
	ImGui::ColorEdit4("Cube 3 Color", (float*)&m_vMeshColor3, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreview);
	
	ImGui::Separator();
	ImGui::Text("Anti-Aliasing Settings");
	bool aaChanged = false;
	const char* aaModes[] = { "No Anti-Aliasing", "MSAA", "FXAA" };
	if (ImGui::Combo("AA Mode", &m_AAMode, aaModes, 3))
		aaChanged = true;
	
	if (m_AAMode == AA_MSAA)
	{
		const char* sampleCounts[] = { "1x", "2x", "4x", "8x" };
		int sampleIndex = (m_MSAASampleCount == 1) ? 0 : (m_MSAASampleCount == 2) ? 1 : (m_MSAASampleCount == 4) ? 2 : 3;
		if (ImGui::Combo("Sample Count", &sampleIndex, sampleCounts, 4))
		{
			int counts[] = { 1, 2, 4, 8 };
			m_MSAASampleCount = counts[sampleIndex];
			aaChanged = true;
		}
	}
	
	if (m_AAMode == AA_FXAA)
	{
		ImGui::Text("FXAA Quality Parameters (NVIDIA 3.11)");
		
		// Subpixel Quality
		ImGui::SliderFloat("Subpixel Quality", &m_FXAAQualitySubpix, 0.0f, 1.0f, "%.2f");
		ImGui::Text("  (Higher = Better text & small details)");
		
		// Edge Threshold
		ImGui::SliderFloat("Edge Threshold", &m_FXAAQualityEdgeThreshold, 0.063f, 0.333f, "%.3f");
		ImGui::Text("  (Lower = More edges detected)");
		
		// Edge Threshold Min
		ImGui::SliderFloat("Edge Threshold Min", &m_FXAAQualityEdgeThresholdMin, 0.0312f, 0.0833f, "%.4f");
		ImGui::Text("  (Lower = More sensitive)");
		
		if (ImGui::Button("Reset to Default"))
		{
			m_FXAAQualitySubpix = 0.75f;
			m_FXAAQualityEdgeThreshold = 0.166f;
			m_FXAAQualityEdgeThresholdMin = 0.0833f;
		}
	}
	
	if (aaChanged)
	{
		UninitFXAA();
		UninitMSAA();
		InitMSAA();
		InitFXAA();
	}
	
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
