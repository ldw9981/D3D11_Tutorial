#include "TutorialApp.h"
#include "../Common/Helper.h"
#include <d3dcompiler.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

namespace
{
    struct CBGeometry
    {
        Matrix World;
        Matrix View;
        Matrix Projection;
        Vector4 BaseColor;
    };

    struct CBLight
    {
        Vector4 LightPosVS_Radius; // xyz = positionVS, w = radius
        Vector4 LightColor; // rgb = light color
        Vector4 Ambient; // xyz ambient, w unused
    };
}

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
    m_Camera.GetViewMatrix(m_View);

    // Simple light animation
    float t = GameTimer::m_Instance->TotalTime();
	
    m_LightPosWorld.x = 10.0f * cosf(t + m_LightVariance.x);
    m_LightPosWorld.y = 10.0f * sinf(t + m_LightVariance.y);
    m_LightPosWorld.z = 10.0f * cosf(t + m_LightVariance.z);	
}

void TutorialApp::OnRender()
{
    if(m_UseDeferredRendering)
        RenderDeferred();
    else
		RenderFoward();
}

void TutorialApp::RenderFoward()
{
	// 1) Geometry pass -> GBuffer MRTs
	float clearBaseColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float clearNormal[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	float clearPos[4] = { 0, 0, 0, 1 };




	// 2) Light pass -> back buffer
	float clearBB[4] = { 0.2f, 0.0f, 0.2f, 1.0f }; // 디버그: 보라색으로 변경
	m_pDeviceContext->OMSetRenderTargets(1, m_pBackBufferRTV.GetAddressOf(), nullptr);
	m_pDeviceContext->ClearRenderTargetView(m_pBackBufferRTV.Get(), clearBB);
	CBGeometry cbGeom;
	cbGeom.World = m_World.Transpose();
	cbGeom.View = m_View.Transpose();
	cbGeom.Projection = m_Projection.Transpose();
	cbGeom.BaseColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	m_pDeviceContext->UpdateSubresource(m_pCBGeometry.Get(), 0, nullptr, &cbGeom, 0, 0);

	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pDeviceContext->IASetVertexBuffers(0, 1, m_pCubeVB.GetAddressOf(), &m_CubeVBStride, &m_CubeVBOffset);
	m_pDeviceContext->IASetIndexBuffer(m_pCubeIB.Get(), DXGI_FORMAT_R16_UINT, 0);
	m_pDeviceContext->IASetInputLayout(m_pCubeInputLayout.Get());

	m_pDeviceContext->VSSetShader(m_pGBufferVS.Get(), nullptr, 0);
	m_pDeviceContext->VSSetConstantBuffers(0, 1, m_pCBGeometry.GetAddressOf());
	m_pDeviceContext->PSSetShader(m_pGBufferPS.Get(), nullptr, 0);
	m_pDeviceContext->PSSetConstantBuffers(0, 1, m_pCBGeometry.GetAddressOf());

	m_pDeviceContext->DrawIndexed(m_CubeIndexCount, 0, 0);

	// ImGui Rendering
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// ImGui Window - Settings
	ImGui::Begin("Forward Rendering Settings");
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::Separator();

    ImGui::Checkbox("Deferred Rendering", (bool*)&m_UseDeferredRendering);
	ImGui::Text("Camera Info");
	ImGui::Text("Position: (%.2f, %.2f, %.2f)", m_Camera.m_Position.x, m_Camera.m_Position.y, m_Camera.m_Position.z);
	Vector3 forward = m_Camera.GetForward();
	ImGui::Text("Forward: (%.2f, %.2f, %.2f)", forward.x, forward.y, forward.z);
	ImGui::Separator();

	ImGui::Text("Light Settings");	ImGui::Text("Light Position: (%.2f, %.2f, %.2f)", m_LightPosWorld.x, m_LightPosWorld.y, m_LightPosWorld.z);	ImGui::Text("Light Position: (%.2f, %.2f, %.2f)", m_LightPosWorld.x, m_LightPosWorld.y, m_LightPosWorld.z);
	ImGui::ColorEdit3("Light Color", (float*)&m_LightColor);
	ImGui::SliderFloat("Light Radius", &m_LightRadius, 1.0f, 20.0f);	
	ImGui::Separator();
	
	ImGui::End();

	// ImGui Window - G-Buffer Debug View
	ImGui::Begin("G-Buffer Debug View");

	// 첫 번째 줄: Color, Normal
	ImGui::BeginGroup();
	ImGui::Text("Color (Albedo)");
	ImGui::Image((ImTextureID)m_pGBufferSRVs[0].Get(), ImVec2(128, 128));
	ImGui::EndGroup();

	ImGui::SameLine();

	ImGui::BeginGroup();
	ImGui::Text("Normal");
	ImGui::Image((ImTextureID)m_pGBufferSRVs[1].Get(), ImVec2(128, 128));
	ImGui::EndGroup();

	// 두 번째 줄: Position, Depth
	ImGui::BeginGroup();
	ImGui::Text("Position");
	ImGui::Image((ImTextureID)m_pGBufferSRVs[2].Get(), ImVec2(128, 128));
	ImGui::EndGroup();

	ImGui::SameLine();

	ImGui::BeginGroup();
	ImGui::Text("Depth Buffer");
	ImGui::Image((ImTextureID)m_pDepthSRV.Get(), ImVec2(128, 128));
	ImGui::EndGroup();

	ImGui::End();

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	HR_T(m_pSwapChain->Present(0, 0));
}

void TutorialApp::RenderDeferred()
{
	// 1) Geometry pass -> GBuffer MRTs
	float clearAlbedo[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float clearNormal[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	float clearPos[4] = { 0, 0, 0, 1 };

	m_pDeviceContext->ClearRenderTargetView(m_pGBufferRTVs[0].Get(), clearAlbedo);
	m_pDeviceContext->ClearRenderTargetView(m_pGBufferRTVs[1].Get(), clearNormal);
	m_pDeviceContext->ClearRenderTargetView(m_pGBufferRTVs[2].Get(), clearPos);
	m_pDeviceContext->ClearDepthStencilView(m_pDepthDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	ID3D11RenderTargetView* rtvs[GBufferCount] = { m_pGBufferRTVs[0].Get(), m_pGBufferRTVs[1].Get(), m_pGBufferRTVs[2].Get() };
	m_pDeviceContext->OMSetRenderTargets(GBufferCount, rtvs, m_pDepthDSV.Get());

	// 파이프라인 상태 설정
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pDeviceContext->IASetVertexBuffers(0, 1, m_pCubeVB.GetAddressOf(), &m_CubeVBStride, &m_CubeVBOffset);
	m_pDeviceContext->IASetIndexBuffer(m_pCubeIB.Get(), DXGI_FORMAT_R16_UINT, 0);
	m_pDeviceContext->IASetInputLayout(m_pCubeInputLayout.Get());
	m_pDeviceContext->VSSetShader(m_pGBufferVS.Get(), nullptr, 0);
	m_pDeviceContext->VSSetConstantBuffers(0, 1, m_pCBGeometry.GetAddressOf());
	m_pDeviceContext->PSSetShader(m_pGBufferPS.Get(), nullptr, 0);
	m_pDeviceContext->PSSetConstantBuffers(0, 1, m_pCBGeometry.GetAddressOf());

	// 5x5 그리드로 25개 큐브 렌더링 (XY 평면)
	CBGeometry cbGeom;
	cbGeom.View = m_View.Transpose();
	cbGeom.Projection = m_Projection.Transpose();
    cbGeom.BaseColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

	const int gridSize = 5;
	const float spacing = 5.0f;
	const float offset = (gridSize - 1) * spacing * 0.5f; // 중앙 정렬
    
	for (int y = 0; y < gridSize; y++)
	{
		for (int x = 0; x < gridSize; x++)
		{
			for (int z = 0; z < gridSize; z++)
			{
				Vector3 position(
					x * spacing - offset,
					y * spacing - offset,
					z * spacing - offset
				);

				Matrix world = Matrix::CreateTranslation(position);
				cbGeom.World = world.Transpose();

				m_pDeviceContext->UpdateSubresource(m_pCBGeometry.Get(), 0, nullptr, &cbGeom, 0, 0);
				m_pDeviceContext->DrawIndexed(m_CubeIndexCount, 0, 0);
            }
		}
	}
    
	// 2) Light pass -> back buffer
	float clearBB[4] = { 0.2f, 0.0f, 0.2f, 1.0f }; // 디버그: 보라색으로 변경
	m_pDeviceContext->OMSetRenderTargets(1, m_pBackBufferRTV.GetAddressOf(), nullptr);
	m_pDeviceContext->ClearRenderTargetView(m_pBackBufferRTV.Get(), clearBB);

	// Transform light to view-space
	Vector3 lightPosVS = Vector3::Transform(m_LightPosWorld, m_View);

	CBLight cbLight;
	cbLight.LightPosVS_Radius = Vector4(lightPosVS.x, lightPosVS.y, lightPosVS.z, m_LightRadius);
	cbLight.LightColor = Vector4(m_LightColor.x, m_LightColor.y, m_LightColor.z, 0.0f);
	cbLight.Ambient = Vector4(0.06f, 0.06f, 0.06f, 0.0f);
	m_pDeviceContext->UpdateSubresource(m_pCBLight.Get(), 0, nullptr, &cbLight, 0, 0);

	ID3D11ShaderResourceView* srvs[GBufferCount] = { m_pGBufferSRVs[0].Get(), m_pGBufferSRVs[1].Get(), m_pGBufferSRVs[2].Get() };

	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pDeviceContext->IASetVertexBuffers(0, 1, m_pQuadVB.GetAddressOf(), &m_QuadVBStride, &m_QuadVBOffset);
	m_pDeviceContext->IASetIndexBuffer(m_pQuadIB.Get(), DXGI_FORMAT_R16_UINT, 0);
	m_pDeviceContext->IASetInputLayout(m_pQuadInputLayout.Get());

	m_pDeviceContext->VSSetShader(m_pQuadVS.Get(), nullptr, 0);
	m_pDeviceContext->PSSetShader(m_pLightPS.Get(), nullptr, 0);
	m_pDeviceContext->PSSetConstantBuffers(0, 1, m_pCBLight.GetAddressOf());

	m_pDeviceContext->PSSetShaderResources(0, GBufferCount, srvs);
	m_pDeviceContext->PSSetSamplers(0, 1, m_pSamplerLinear.GetAddressOf());

	m_pDeviceContext->DrawIndexed(m_QuadIndexCount, 0, 0);

	// Unbind SRVs
	ID3D11ShaderResourceView* nullSRVs[GBufferCount] = { nullptr, nullptr, nullptr };
	m_pDeviceContext->PSSetShaderResources(0, GBufferCount, nullSRVs);

	// ImGui Rendering
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// ImGui Window - Settings
    ImGui::Begin("Deferred Rendering Settings");
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::Checkbox("Deferred Rendering", (bool*)&m_UseDeferredRendering);
    ImGui::Separator();

	ImGui::Text("Camera Info");
	ImGui::Text("Position: (%.2f, %.2f, %.2f)", m_Camera.m_Position.x, m_Camera.m_Position.y, m_Camera.m_Position.z);
	Vector3 forward = m_Camera.GetForward();
	ImGui::Text("Forward: (%.2f, %.2f, %.2f)", forward.x, forward.y, forward.z);
	ImGui::Separator();

	ImGui::Text("Light Settings");	
    ImGui::Text("Light Position: (%.2f, %.2f, %.2f)", m_LightPosWorld.x, m_LightPosWorld.y, m_LightPosWorld.z);   
	ImGui::ColorEdit3("Light Color", (float*)&m_LightColor);
	ImGui::SliderFloat("Light Radius", &m_LightRadius, 1.0f, 20.0f);	
	ImGui::Separator();
	
	ImGui::End();

	// ImGui Window - G-Buffer Debug View
	ImGui::Begin("G-Buffer Debug View");

	// 첫 번째 줄: Color, Normal
	ImGui::BeginGroup();
	ImGui::Text("Color (Albedo)");
	ImGui::Image((ImTextureID)m_pGBufferSRVs[0].Get(), ImVec2(128, 128));
	ImGui::EndGroup();

	ImGui::SameLine();

	ImGui::BeginGroup();
	ImGui::Text("Normal");
	ImGui::Image((ImTextureID)m_pGBufferSRVs[1].Get(), ImVec2(128, 128));
	ImGui::EndGroup();

	// 두 번째 줄: Position, Depth
	ImGui::BeginGroup();
	ImGui::Text("Position");
	ImGui::Image((ImTextureID)m_pGBufferSRVs[2].Get(), ImVec2(128, 128));
	ImGui::EndGroup();

	ImGui::SameLine();

	ImGui::BeginGroup();
	ImGui::Text("Depth Buffer");
	ImGui::Image((ImTextureID)m_pDepthSRV.Get(), ImVec2(128, 128));
	ImGui::EndGroup();

	ImGui::End();

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	HR_T(m_pSwapChain->Present(0, 0));
}

bool TutorialApp::InitD3D()
{
    if (!CreateSwapChainAndBackBuffer())
        return false;

    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = static_cast<float>(m_ClientWidth);
    viewport.Height = static_cast<float>(m_ClientHeight);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    m_pDeviceContext->RSSetViewports(1, &viewport);

    if (!CreateDepthBuffer())
        return false;

    if (!CreateGBuffer())
        return false;

    return true;
}

void TutorialApp::UninitD3D()
{
 
}

bool TutorialApp::InitScene()
{
    if (!CreateCube())
        return false;

    if (!CreateQuad())
        return false;

    if (!CreateShaders())
        return false;

    if (!CreateStates())
        return false;

    // Constant buffers
    {
        D3D11_BUFFER_DESC bd = {};
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

        bd.ByteWidth = sizeof(CBGeometry);
        HR_T(m_pDevice->CreateBuffer(&bd, nullptr, m_pCBGeometry.GetAddressOf()));

        bd.ByteWidth = sizeof(CBLight);
        HR_T(m_pDevice->CreateBuffer(&bd, nullptr, m_pCBLight.GetAddressOf()));
    }

    // Scene matrices
    m_World = Matrix::Identity;

    // Put camera a bit back (camera itself is driven by GameApp input too)
    m_Camera.Reset();
    m_Camera.m_Position = Vector3(0.0f, 2.0f, -50.0f);
    m_Camera.m_Rotation = Vector3(0.0f, 0.0f, 0.0f);
    m_Camera.Update(0.0f);
    m_Camera.GetViewMatrix(m_View);

    	
	m_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, m_ClientWidth / (FLOAT)m_ClientHeight, 1.0f, 10000.0f);

    return true;
}

void TutorialApp::UninitScene()
{
  
}

bool TutorialApp::CreateSwapChainAndBackBuffer()
{
    DXGI_SWAP_CHAIN_DESC swapDesc = {};
    swapDesc.BufferCount = 2;
    swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
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

    HR_T(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, creationFlags,
        nullptr, 0, D3D11_SDK_VERSION, &swapDesc, m_pSwapChain.GetAddressOf(), m_pDevice.GetAddressOf(), nullptr, m_pDeviceContext.GetAddressOf()));

    ComPtr<ID3D11Texture2D> backBuffer;
    HR_T(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)backBuffer.GetAddressOf()));
    HR_T(m_pDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, m_pBackBufferRTV.GetAddressOf()));

    return true;
}

bool TutorialApp::CreateDepthBuffer()
{
    D3D11_TEXTURE2D_DESC descDepth = {};
    descDepth.Width = m_ClientWidth;
    descDepth.Height = m_ClientHeight;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_R24G8_TYPELESS; // Typeless로 변경하여 SRV 생성 가능하게
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE; // SRV 플래그 추가

    HR_T(m_pDevice->CreateTexture2D(&descDepth, nullptr, m_pDepthTexture.GetAddressOf()));

    // Depth Stencil View 생성
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;
    HR_T(m_pDevice->CreateDepthStencilView(m_pDepthTexture.Get(), &dsvDesc, m_pDepthDSV.GetAddressOf()));

    // Shader Resource View 생성 (Depth를 텐스처로 읽기 위해)
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.MostDetailedMip = 0;
    HR_T(m_pDevice->CreateShaderResourceView(m_pDepthTexture.Get(), &srvDesc, m_pDepthSRV.GetAddressOf()));

    return true;
}

bool TutorialApp::CreateGBuffer()
{
    ReleaseGBuffer();

    struct RTDesc
    {
        DXGI_FORMAT format;
    };

    RTDesc formats[GBufferCount] = {
        { DXGI_FORMAT_R8G8B8A8_UNORM_SRGB },   // Color (Albedo) - 4바이트
        { DXGI_FORMAT_R8G8B8A8_UNORM },     // Normal - 4바이트 (단위벡터이므로 8비트 충분)
        { DXGI_FORMAT_R16G16B16A16_FLOAT }, // PositionVS - 8바이트
    };

    for (int i = 0; i < GBufferCount; ++i)
    {
        D3D11_TEXTURE2D_DESC td = {};
        td.Width = m_ClientWidth;
        td.Height = m_ClientHeight;
        td.MipLevels = 1;
        td.ArraySize = 1;
        td.Format = formats[i].format;
        td.SampleDesc.Count = 1;
        td.Usage = D3D11_USAGE_DEFAULT;
        td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

        HR_T(m_pDevice->CreateTexture2D(&td, nullptr, m_pGBufferTextures[i].GetAddressOf()));
        HR_T(m_pDevice->CreateRenderTargetView(m_pGBufferTextures[i].Get(), nullptr, m_pGBufferRTVs[i].GetAddressOf()));
        HR_T(m_pDevice->CreateShaderResourceView(m_pGBufferTextures[i].Get(), nullptr, m_pGBufferSRVs[i].GetAddressOf()));
    }

    return true;
}

void TutorialApp::ReleaseGBuffer()
{
    for (int i = 0; i < GBufferCount; ++i)
    {
        m_pGBufferSRVs[i].Reset();
        m_pGBufferRTVs[i].Reset();
        m_pGBufferTextures[i].Reset();
    }
}

bool TutorialApp::CreateCube()
{
    struct CubeVertex
    {
        Vector3 Pos;
        Vector3 Normal;
    };

    CubeVertex vertices[] =
    {
        { Vector3(-1.0f, 1.0f, -1.0f), Vector3(0.0f, 1.0f, 0.0f) },
        { Vector3(1.0f, 1.0f, -1.0f),  Vector3(0.0f, 1.0f, 0.0f) },
        { Vector3(1.0f, 1.0f, 1.0f),   Vector3(0.0f, 1.0f, 0.0f) },
        { Vector3(-1.0f, 1.0f, 1.0f),  Vector3(0.0f, 1.0f, 0.0f) },

        { Vector3(-1.0f, -1.0f, -1.0f), Vector3(0.0f, -1.0f, 0.0f) },
        { Vector3(1.0f, -1.0f, -1.0f),  Vector3(0.0f, -1.0f, 0.0f) },
        { Vector3(1.0f, -1.0f, 1.0f),   Vector3(0.0f, -1.0f, 0.0f) },
        { Vector3(-1.0f, -1.0f, 1.0f),  Vector3(0.0f, -1.0f, 0.0f) },

        { Vector3(-1.0f, -1.0f, 1.0f),  Vector3(-1.0f, 0.0f, 0.0f) },
        { Vector3(-1.0f, -1.0f, -1.0f), Vector3(-1.0f, 0.0f, 0.0f) },
        { Vector3(-1.0f, 1.0f, -1.0f),  Vector3(-1.0f, 0.0f, 0.0f) },
        { Vector3(-1.0f, 1.0f, 1.0f),   Vector3(-1.0f, 0.0f, 0.0f) },

        { Vector3(1.0f, -1.0f, 1.0f),   Vector3(1.0f, 0.0f, 0.0f) },
        { Vector3(1.0f, -1.0f, -1.0f),  Vector3(1.0f, 0.0f, 0.0f) },
        { Vector3(1.0f, 1.0f, -1.0f),   Vector3(1.0f, 0.0f, 0.0f) },
        { Vector3(1.0f, 1.0f, 1.0f),    Vector3(1.0f, 0.0f, 0.0f) },

        { Vector3(-1.0f, -1.0f, -1.0f), Vector3(0.0f, 0.0f, -1.0f) },
        { Vector3(1.0f, -1.0f, -1.0f),  Vector3(0.0f, 0.0f, -1.0f) },
        { Vector3(1.0f, 1.0f, -1.0f),   Vector3(0.0f, 0.0f, -1.0f) },
        { Vector3(-1.0f, 1.0f, -1.0f),  Vector3(0.0f, 0.0f, -1.0f) },

        { Vector3(-1.0f, -1.0f, 1.0f),  Vector3(0.0f, 0.0f, 1.0f) },
        { Vector3(1.0f, -1.0f, 1.0f),   Vector3(0.0f, 0.0f, 1.0f) },
        { Vector3(1.0f, 1.0f, 1.0f),    Vector3(0.0f, 0.0f, 1.0f) },
        { Vector3(-1.0f, 1.0f, 1.0f),   Vector3(0.0f, 0.0f, 1.0f) },
    };

    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.ByteWidth = sizeof(vertices);
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    D3D11_SUBRESOURCE_DATA vbData = {};
    vbData.pSysMem = vertices;
    HR_T(m_pDevice->CreateBuffer(&vbDesc, &vbData, m_pCubeVB.GetAddressOf()));
    m_CubeVBStride = sizeof(CubeVertex);
    m_CubeVBOffset = 0;

    WORD indices[] =
    {
        3,1,0, 2,1,3,
        6,4,5, 7,4,6,
        11,9,8, 10,9,11,
        14,12,13, 15,12,14,
        19,17,16, 18,17,19,
        22,20,21, 23,20,22
    };

    m_CubeIndexCount = ARRAYSIZE(indices);

    D3D11_BUFFER_DESC ibDesc = {};
    ibDesc.ByteWidth = sizeof(indices);
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibDesc.Usage = D3D11_USAGE_DEFAULT;
    D3D11_SUBRESOURCE_DATA ibData = {};
    ibData.pSysMem = indices;
    HR_T(m_pDevice->CreateBuffer(&ibDesc, &ibData, m_pCubeIB.GetAddressOf()));

    return true;
}

bool TutorialApp::CreateQuad()
{
    struct QuadVertex
    {
        Vector3 position;
        Vector2 uv;
    };

    QuadVertex vertices[] =
    {
        { Vector3(-1.0f,  1.0f, 0.0f), Vector2(0.0f, 0.0f) },
        { Vector3(1.0f,   1.0f, 0.0f), Vector2(1.0f, 0.0f) },
        { Vector3(-1.0f, -1.0f, 0.0f), Vector2(0.0f, 1.0f) },
        { Vector3(1.0f,  -1.0f, 0.0f), Vector2(1.0f, 1.0f) },
    };

    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.ByteWidth = sizeof(vertices);
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    D3D11_SUBRESOURCE_DATA vbData = {};
    vbData.pSysMem = vertices;
    HR_T(m_pDevice->CreateBuffer(&vbDesc, &vbData, m_pQuadVB.GetAddressOf()));
    m_QuadVBStride = sizeof(QuadVertex);
    m_QuadVBOffset = 0;

    WORD indices[] = { 0, 1, 2, 2, 1, 3 };
    m_QuadIndexCount = ARRAYSIZE(indices);

    D3D11_BUFFER_DESC ibDesc = {};
    ibDesc.ByteWidth = sizeof(indices);
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibDesc.Usage = D3D11_USAGE_DEFAULT;
    D3D11_SUBRESOURCE_DATA ibData = {};
    ibData.pSysMem = indices;
    HR_T(m_pDevice->CreateBuffer(&ibDesc, &ibData, m_pQuadIB.GetAddressOf()));

    return true;
}

bool TutorialApp::CreateShaders()
{
    // Geometry pass
    {
        ComPtr<ID3DBlob> vsBlob;
        HR_T(CompileShaderFromFile(L"../Shaders/15_GBufferVS.hlsl", "main", "vs_4_0", vsBlob.GetAddressOf()));

        D3D11_INPUT_ELEMENT_DESC layout[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        HR_T(m_pDevice->CreateInputLayout(layout, ARRAYSIZE(layout), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), m_pCubeInputLayout.GetAddressOf()));
        HR_T(m_pDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, m_pGBufferVS.GetAddressOf()));

        ComPtr<ID3DBlob> psBlob;
        HR_T(CompileShaderFromFile(L"../Shaders/15_GBufferPS.hlsl", "main", "ps_4_0", psBlob.GetAddressOf()));
        HR_T(m_pDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, m_pGBufferPS.GetAddressOf()));
    }

    // Light pass
    {
        ComPtr<ID3DBlob> vsBlob;
        HR_T(CompileShaderFromFile(L"../Shaders/15_QuadVS.hlsl", "main", "vs_4_0", vsBlob.GetAddressOf()));

        D3D11_INPUT_ELEMENT_DESC layout[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        HR_T(m_pDevice->CreateInputLayout(layout, ARRAYSIZE(layout), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), m_pQuadInputLayout.GetAddressOf()));
        HR_T(m_pDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, m_pQuadVS.GetAddressOf()));

        ComPtr<ID3DBlob> psBlob;
        HR_T(CompileShaderFromFile(L"../Shaders/15_LightPassPS.hlsl", "main", "ps_4_0", psBlob.GetAddressOf()));
        HR_T(m_pDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, m_pLightPS.GetAddressOf()));
    }

    return true;
}

bool TutorialApp::CreateStates()
{
    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    HR_T(m_pDevice->CreateSamplerState(&sampDesc, m_pSamplerLinear.GetAddressOf()));

    return true;
}

bool TutorialApp::InitImGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(m_hWnd);
    ImGui_ImplDX11_Init(m_pDevice.Get(), m_pDeviceContext.Get());

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
