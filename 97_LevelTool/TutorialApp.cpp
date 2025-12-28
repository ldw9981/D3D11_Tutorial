
#include "TutorialApp.h"
#include "../Common/Helper.h"
#include <d3dcompiler.h>
#include "CubeObject.h"
#include <imgui.h>

#pragma comment (lib, "d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")


// RTTR 사용을 위한 헤더와 라이브러리 링크
#define RTTR_DLL
#include <rttr/registration>

#ifdef _DEBUG
#pragma comment(lib, "rttr_core_d.lib")
#else
#pragma comment(lib, "rttr_core.lib")
#endif

// 정점 구조체
struct Vertex
{
	Vector3 Pos;		// 위치 정보
	Vector3 Normal;		// 법선 정보
};

struct ConstantBuffer
{
	Matrix mWorld;
	Matrix mView;
	Matrix mProjection;
	Vector4 vLightDir;
	Vector4 vLightColor;
	Vector4 vMaterialColor;
};


bool TutorialApp::OnInitialize()
{
	if (!InitD3D())
		return false;

	if (!InitScene())
		return false;

	if (!InitImGui())
		return false;

	return true;
}

void TutorialApp::OnUninitialize()
{
	UninitImGui();
	UninitScene();
	UninitD3D();
}

void TutorialApp::OnUpdate()
{
	float t = GameTimer::m_Instance->TotalTime();
	float dt = GameTimer::m_Instance->DeltaTime();

	// 카메라 업데이트
	m_Camera.Update(dt);

	// 모든 GameObject AABB 업데이트
	for (GameObject* obj : m_World.GetGameObjects())
	{
		if (obj)
			obj->UpdateAABB();
	}

	m_Camera.GetViewMatrix(m_View);
}

void TutorialApp::OnRender()
{
	float color[4] = { 0.0f, 0.5f, 0.5f, 1.0f };

	// 렌더타겟 설정
	ID3D11RenderTargetView* rtvs[] = { m_pRenderTargetView.Get() };
	m_pDeviceContext->OMSetRenderTargets(1, rtvs, m_pDepthStencilView.Get());

	// Clear
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView.Get(), color);
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	// 모든 GameObject 렌더링
	for (GameObject* obj : m_World.GetGameObjects())
	{
		if (!obj)
			continue;

		// AABB 업데이트
		//obj->UpdateAABB();

		// CubeObject만 렌더링
		CubeObject* cube = dynamic_cast<CubeObject*>(obj);
		if (!cube)
			continue;

		// 상수 버퍼 업데이트
		ConstantBuffer cb;
		cb.mWorld = cube->GetWorldMatrix().Transpose();
		cb.mView = XMMatrixTranspose(m_View);
		cb.mProjection = XMMatrixTranspose(m_Projection);
		cb.vLightDir = m_LightDir;
		cb.vLightColor = m_LightColor;
		cb.vMaterialColor = cube->m_Color;
		m_pDeviceContext->UpdateSubresource(m_pConstantBuffer.Get(), 0, nullptr, &cb, 0, 0);

		// 렌더 설정
		m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		ID3D11Buffer* vbs[] = { m_pVertexBuffer.Get() };
		m_pDeviceContext->IASetVertexBuffers(0, 1, vbs, &m_VertexBufferStride, &m_VertexBufferOffset);
		m_pDeviceContext->IASetInputLayout(m_pInputLayout.Get());
		m_pDeviceContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
		m_pDeviceContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0);
		ID3D11Buffer* cbs[] = { m_pConstantBuffer.Get() };
		m_pDeviceContext->VSSetConstantBuffers(0, 1, cbs);
		m_pDeviceContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0);
		m_pDeviceContext->PSSetConstantBuffers(0, 1, cbs);

		m_pDeviceContext->DrawIndexed(m_nIndices, 0, 0);
	}

	// 모든 오브젝트 AABB 디버그 렌더링
	if (m_bShowAllAABB)
	{
		// 렌더타겟 다시 설정 (ImGui가 변경했을 수 있음)
		m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), m_pDepthStencilView.Get());

		// DebugDraw의 BasicEffect 설정
		DebugDraw::g_BatchEffect->SetWorld(Matrix::Identity);
		DebugDraw::g_BatchEffect->SetView(m_View);
		DebugDraw::g_BatchEffect->SetProjection(m_Projection);
		DebugDraw::g_BatchEffect->Apply(m_pDeviceContext.Get());

		// InputLayout 설정
		m_pDeviceContext->IASetInputLayout(DebugDraw::g_pBatchInputLayout.Get());

		// 블렌드 스테이트 설정
		m_pDeviceContext->OMSetBlendState(DebugDraw::g_States->AlphaBlend(), nullptr, 0xFFFFFFFF);
		m_pDeviceContext->OMSetDepthStencilState(DebugDraw::g_States->DepthNone(), 0);
		m_pDeviceContext->RSSetState(DebugDraw::g_States->CullNone());

		DebugDraw::g_Batch->Begin();

		for (GameObject* obj : m_World.GetGameObjects())
		{
			if (!obj)
				continue;

			// 모든 오브젝트는 어두운 초록색
			XMVECTOR color = XMVectorSet(0.0f, 0.2f, 0.0f, 0.7f);
			DebugDraw::Draw(DebugDraw::g_Batch.get(), obj->m_AABB, color);
		}

		DebugDraw::g_Batch->End();

		// 렌더 스테이트 복원
		m_pDeviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
		m_pDeviceContext->OMSetDepthStencilState(nullptr, 0);
		m_pDeviceContext->RSSetState(nullptr);
	}

	// 선택된 오브젝트 AABB 렌더링 (설정 이후, ImGui 이전) - 항상 표시
	if (m_pSelectedObject != nullptr)
	{
		// 렌더타겟 다시 설정 (ImGui가 변경했을 수 있음)
		m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), m_pDepthStencilView.Get());

		// DebugDraw의 BasicEffect 설정
		DebugDraw::g_BatchEffect->SetWorld(Matrix::Identity);
		DebugDraw::g_BatchEffect->SetView(m_View);
		DebugDraw::g_BatchEffect->SetProjection(m_Projection);
		DebugDraw::g_BatchEffect->Apply(m_pDeviceContext.Get());

		// InputLayout 설정
		m_pDeviceContext->IASetInputLayout(DebugDraw::g_pBatchInputLayout.Get());

		// 블렌드 스테이트 설정 (깊이 테스트 활성화)
		m_pDeviceContext->OMSetBlendState(DebugDraw::g_States->AlphaBlend(), nullptr, 0xFFFFFFFF);
		m_pDeviceContext->OMSetDepthStencilState(DebugDraw::g_States->DepthRead(), 0);
		m_pDeviceContext->RSSetState(DebugDraw::g_States->CullNone());

		DebugDraw::g_Batch->Begin();

		// 선택된 오브젝트는 밝은 초록색
		XMVECTOR color = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
		DebugDraw::Draw(DebugDraw::g_Batch.get(), m_pSelectedObject->m_AABB, color);

		DebugDraw::g_Batch->End();

		// 렌더 스테이트 복원
		m_pDeviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
		m_pDeviceContext->OMSetDepthStencilState(nullptr, 0);
		m_pDeviceContext->RSSetState(nullptr);
	}
	

	// RTTR로 GameObject 멤버를 조회해 ImGui UI를 자동 생성하는 함수 호출
	RenderImGuiCubeRTTR();

	// 레이 디버그 렌더링(ImGui 이후에 그려서 위에 표시)
	if (m_bShowDebugRay)
	{
		// 렌더타겟 다시 설정 (ImGui가 변경했을 수 있음)
		m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), m_pDepthStencilView.Get());

		// DebugDraw의 BasicEffect 설정
		DebugDraw::g_BatchEffect->SetWorld(Matrix::Identity);
		DebugDraw::g_BatchEffect->SetView(m_View);
		DebugDraw::g_BatchEffect->SetProjection(m_Projection);
		DebugDraw::g_BatchEffect->Apply(m_pDeviceContext.Get());

		// InputLayout 설정
		m_pDeviceContext->IASetInputLayout(DebugDraw::g_pBatchInputLayout.Get());

		// 블렌드 스테이트 설정 (알파 블렌드)
		m_pDeviceContext->OMSetBlendState(DebugDraw::g_States->AlphaBlend(), nullptr, 0xFFFFFFFF);
		m_pDeviceContext->OMSetDepthStencilState(DebugDraw::g_States->DepthNone(), 0);
		m_pDeviceContext->RSSetState(DebugDraw::g_States->CullNone());

		DebugDraw::g_Batch->Begin();

		Vector3 rayStart = m_DebugRayOrigin;
		Vector3 rayEnd = m_DebugRayOrigin + m_DebugRayDirection * 100.0f;

		// 빨강에서 노랑으로 그라데이션 레이
		VertexPositionColor v1(rayStart, Vector4(1.0f, 0.0f, 0.0f, 1.0f));
		VertexPositionColor v2(rayEnd, Vector4(1.0f, 1.0f, 0.0f, 1.0f));

		DebugDraw::g_Batch->DrawLine(v1, v2);

		DebugDraw::g_Batch->End();

		// 렌더 스테이트 복원
		m_pDeviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
		m_pDeviceContext->OMSetDepthStencilState(nullptr, 0);
		m_pDeviceContext->RSSetState(nullptr);
	}

	// Present
	m_pSwapChain->Present(0, 0);
}

bool TutorialApp::InitD3D()
{
	HRESULT hr = 0;

	// 스왑체인 속성 설정 구조체 생성
	DXGI_SWAP_CHAIN_DESC swapDesc = {};
	swapDesc.BufferCount = 1;
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

	UINT creationFlags = 0;
#ifdef _DEBUG
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// 디바이스, 스왑체인, 디바이스 컨텍스트 생성
	HR_T(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, creationFlags, NULL, NULL,
		D3D11_SDK_VERSION, &swapDesc, m_pSwapChain.GetAddressOf(), m_pDevice.GetAddressOf(), NULL, m_pDeviceContext.GetAddressOf()));

	// 렌더타겟뷰 생성 (백버퍼를 이용하는 렌더타겟뷰)
	ComPtr<ID3D11Texture2D> pBackBufferTexture;
	HR_T(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)pBackBufferTexture.GetAddressOf()));
	HR_T(m_pDevice->CreateRenderTargetView(pBackBufferTexture.Get(), NULL, m_pRenderTargetView.GetAddressOf()));

	ID3D11RenderTargetView* rtvs[] = { m_pRenderTargetView.Get() };
	m_pDeviceContext->OMSetRenderTargets(1, rtvs, NULL);

	// 뷰포트 설정
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)m_ClientWidth;
	viewport.Height = (float)m_ClientHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	m_pDeviceContext->RSSetViewports(1, &viewport);

	// 깊이&스텐실 버퍼 생성
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

	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	HR_T(m_pDevice->CreateDepthStencilView(textureDepthStencil.Get(), &descDSV, m_pDepthStencilView.GetAddressOf()));

	ID3D11RenderTargetView* rtvs2[] = { m_pRenderTargetView.Get() };
	m_pDeviceContext->OMSetRenderTargets(1, rtvs2, m_pDepthStencilView.Get());

	return true;
}

void TutorialApp::UninitD3D()
{
	// ComPtr 사용으로 자동 해제
}

bool TutorialApp::InitScene()
{
	HRESULT hr = 0;
	ID3D10Blob* errorMessage = nullptr;

	// 버텍스 정의 (설정)
	Vertex vertices[] =
	{
		// 앞면 (Z-)
		{ Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.0f, 0.0f, -1.0f) },
		{ Vector3(-0.5f,  0.5f, -0.5f), Vector3(0.0f, 0.0f, -1.0f) },
		{ Vector3( 0.5f,  0.5f, -0.5f), Vector3(0.0f, 0.0f, -1.0f) },
		{ Vector3( 0.5f, -0.5f, -0.5f), Vector3(0.0f, 0.0f, -1.0f) },

		// 뒷면 (Z+)
		{ Vector3(-0.5f, -0.5f, 0.5f), Vector3(0.0f, 0.0f, 1.0f) },
		{ Vector3( 0.5f, -0.5f, 0.5f), Vector3(0.0f, 0.0f, 1.0f) },
		{ Vector3( 0.5f,  0.5f, 0.5f), Vector3(0.0f, 0.0f, 1.0f) },
		{ Vector3(-0.5f,  0.5f, 0.5f), Vector3(0.0f, 0.0f, 1.0f) },

		// 왼쪽면(X-)
		{ Vector3(-0.5f, -0.5f,  0.5f), Vector3(-1.0f, 0.0f, 0.0f) },
		{ Vector3(-0.5f,  0.5f,  0.5f), Vector3(-1.0f, 0.0f, 0.0f) },
		{ Vector3(-0.5f,  0.5f, -0.5f), Vector3(-1.0f, 0.0f, 0.0f) },
		{ Vector3(-0.5f, -0.5f, -0.5f), Vector3(-1.0f, 0.0f, 0.0f) },

		// 오른쪽면 (X+)
		{ Vector3(0.5f, -0.5f, -0.5f), Vector3(1.0f, 0.0f, 0.0f) },
		{ Vector3(0.5f,  0.5f, -0.5f), Vector3(1.0f, 0.0f, 0.0f) },
		{ Vector3(0.5f,  0.5f,  0.5f), Vector3(1.0f, 0.0f, 0.0f) },
		{ Vector3(0.5f, -0.5f,  0.5f), Vector3(1.0f, 0.0f, 0.0f) },

		// 윗면 (Y+)
		{ Vector3(-0.5f, 0.5f, -0.5f), Vector3(0.0f, 1.0f, 0.0f) },
		{ Vector3(-0.5f, 0.5f,  0.5f), Vector3(0.0f, 1.0f, 0.0f) },
		{ Vector3( 0.5f, 0.5f,  0.5f), Vector3(0.0f, 1.0f, 0.0f) },
		{ Vector3( 0.5f, 0.5f, -0.5f), Vector3(0.0f, 1.0f, 0.0f) },

		// 아래면(Y-)
		{ Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.0f, -1.0f, 0.0f) },
		{ Vector3( 0.5f, -0.5f, -0.5f), Vector3(0.0f, -1.0f, 0.0f) },
		{ Vector3( 0.5f, -0.5f,  0.5f), Vector3(0.0f, -1.0f, 0.0f) },
		{ Vector3(-0.5f, -0.5f,  0.5f), Vector3(0.0f, -1.0f, 0.0f) },
	};

	// 버텍스 버퍼 생성
	D3D11_BUFFER_DESC bd = {};
	bd.ByteWidth = sizeof(Vertex) * ARRAYSIZE(vertices);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA vbData = {};
	vbData.pSysMem = vertices;
	HR_T(m_pDevice->CreateBuffer(&bd, &vbData, m_pVertexBuffer.GetAddressOf()));

	m_VertexBufferStride = sizeof(Vertex);
	m_VertexBufferOffset = 0;

	// 인덱스 정의
	WORD indices[] =
	{
		0, 1, 2,  0, 2, 3,    // 앞면
		4, 5, 6,  4, 6, 7,    // 뒷면
		8, 9, 10, 8, 10, 11,  // 왼쪽면
		12, 13, 14, 12, 14, 15, // 오른쪽면
		16, 17, 18, 16, 18, 19, // 윗면
		20, 21, 22, 20, 22, 23  // 아래면
	};

	m_nIndices = ARRAYSIZE(indices);

	// 인덱스 버퍼 생성
	bd = {};
	bd.ByteWidth = sizeof(WORD) * ARRAYSIZE(indices);
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA ibData = {};
	ibData.pSysMem = indices;
	HR_T(m_pDevice->CreateBuffer(&bd, &ibData, m_pIndexBuffer.GetAddressOf()));

	// Input Layout 생성
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	ComPtr<ID3D10Blob> vertexShaderBuffer;
	HR_T(CompileShaderFromFile(L"../Shaders/97_VertexShader.hlsl", "main", "vs_4_0", vertexShaderBuffer.GetAddressOf()));
	HR_T(m_pDevice->CreateInputLayout(layout, ARRAYSIZE(layout),
		vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), m_pInputLayout.GetAddressOf()));

	// 버텍스 셰이더 생성
	HR_T(m_pDevice->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), NULL, m_pVertexShader.GetAddressOf()));

	// 픽셀 셰이더 생성
	ComPtr<ID3D10Blob> pixelShaderBuffer;
	HR_T(CompileShaderFromFile(L"../Shaders/97_PixelShader.hlsl", "main", "ps_4_0", pixelShaderBuffer.GetAddressOf()));
	HR_T(m_pDevice->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
		pixelShaderBuffer->GetBufferSize(), NULL, m_pPixelShader.GetAddressOf()));

	// 상수 버퍼 생성
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	HR_T(m_pDevice->CreateBuffer(&bd, nullptr, m_pConstantBuffer.GetAddressOf()));

	// 초기값 설정
	XMVECTOR Eye = XMVectorSet(0.0f, 2.0f, -5.0f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	m_View = XMMatrixLookAtLH(Eye, At, Up);
	m_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, m_ClientWidth / (FLOAT)m_ClientHeight, 0.1f, 1000.0f);

	// 테스트용 Cube 생성
	CubeObject* cube = m_World.CreateGameObject<CubeObject>();
	cube->m_Name = "TestCube";
	cube->m_Position = Vector3(0.0f, 0.0f, 0.0f);
	m_pSelectedObject = cube;

	// RTTR에 등록된 GameObject 파생 클래스들을 가져와서 오브젝트 팔레트에 표시
	using namespace rttr;
	type baseType = type::get<GameObject>();
	for (auto& t : type::get_types())
	{
		// GameObject에서 파생했고 자신은 참조 타입이 아니며 클래스인 타입인 것만 추가
		if (t.is_derived_from(baseType) &&
			t != baseType &&
			!t.is_pointer() &&
			!t.is_array() &&
			t.is_class())
		{
			std::string typeName = t.get_name().to_string();
			// 이름에 '*'와 '&'가 포함된 경우는 제외
			if (typeName.find('*') == std::string::npos &&
				typeName.find('&') == std::string::npos)
			{
				m_AvailableObjectTypes.push_back(typeName);
			}
		}
	}

	// 카메라 초기화
	m_Camera.m_Position = Vector3(0.0f, 2.0f, -10.0f);
	m_Camera.m_Rotation = Vector3(0.0f, 0.0f, 0.0f);

	// DebugDraw 초기화
	HR_T(DebugDraw::Initialize(m_pDevice, m_pDeviceContext));

	return true;
}

void TutorialApp::UninitScene()
{
	// DebugDraw 해제
	DebugDraw::Uninitialize();

	// ComPtr 사용으로 자동 해제
}


void TutorialApp::RenderImGuiCubeRTTR()
{
	m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), nullptr);

	//////////////////////////////////////////////////////////////////////////
	// ImGui Rendering
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// 메뉴 바
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Save", "Ctrl+S"))
			{
				SaveScene();
			}
			if (ImGui::MenuItem("Load", "Ctrl+O"))
			{
				LoadScene();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Exit", "Alt+F4"))
			{
				PostQuitMessage(0);
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Debug"))
		{
			ImGui::MenuItem("Show Ray", nullptr, &m_bShowDebugRay);
			ImGui::MenuItem("Show All AABB", nullptr, &m_bShowAllAABB);
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	// 자유롭게 옮길 수 있는 창들
	// World Hierarchy (초기 위치만 설정, 이후 사용자가 자유롭게 이동 가능)
	ImGui::SetNextWindowPos(ImVec2(10, 30), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);
	ImGui::Begin("World Hierarchy");
	RenderWorldHierarchyContent();
	ImGui::End();

	// Object Palette
	ImGui::SetNextWindowPos(ImVec2(10, 450), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);
	ImGui::Begin("Object Palette");
	RenderObjectPaletteContent();
	ImGui::End();

	// Inspector
	ImGui::SetNextWindowPos(ImVec2(static_cast<float>(m_ClientWidth - 410), 30), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(400, 700), ImGuiCond_FirstUseEver);
	ImGui::Begin("Inspector");
	RenderInspectorContent();
	ImGui::End();

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

bool TutorialApp::InitImGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui::StyleColorsDark();

	if(!ImGui_ImplWin32_Init(m_hWnd))
		return false;

	if(!ImGui_ImplDX11_Init(m_pDevice.Get(), m_pDeviceContext.Get()))
		return false;

	return true;
}

void TutorialApp::UninitImGui()
{
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

void TutorialApp::OnInputProcess(const Keyboard::State& KeyState, const Keyboard::KeyboardStateTracker& KeyTracker,
	const Mouse::State& MouseState, const Mouse::ButtonStateTracker& MouseTracker)
{
	// 카메라로 입력 전달
	m_Camera.OnInputProcess(KeyState, KeyTracker, MouseState, MouseTracker);

	// Ray Picking: 마우스 클릭으로 오브젝트 선택
	if (MouseTracker.leftButton == Mouse::ButtonStateTracker::PRESSED)
	{
		// ImGui 창이 마우스를 캡쳐했는지 확인 (3D 뷰포트 클릭만 처리)
		if (!ImGui::GetIO().WantCaptureMouse)
		{
			// 마우스 스크린 좌표를 NDC로 변환   x,y: [-1,1]   z: [0,1]
			float x = (2.0f * MouseState.x) / m_ClientWidth - 1.0f;
			float y = 1.0f - (2.0f * MouseState.y) / m_ClientHeight;

			Matrix invViewProj = (m_View * m_Projection).Invert();

			// Near / Far in NDC (D3D: z=0 near, z=1 far)
			Vector4 nearClip(x, y, 0.0f, 1.0f);
			Vector4 farClip(x, y, 1.0f, 1.0f);

			// View는 World Space->Camera Space 변환 , Projection은 Camera Space ->Projection(Clip) Space 변환이므로
			Vector4 nearWorld = Vector4::Transform(nearClip, invViewProj);
			Vector4 farWorld = Vector4::Transform(farClip, invViewProj);

			// 투영 행렬은 w를 z에 연동시켜 원근을 만들기 때문에, 역변환 후에도 (x,y,z,w) 동차좌표가 나온다.
			// 실제 월드 좌표를 얻으려면 perspective divide로 xyz를 w로 나눠야 함.
			nearWorld /= nearWorld.w;
			farWorld /= farWorld.w;

			Vector3 rayOrigin(nearWorld.x, nearWorld.y, nearWorld.z);
			Vector3 rayDir = Vector3(farWorld.x - nearWorld.x,
				farWorld.y - nearWorld.y,
				farWorld.z - nearWorld.z);
			rayDir.Normalize();

			// Ray 생성 (화면의 점에서 시작하는 레이)
			Ray ray(Vector3(nearWorld), rayDir);

			// 디버그 레이 정보 저장 (Show Ray가 켜져있을 때만)
			if (m_bShowDebugRay)
			{
				m_DebugRayOrigin = Vector3(nearWorld);
				m_DebugRayDirection = rayDir;
			}

			// GameWorld에서 Ray 충돌 검사
			float hitDistance = 0.0f;
			GameObject* hitObject = m_World.RayCast(ray, &hitDistance);

			// 선택된 오브젝트 업데이트
			m_pSelectedObject = hitObject;
		}
	}
}

void TutorialApp::SaveScene()
{
	// 파일 저장 다이얼로그
	OPENFILENAMEA ofn = {};
	char szFile[260] = {};

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = m_hWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "JSON Files (*.json)\0*.json\0All Files (*.*)\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = "json";

	if (GetSaveFileNameA(&ofn) != TRUE)
		return; // 사용자가 취소함

	std::string filename = szFile;

	// GameWorld를 파일에 저장
	if (m_World.SaveToFile(filename))
	{
		MessageBoxA(m_hWnd, "Scene saved successfully!", "Save", MB_OK | MB_ICONINFORMATION);
	}
	else
	{
		MessageBoxA(m_hWnd, "Failed to save scene!", "Error", MB_OK | MB_ICONERROR);
	}
}

void TutorialApp::LoadScene()
{
	// 파일 열기 다이얼로그
	OPENFILENAMEA ofn = {};
	char szFile[260] = {};

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = m_hWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "JSON Files (*.json)\0*.json\0All Files (*.*)\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileNameA(&ofn) != TRUE)
		return; // 사용자가 취소함

	std::string filename = szFile;

	// GameWorld를 파일에서 로드
	if (m_World.LoadFromFile(filename))
	{
		// 첫번째 객체를 선택
		if (m_World.GetCount() > 0)
		{
			m_pSelectedObject = m_World.GetGameObjects()[0];
		}
		else
		{
			m_pSelectedObject = nullptr;
		}

		MessageBoxA(m_hWnd, "Scene loaded successfully!", "Load", MB_OK | MB_ICONINFORMATION);
	}
	else
	{
		MessageBoxA(m_hWnd, "Failed to load scene! File not found.", "Error", MB_OK | MB_ICONERROR);
	}
}

void TutorialApp::RenderObjectPaletteContent()
{
	ImGui::Text("Available Objects:");
	ImGui::Separator();

	// 사용 가능한 오브젝트 타입 목록 표시
	for (const std::string& typeName : m_AvailableObjectTypes)
	{
		ImGui::Button(typeName.c_str(), ImVec2(-1, 30));

		// 드래그 소스 설정
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
		{
			// 페이로드로 타입 이름 전달
			ImGui::SetDragDropPayload("GAMEOBJECT_TYPE", typeName.c_str(), typeName.size() + 1);
			ImGui::Text("Create %s", typeName.c_str());
			ImGui::EndDragDropSource();
		}
	}
}

void TutorialApp::RenderWorldHierarchyContent()
{
	ImGui::Text("Scene Objects (%zu)", m_World.GetCount());
	ImGui::Separator();

	// 전체 창 영역을 드롭 타겟으로 설정
	ImVec2 contentSize = ImGui::GetContentRegionAvail();

	// InvisibleButton으로 전체 영역을 드롭 타겟으로 만듦
	ImGui::InvisibleButton("##HierarchyDropZone", contentSize);

	// 드롭 타겟 설정
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GAMEOBJECT_TYPE"))
		{
			const char* typeName = (const char*)payload->Data;
			GameObject* newObj = m_World.CreateGameObjectByTypeName(typeName);
			if (newObj)
			{
				// 카메라 정면(Z축+10) 위치에 생성
				Vector3 forward = Vector3(0.0f, 0.0f, 1.0f);
				Matrix rotationMatrix = Matrix::CreateFromYawPitchRoll(m_Camera.m_Rotation.y, m_Camera.m_Rotation.x, m_Camera.m_Rotation.z);
				forward = Vector3::Transform(forward, rotationMatrix);
				newObj->m_Position = m_Camera.m_Position + forward * 10.0f;

				m_pSelectedObject = newObj;

				CubeObject* cube = dynamic_cast<CubeObject*>(newObj);
				if (cube)
				{
					static int cubeCount = 0;
					cube->m_Name = "Cube_" + std::to_string(cubeCount++);
				}
			}
		}
		ImGui::EndDragDropTarget();
	}

	// 리스트를 InvisibleButton 위에 겹쳐 그리기
	ImVec2 listPos = ImGui::GetItemRectMin();
	ImGui::SetCursorScreenPos(listPos);

	ImGui::BeginChild("HierarchyList", contentSize, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground);

	// 모든 GameObject 목록 표시
	const auto& objects = m_World.GetGameObjects();
	for (size_t i = 0; i < objects.size(); ++i)
	{
		GameObject* obj = objects[i];
		if (!obj)
			continue;

		// 선택된 오브젝트는 하이라이트
		bool isSelected = (obj == m_pSelectedObject);

		// 오브젝트 이름 가져오기(CubeObject의 경우)
		std::string objectName = "GameObject";
		CubeObject* cube = dynamic_cast<CubeObject*>(obj);
		if (cube && !cube->m_Name.empty())
		{
			objectName = cube->m_Name;
		}
		else
		{
			// RTTR로 타입 이름 가져오기
			using namespace rttr;
			type t = type::get(*obj);
			objectName = t.get_name().to_string() + "_" + std::to_string(i);
		}

		// Selectable 위젯으로 표시
		if (ImGui::Selectable(objectName.c_str(), isSelected))
		{
			m_pSelectedObject = obj;
		}

		// 우클릭 메뉴
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete"))
			{
				m_World.DestroyGameObject(obj);
				if (m_pSelectedObject == obj)
					m_pSelectedObject = nullptr;
			}
			ImGui::EndPopup();
		}
	}

	// 리스트가 비어있을 때 안내 메시지
	if (objects.empty())
	{
		ImGui::TextDisabled("(Empty - Drag objects here)");
	}

	ImGui::EndChild();
}

void TutorialApp::RenderInspectorContent()
{
	using namespace rttr;

	// 선택된 객체가 없으면 메시지 표시
	if (!m_pSelectedObject)
	{
		ImGui::Text("No GameObject selected");
		return;
	}

	type t = type::get(*m_pSelectedObject);
	ImGui::Text("Type: %s", t.get_name().to_string().c_str());
	ImGui::Separator();

	for (auto& prop : t.get_properties())
	{
		variant value = prop.get_value(*m_pSelectedObject);
		std::string name = prop.get_name().to_string();
		if (value.is_type<float>())
		{
			float v = value.get_value<float>();
			if (ImGui::DragFloat(name.c_str(), &v))
				prop.set_value(*m_pSelectedObject, v);
		}
		else if (value.is_type<std::string>())
		{
			char buf[256] = {};
			strncpy_s(buf, value.get_value<std::string>().c_str(), sizeof(buf) - 1);
			if (ImGui::InputText(name.c_str(), buf, sizeof(buf), ImGuiInputTextFlags_EnterReturnsTrue))
				prop.set_value(*m_pSelectedObject, std::string(buf));
		}
		else if (value.is_type<Vector3>())
		{
			Vector3 v = value.get_value<Vector3>();
			float arr[3] = { v.x, v.y, v.z };
			if (ImGui::DragFloat3(name.c_str(), arr))
			{
				v.x = arr[0]; v.y = arr[1]; v.z = arr[2];
				prop.set_value(*m_pSelectedObject, v);
			}
		}
		else if (value.is_type<Vector4>())
		{
			Vector4 v = value.get_value<Vector4>();
			float arr[4] = { v.x, v.y, v.z, v.w };
			if (ImGui::DragFloat4(name.c_str(), arr))
			{
				v.x = arr[0]; v.y = arr[1]; v.z = arr[2]; v.w = arr[3];
				prop.set_value(*m_pSelectedObject, v);
			}
		}
		else if (value.is_type<Color>())
		{
			Color v = value.get_value<Color>();
			float arr[4] = { v.x, v.y, v.z, v.w };
			if (ImGui::ColorEdit4(name.c_str(), arr))
			{
				v.x = arr[0]; v.y = arr[1]; v.z = arr[2]; v.w = arr[3];
				prop.set_value(*m_pSelectedObject, v);
			}
		}


		// 속성 메타데이터 가져오기
		auto desc_var = prop.get_metadata("desc");
		if (desc_var.is_valid() && ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("%s", desc_var.to_string().c_str());
		}
	}
}
