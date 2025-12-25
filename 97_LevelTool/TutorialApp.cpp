#include "TutorialApp.h"
#include "../Common/Helper.h"
#include <d3dcompiler.h>
#include "CubeObject.h"
#include <imgui.h>
#include <rttr/registration>


#pragma comment (lib, "d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")


// 정점 구조체
struct Vertex
{
	Vector3 Pos;		// 위치 정보.
	Vector3 Normal;		// 법선 정보.
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
	// CubeObject의 회전값을 갱신
	
	m_Cube.UpdateAABB();
	
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

	// 상수 버퍼 업데이트
	ConstantBuffer cb;
	cb.mWorld = m_Cube.GetWorldMatrix().Transpose();
	cb.mView = XMMatrixTranspose(m_View);
	cb.mProjection = XMMatrixTranspose(m_Projection);
	cb.vLightDir = m_LightDir;
	cb.vLightColor = m_LightColor;
	cb.vMaterialColor = m_Cube.m_Color;
	m_pDeviceContext->UpdateSubresource(m_pConstantBuffer.Get(), 0, nullptr, &cb, 0, 0);

	// 렌더 큐브
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

	// ImGui로 CubeObject 멤버를 조회/수정하는 UI 함수 추가
	//RenderImGuiCube();

	// RTTR로 CubeObject 멤버를 조회해 ImGui UI를 자동 생성하는 함수 호출
	RenderImGuiCubeRTTR();

	// Present
	m_pSwapChain->Present(0, 0);
}

bool TutorialApp::InitD3D()
{
	HRESULT hr = 0;

	// 스왑체인 속성 설정 구조체 생성.
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

	// 디바이스, 스왑체인, 디바이스 컨텍스트 생성.
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

	// 버텍스 정의 (큐브)
	Vertex vertices[] =
	{
		// 앞면 (Z-)
		{ Vector3(-1.0f, -1.0f, -1.0f), Vector3(0.0f, 0.0f, -1.0f) },
		{ Vector3(-1.0f,  1.0f, -1.0f), Vector3(0.0f, 0.0f, -1.0f) },
		{ Vector3( 1.0f,  1.0f, -1.0f), Vector3(0.0f, 0.0f, -1.0f) },
		{ Vector3( 1.0f, -1.0f, -1.0f), Vector3(0.0f, 0.0f, -1.0f) },

		// 뒷면 (Z+)
		{ Vector3(-1.0f, -1.0f, 1.0f), Vector3(0.0f, 0.0f, 1.0f) },
		{ Vector3( 1.0f, -1.0f, 1.0f), Vector3(0.0f, 0.0f, 1.0f) },
		{ Vector3( 1.0f,  1.0f, 1.0f), Vector3(0.0f, 0.0f, 1.0f) },
		{ Vector3(-1.0f,  1.0f, 1.0f), Vector3(0.0f, 0.0f, 1.0f) },

		// 왼쪽면 (X-)
		{ Vector3(-1.0f, -1.0f,  1.0f), Vector3(-1.0f, 0.0f, 0.0f) },
		{ Vector3(-1.0f,  1.0f,  1.0f), Vector3(-1.0f, 0.0f, 0.0f) },
		{ Vector3(-1.0f,  1.0f, -1.0f), Vector3(-1.0f, 0.0f, 0.0f) },
		{ Vector3(-1.0f, -1.0f, -1.0f), Vector3(-1.0f, 0.0f, 0.0f) },

		// 오른쪽면 (X+)
		{ Vector3(1.0f, -1.0f, -1.0f), Vector3(1.0f, 0.0f, 0.0f) },
		{ Vector3(1.0f,  1.0f, -1.0f), Vector3(1.0f, 0.0f, 0.0f) },
		{ Vector3(1.0f,  1.0f,  1.0f), Vector3(1.0f, 0.0f, 0.0f) },
		{ Vector3(1.0f, -1.0f,  1.0f), Vector3(1.0f, 0.0f, 0.0f) },

		// 윗면 (Y+)
		{ Vector3(-1.0f, 1.0f, -1.0f), Vector3(0.0f, 1.0f, 0.0f) },
		{ Vector3(-1.0f, 1.0f,  1.0f), Vector3(0.0f, 1.0f, 0.0f) },
		{ Vector3( 1.0f, 1.0f,  1.0f), Vector3(0.0f, 1.0f, 0.0f) },
		{ Vector3( 1.0f, 1.0f, -1.0f), Vector3(0.0f, 1.0f, 0.0f) },

		// 아랫면 (Y-)
		{ Vector3(-1.0f, -1.0f, -1.0f), Vector3(0.0f, -1.0f, 0.0f) },
		{ Vector3( 1.0f, -1.0f, -1.0f), Vector3(0.0f, -1.0f, 0.0f) },
		{ Vector3( 1.0f, -1.0f,  1.0f), Vector3(0.0f, -1.0f, 0.0f) },
		{ Vector3(-1.0f, -1.0f,  1.0f), Vector3(0.0f, -1.0f, 0.0f) },
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
		20, 21, 22, 20, 22, 23  // 아랫면
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
	m_World = XMMatrixIdentity();
	XMVECTOR Eye = XMVectorSet(0.0f, 2.0f, -5.0f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	m_View = XMMatrixLookAtLH(Eye, At, Up);
	m_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, m_ClientWidth / (FLOAT)m_ClientHeight, 0.1f, 1000.0f);

	return true;
}

void TutorialApp::UninitScene()
{
	// ComPtr 사용으로 자동 해제
}

void TutorialApp::RenderImGuiCube()
{
    ImGui::Begin("CubeObject Inspector");
    ImGui::Text("Name: %s", m_Cube.m_Name.c_str());
    ImGui::InputFloat3("Position", &m_Cube.m_Position.x);
    ImGui::InputFloat3("Rotation", &m_Cube.m_Rotation.x);
    ImGui::InputFloat3("Scale", &m_Cube.m_Scale.x);
    ImGui::ColorEdit4("Color", &m_Cube.m_Color.x);
    ImGui::InputFloat("Value", &m_Cube.m_Value);
    ImGui::End();
}

void TutorialApp::RenderImGuiCubeRTTR()
{
	m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), nullptr);

	//////////////////////////////////////////////////////////////////////////
	 // ImGui Rendering
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// 메뉴 바 추가
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
		ImGui::EndMainMenuBar();
	}

    using namespace rttr;
    ImGui::Begin("CubeObject RTTR Inspector");
    type t = type::get(m_Cube);
    for (auto& prop : t.get_properties())
    {
        variant value = prop.get_value(m_Cube);
        std::string name = prop.get_name().to_string();
        if (value.is_type<float>())
        {
            float v = value.get_value<float>();
            ImGui::InputFloat(name.c_str(), &v, 0.0f, 0.0f, "%.3f");
            if (ImGui::IsItemDeactivatedAfterEdit())
                prop.set_value(m_Cube, v);
        }
        else if (value.is_type<std::string>())
        {
            char buf[256] = {};
            strncpy_s(buf, value.get_value<std::string>().c_str(), sizeof(buf)-1);
            if (ImGui::InputText(name.c_str(), buf, sizeof(buf), ImGuiInputTextFlags_EnterReturnsTrue))
                prop.set_value(m_Cube, std::string(buf));
        }
        else if (value.is_type<Vector3>())
        {
            Vector3 v = value.get_value<Vector3>();
            float arr[3] = { v.x, v.y, v.z };
            ImGui::InputFloat3(name.c_str(), arr, "%.3f");
            if (ImGui::IsItemDeactivatedAfterEdit())
            {
                v.x = arr[0]; v.y = arr[1]; v.z = arr[2];
                prop.set_value(m_Cube, v);
            }
        }
        else if (value.is_type<Vector4>())
        {
            Vector4 v = value.get_value<Vector4>();
            float arr[4] = { v.x, v.y, v.z, v.w };
            if (ImGui::ColorEdit4(name.c_str(), arr))
            {
                v.x = arr[0]; v.y = arr[1]; v.z = arr[2]; v.w = arr[3];
                prop.set_value(m_Cube, v);
            }
        }

		// 설명 메타데이터 가져오기
		auto desc_var = prop.get_metadata("desc");
		if (desc_var.is_valid() && ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("%s", desc_var.to_string().c_str());
		}
    }
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

void TutorialApp::SaveScene()
{
	// TODO: 파일 다이얼로그를 통해 저장 경로 선택
	// 임시로 고정 경로 사용
	std::string filename = "scene.json";

	// JSON 형식으로 저장 (간단한 구현)
	FILE* file = nullptr;
	fopen_s(&file, filename.c_str(), "w");
	if (file)
	{
		fprintf(file, "{\n");
		fprintf(file, "  \"Name\": \"%s\",\n", m_Cube.m_Name.c_str());
		fprintf(file, "  \"Position\": [%.3f, %.3f, %.3f],\n", m_Cube.m_Position.x, m_Cube.m_Position.y, m_Cube.m_Position.z);
		fprintf(file, "  \"Rotation\": [%.3f, %.3f, %.3f],\n", m_Cube.m_Rotation.x, m_Cube.m_Rotation.y, m_Cube.m_Rotation.z);
		fprintf(file, "  \"Scale\": [%.3f, %.3f, %.3f],\n", m_Cube.m_Scale.x, m_Cube.m_Scale.y, m_Cube.m_Scale.z);
		fprintf(file, "  \"Color\": [%.3f, %.3f, %.3f, %.3f],\n", m_Cube.m_Color.x, m_Cube.m_Color.y, m_Cube.m_Color.z, m_Cube.m_Color.w);
		fprintf(file, "  \"Value\": %.3f\n", m_Cube.m_Value);
		fprintf(file, "}\n");
		fclose(file);

		MessageBoxA(m_hWnd, "Scene saved successfully!", "Save", MB_OK | MB_ICONINFORMATION);
	}
	else
	{
		MessageBoxA(m_hWnd, "Failed to save scene!", "Error", MB_OK | MB_ICONERROR);
	}
}

void TutorialApp::LoadScene()
{
	// TODO: 파일 다이얼로그를 통해 로드 경로 선택
	// 임시로 고정 경로 사용
	std::string filename = "scene.json";

	FILE* file = nullptr;
	fopen_s(&file, filename.c_str(), "r");
	if (file)
	{
		char name[256];
		Vector3 pos, rot, scale;
		Vector4 color;
		float value;

		// 간단한 JSON 파싱 (실제로는 JSON 라이브러리 사용 권장)
		fscanf_s(file, "{\n");
		fscanf_s(file, "  \"Name\": \"%[^\"]\",\n", name, (unsigned)_countof(name));
		fscanf_s(file, "  \"Position\": [%f, %f, %f],\n", &pos.x, &pos.y, &pos.z);
		fscanf_s(file, "  \"Rotation\": [%f, %f, %f],\n", &rot.x, &rot.y, &rot.z);
		fscanf_s(file, "  \"Scale\": [%f, %f, %f],\n", &scale.x, &scale.y, &scale.z);
		fscanf_s(file, "  \"Color\": [%f, %f, %f, %f],\n", &color.x, &color.y, &color.z, &color.w);
		fscanf_s(file, "  \"Value\": %f\n", &value);

		m_Cube.m_Name = name;
		m_Cube.m_Position = pos;
		m_Cube.m_Rotation = rot;
		m_Cube.m_Scale = scale;
		m_Cube.m_Color = color;
		m_Cube.m_Value = value;

		fclose(file);

		MessageBoxA(m_hWnd, "Scene loaded successfully!", "Load", MB_OK | MB_ICONINFORMATION);
	}
	else
	{
		MessageBoxA(m_hWnd, "Failed to load scene! File not found.", "Error", MB_OK | MB_ICONERROR);
	}
}