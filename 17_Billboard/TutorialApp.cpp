#include "TutorialApp.h"
#include "../Common/Helper.h"
#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

bool TutorialApp::OnInitialize()
{
	if (!InitD3D())
		return false;

	if (!CreateQuadGeometry())
		return false;

	if (!CreateShaders())
		return false;

	// 빌보드 행렬 초기화
	m_BillboardMatrix = Matrix::Identity;
	m_BillboardPrevPosition = m_BillboardPosition;
	m_BillboardVelocity = Vector3::Zero;

	m_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, m_ClientWidth / (FLOAT)m_ClientHeight, 1.0f, 10000.0f);
	// ImGui 초기화
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(m_hWnd);
	ImGui_ImplDX11_Init(m_pDevice, m_pDeviceContext);

	// 카메라 초기 위치 설정 (왼손 좌표계: 카메라를 +Z에 놓고 -Z 방향을 봄)
	m_Camera.m_PositionInitial = Vector3(0.0f, 0.0f, -20.0f);
	m_Camera.Reset();

	return true;
}

void TutorialApp::OnUninitialize()
{
	// ImGui 정리
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	UninitD3D();
}

void TutorialApp::OnUpdate()
{
	// Camera 클래스가 자동으로 입력 처리
	m_Camera.GetViewMatrix(m_View);

	const Vector3 billboardPos = m_BillboardPosition;
	const float dt = m_Timer.DeltaTime();
	if (dt > 0.000001f)
	{
		m_BillboardVelocity = (billboardPos - m_BillboardPrevPosition) / dt;
	}
	else
	{
		m_BillboardVelocity = Vector3::Zero;
	}
	m_BillboardPrevPosition = billboardPos;

	// Billboard 타입에 따른 빌보드 행렬 계산
	switch (m_CurrentBillboardType)
	{
	case BillboardType::Identity:
	{
		// 빌보드 없음 - 단위 행렬
		m_BillboardMatrix = Matrix::Identity;
		break;
	}
	case BillboardType::YAxisLocked:
	{
		// Y축 고정 빌보드 (연기/불꽃)
		// 카메라 방향을 XZ 평면에 투영하여 회전
		Vector3 cameraPos = m_Camera.m_Position;
		// 쿼드의 앞면 노멀(-Z)이 카메라를 향하도록, 카메라->빌보드 방향을 사용
		Vector3 forward = billboardPos - cameraPos;
		forward.y = 0.0f; // Y축 제거

		if (forward.LengthSquared() < 0.0001f)
		{
			// 카메라가 정확히 위나 아래에 있으면 기본 방향 사용
			forward = Vector3::Forward;
		}
		else
		{
			forward.Normalize();
		}

		Vector3 up = Vector3::Up;
		Vector3 right = up.Cross(forward);
		right.Normalize();

		m_BillboardMatrix = Matrix::Identity;
		m_BillboardMatrix._11 = right.x;
		m_BillboardMatrix._12 = right.y;
		m_BillboardMatrix._13 = right.z;
		m_BillboardMatrix._21 = up.x;
		m_BillboardMatrix._22 = up.y;
		m_BillboardMatrix._23 = up.z;
		m_BillboardMatrix._31 = forward.x;
		m_BillboardMatrix._32 = forward.y;
		m_BillboardMatrix._33 = forward.z;
		break;
	}
	case BillboardType::Spherical:
	{
		// Spherical 빌보드 (스파크)
		// 완전히 카메라를 향함
		Vector3 cameraPos = m_Camera.m_Position;
		// 쿼드 앞면(-Z)이 카메라를 향하도록 카메라->빌보드 방향 사용
		Vector3 forward = billboardPos - cameraPos;
		if (forward.LengthSquared() < 0.0001f)
			forward = Vector3::Forward;
		else
			forward.Normalize();

		Vector3 up = Vector3::Up;
		// forward가 up과 평행한 경우 처리
		if (abs(forward.Dot(up)) > 0.999f)
			up = Vector3::Right;

		Vector3 right = up.Cross(forward);
		right.Normalize();
		up = forward.Cross(right);
		up.Normalize();

		m_BillboardMatrix = Matrix::Identity;
		m_BillboardMatrix._11 = right.x;
		m_BillboardMatrix._12 = right.y;
		m_BillboardMatrix._13 = right.z;
		m_BillboardMatrix._21 = up.x;
		m_BillboardMatrix._22 = up.y;
		m_BillboardMatrix._23 = up.z;
		m_BillboardMatrix._31 = forward.x;
		m_BillboardMatrix._32 = forward.y;
		m_BillboardMatrix._33 = forward.z;
		break;
	}
	case BillboardType::ScreenAligned:
	{
		// Screen Aligned 빌보드 (UI 이펙트)
		// 뷰 행렬의 역행렬에서 회전 부분만 추출
		Matrix viewInverse = m_View.Invert();
		m_BillboardMatrix = Matrix::Identity;
		m_BillboardMatrix._11 = viewInverse._11;
		m_BillboardMatrix._12 = viewInverse._12;
		m_BillboardMatrix._13 = viewInverse._13;
		m_BillboardMatrix._21 = viewInverse._21;
		m_BillboardMatrix._22 = viewInverse._22;
		m_BillboardMatrix._23 = viewInverse._23;
		m_BillboardMatrix._31 = viewInverse._31;
		m_BillboardMatrix._32 = viewInverse._32;
		m_BillboardMatrix._33 = viewInverse._33;
		break;
	}
	}

	// 최종 월드 행렬 = (고정 스케일) * (타입별 빌보드 회전) * (위치이동)
	// 위치(m_BillboardPosition)만 외부에서 관리하고, 정렬(회전)은 타입에 의해 결정.
	constexpr float kBillboardUniformScale = 10.0f;
	m_World = Matrix::CreateScale(kBillboardUniformScale) * m_BillboardMatrix * Matrix::CreateTranslation(m_BillboardPosition);
}

void TutorialApp::OnRender()
{
	// 렌더타겟 설정
	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, nullptr);

	// 백버퍼 클리어
	const float clearColor[4] = { m_ClearColor.x, m_ClearColor.y, m_ClearColor.z, m_ClearColor.w };
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, clearColor);

	// 상수 버퍼 업데이트
	ConstantBuffer cb;
	cb.world = m_World.Transpose();
	cb.view = m_View.Transpose();
	cb.projection = m_Projection.Transpose();
	m_pDeviceContext->UpdateSubresource(m_pConstantBuffer, 0, nullptr, &cb, 0, 0);

	// Input Assembler 설정
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
	m_pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pDeviceContext->IASetInputLayout(m_pInputLayout);

	// 셰이더 설정
	m_pDeviceContext->VSSetShader(m_pVertexShader, nullptr, 0);
	m_pDeviceContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	m_pDeviceContext->PSSetShader(m_pPixelShader, nullptr, 0);

	// 사각형 그리기 (2개의 삼각형 = 6개 인덱스)
	m_pDeviceContext->DrawIndexed(6, 0, 0);

	// ImGui 렌더링
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// 카메라 정보 윈도우
	ImGui::Begin("Camera Info");

	// 카메라 위치
	ImGui::Text("Camera Position:");
	ImGui::Text("  X: %.2f, Y: %.2f, Z: %.2f", m_Camera.m_Position.x, m_Camera.m_Position.y, m_Camera.m_Position.z);

	ImGui::Separator();

	// 카메라 회전 (라디안 -> 도)
	ImGui::Text("Camera Rotation:");
	ImGui::Text("  Pitch: %.2f degrees", m_Camera.m_Rotation.x * 180.0f / DirectX::XM_PI);
	ImGui::Text("  Yaw: %.2f degrees", m_Camera.m_Rotation.y * 180.0f / DirectX::XM_PI);

	ImGui::Separator();

	// 카메라 속도
	ImGui::Text("Camera Speed: %.2f", m_Camera.m_MoveSpeed);

	ImGui::Separator();

	// 뷰 행렬
	ImGui::Text("View Matrix:");
	ImGui::Text("  [%.2f, %.2f, %.2f, %.2f]", m_View._11, m_View._12, m_View._13, m_View._14);
	ImGui::Text("  [%.2f, %.2f, %.2f, %.2f]", m_View._21, m_View._22, m_View._23, m_View._24);
	ImGui::Text("  [%.2f, %.2f, %.2f, %.2f]", m_View._31, m_View._32, m_View._33, m_View._34);
	ImGui::Text("  [%.2f, %.2f, %.2f, %.2f]", m_View._41, m_View._42, m_View._43, m_View._44);

	ImGui::Separator();

	// Billboard Type 선택
	ImGui::Text("Billboard Type:");
	const char* billboardTypes[] = {
		"Identity",
		"Y-Axis Locked (Smoke/Fire)",
		"Spherical (Sparks)",		
		"Screen Aligned (UI)"
	};
	int currentType = (int)m_CurrentBillboardType;
	if (ImGui::Combo("Type", &currentType, billboardTypes, 4)) {
		m_CurrentBillboardType = (BillboardType)currentType;
	}

	ImGui::End();

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	// 화면에 표시
	m_pSwapChain->Present(0, 0);
}

bool TutorialApp::InitD3D()
{
	HRESULT hr = 0;

	// Create DXGI factory
	HR_T(CreateDXGIFactory1(__uuidof(IDXGIFactory4), (void**)m_pDXGIFactory.GetAddressOf()));
	HR_T(m_pDXGIFactory->EnumAdapters(0, reinterpret_cast<IDXGIAdapter**>(m_pDXGIAdapter.GetAddressOf())));

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

	// 디바이스 및 스왑체인 생성
	HR_T(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, creationFlags, NULL, NULL,
		D3D11_SDK_VERSION, &swapDesc, &m_pSwapChain, &m_pDevice, NULL, &m_pDeviceContext));

	// 렌더타겟뷰 생성
	ID3D11Texture2D* pBackBufferTexture = nullptr;
	HR_T(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBufferTexture));
	HR_T(m_pDevice->CreateRenderTargetView(pBackBufferTexture, NULL, &m_pRenderTargetView));
	SAFE_RELEASE(pBackBufferTexture);

	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, NULL);

	// 뷰포트 설정
	D3D11_VIEWPORT viewport = {};
	viewport.Width = (float)m_ClientWidth;
	viewport.Height = (float)m_ClientHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	m_pDeviceContext->RSSetViewports(1, &viewport);

	return true;
}

void TutorialApp::UninitD3D()
{
	SAFE_RELEASE(m_pConstantBuffer);
	SAFE_RELEASE(m_pInputLayout);
	SAFE_RELEASE(m_pPixelShader);
	SAFE_RELEASE(m_pVertexShader);
	SAFE_RELEASE(m_pIndexBuffer);
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pRenderTargetView);
	SAFE_RELEASE(m_pSwapChain);
	SAFE_RELEASE(m_pDeviceContext);
	SAFE_RELEASE(m_pDevice);
}

bool TutorialApp::CreateQuadGeometry()
{
	// 사각형 정점 데이터 (중앙 기준 0.5 크기)
	Vertex vertices[] = {
		{ Vector3(-0.5f,  0.5f, 0.0f), Vector4(1.0f, 0.0f, 0.0f, 1.0f) },  // 좌상 - 빨강
		{ Vector3( 0.5f,  0.5f, 0.0f), Vector4(0.0f, 1.0f, 0.0f, 1.0f) },  // 우상 - 초록
		{ Vector3( 0.5f, -0.5f, 0.0f), Vector4(0.0f, 0.0f, 1.0f, 1.0f) },  // 우하 - 파랑
		{ Vector3(-0.5f, -0.5f, 0.0f), Vector4(1.0f, 1.0f, 0.0f, 1.0f) }   // 좌하 - 노랑
	};

	// 버텍스 버퍼 생성
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(vertices);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = vertices;

	HRESULT hr = m_pDevice->CreateBuffer(&bufferDesc, &initData, &m_pVertexBuffer);
	if (FAILED(hr))
		return false;

	// 인덱스 데이터 (2개의 삼각형)
	UINT indices[] = {
		0, 1, 2,  // 첫 번째 삼각형
		0, 2, 3   // 두 번째 삼각형
	};

	// 인덱스 버퍼 생성
	bufferDesc.ByteWidth = sizeof(indices);
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	initData.pSysMem = indices;

	hr = m_pDevice->CreateBuffer(&bufferDesc, &initData, &m_pIndexBuffer);
	if (FAILED(hr))
		return false;

	// 상수 버퍼 생성
	bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(ConstantBuffer);
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	hr = m_pDevice->CreateBuffer(&bufferDesc, nullptr, &m_pConstantBuffer);
	if (FAILED(hr))
		return false;

	return true;
}

bool TutorialApp::CreateShaders()
{
	HRESULT hr;

	// 버텍스 셰이더 코드
	const char* vsSource = R"(
		cbuffer ConstantBuffer : register(b0)
		{
			matrix World;
			matrix View;
			matrix Projection;
		};

		struct VS_INPUT
		{
			float3 Position : POSITION;
			float4 Color : COLOR;
		};

		struct VS_OUTPUT
		{
			float4 Position : SV_POSITION;
			float4 Color : COLOR;
		};

		VS_OUTPUT main(VS_INPUT input)
		{
			VS_OUTPUT output;
			float4 worldPos = mul(float4(input.Position, 1.0f), World);
			float4 viewPos = mul(worldPos, View);
			output.Position = mul(viewPos, Projection);
			output.Color = input.Color;
			return output;
		}
	)";

	// 픽셀 셰이더 코드
	const char* psSource = R"(
		struct PS_INPUT
		{
			float4 Position : SV_POSITION;
			float4 Color : COLOR;
		};

		float4 main(PS_INPUT input) : SV_TARGET
		{
			return input.Color;
		}
	)";

	// 버텍스 셰이더 컴파일
	ID3DBlob* pVSBlob = nullptr;
	ID3DBlob* pErrorBlob = nullptr;

	hr = D3DCompile(vsSource, strlen(vsSource), nullptr, nullptr, nullptr,
		"main", "vs_5_0", 0, 0, &pVSBlob, &pErrorBlob);

	if (FAILED(hr))
	{
		if (pErrorBlob)
			pErrorBlob->Release();
		return false;
	}

	hr = m_pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), nullptr, &m_pVertexShader);

	if (FAILED(hr))
	{
		pVSBlob->Release();
		return false;
	}

	// Input Layout 생성
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	hr = m_pDevice->CreateInputLayout(layout, 2,
		pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pInputLayout);

	pVSBlob->Release();

	if (FAILED(hr))
		return false;

	// 픽셀 셰이더 컴파일
	ID3DBlob* pPSBlob = nullptr;
	hr = D3DCompile(psSource, strlen(psSource), nullptr, nullptr, nullptr,
		"main", "ps_5_0", 0, 0, &pPSBlob, &pErrorBlob);

	if (FAILED(hr))
	{
		if (pErrorBlob)
			pErrorBlob->Release();
		return false;
	}

	hr = m_pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(),
		pPSBlob->GetBufferSize(), nullptr, &m_pPixelShader);

	pPSBlob->Release();

	if (FAILED(hr))
		return false;

	return true;
}

// Forward declare ImGui Win32 handler
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK TutorialApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// ImGui 메시지 처리
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
		return true;

	return __super::WndProc(hWnd, message, wParam, lParam);
}
