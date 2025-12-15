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
        Vector4 Albedo;
    };

    struct CBLight
    {
        Vector4 LightPosVS_Radius; // xyz = positionVS, w = radius
        Vector4 LightColor_Exposure; // rgb = light color, w = exposure
        Vector4 Ambient; // xyz ambient, w unused
    };
}

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
    m_Camera.GetViewMatrix(m_View);

    // Simple light animation
    static float t = 0.0f;
    t += m_Timer.DeltaTime();
    m_LightPosWorld.x = 2.0f * cosf(t);
    m_LightPosWorld.z = -2.0f + 2.0f * sinf(t);
}

void TutorialApp::OnRender()
{
    // 1) Geometry pass -> GBuffer MRTs
    float clearAlbedo[4] = { 0.02f, 0.02f, 0.02f, 1.0f };
    float clearNormal[4] = { 0.5f, 0.5f, 1.0f, 1.0f };
    float clearPos[4] = { 0, 0, 0, 1 };

    m_pDeviceContext->ClearRenderTargetView(m_pGBufferRTVs[0], clearAlbedo);
    m_pDeviceContext->ClearRenderTargetView(m_pGBufferRTVs[1], clearNormal);
    m_pDeviceContext->ClearRenderTargetView(m_pGBufferRTVs[2], clearPos);
    m_pDeviceContext->ClearDepthStencilView(m_pDepthDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

    m_pDeviceContext->OMSetRenderTargets(GBufferCount, m_pGBufferRTVs, m_pDepthDSV);

    CBGeometry cbGeom;
    cbGeom.World = m_World.Transpose();
    cbGeom.View = m_View.Transpose();
    cbGeom.Projection = m_Projection.Transpose();
    cbGeom.Albedo = Vector4(0.8f, 0.2f, 0.2f, 1.0f);
    m_pDeviceContext->UpdateSubresource(m_pCBGeometry, 0, nullptr, &cbGeom, 0, 0);

    m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_pDeviceContext->IASetVertexBuffers(0, 1, &m_pCubeVB, &m_CubeVBStride, &m_CubeVBOffset);
    m_pDeviceContext->IASetIndexBuffer(m_pCubeIB, DXGI_FORMAT_R16_UINT, 0);
    m_pDeviceContext->IASetInputLayout(m_pCubeInputLayout);

    m_pDeviceContext->VSSetShader(m_pGBufferVS, nullptr, 0);
    m_pDeviceContext->VSSetConstantBuffers(0, 1, &m_pCBGeometry);
    m_pDeviceContext->PSSetShader(m_pGBufferPS, nullptr, 0);
    m_pDeviceContext->PSSetConstantBuffers(0, 1, &m_pCBGeometry);

    m_pDeviceContext->DrawIndexed(m_CubeIndexCount, 0, 0);

    // 2) Light pass -> back buffer
    float clearBB[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    m_pDeviceContext->OMSetRenderTargets(1, &m_pBackBufferRTV, nullptr);
    m_pDeviceContext->ClearRenderTargetView(m_pBackBufferRTV, clearBB);

    // Transform light to view-space
    Vector3 lightPosVS = Vector3::Transform(m_LightPosWorld, m_View);

    CBLight cbLight;
    cbLight.LightPosVS_Radius = Vector4(lightPosVS.x, lightPosVS.y, lightPosVS.z, m_LightRadius);
    cbLight.LightColor_Exposure = Vector4(m_LightColor.x, m_LightColor.y, m_LightColor.z, m_Exposure);
    cbLight.Ambient = Vector4(0.06f, 0.06f, 0.06f, 0.0f);
    m_pDeviceContext->UpdateSubresource(m_pCBLight, 0, nullptr, &cbLight, 0, 0);

    ID3D11ShaderResourceView* srvs[GBufferCount] = { m_pGBufferSRVs[0], m_pGBufferSRVs[1], m_pGBufferSRVs[2] };

    m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_pDeviceContext->IASetVertexBuffers(0, 1, &m_pQuadVB, &m_QuadVBStride, &m_QuadVBOffset);
    m_pDeviceContext->IASetIndexBuffer(m_pQuadIB, DXGI_FORMAT_R16_UINT, 0);
    m_pDeviceContext->IASetInputLayout(m_pQuadInputLayout);

    m_pDeviceContext->VSSetShader(m_pQuadVS, nullptr, 0);
    m_pDeviceContext->PSSetShader(m_pLightPS, nullptr, 0);
    m_pDeviceContext->PSSetConstantBuffers(0, 1, &m_pCBLight);

    m_pDeviceContext->PSSetShaderResources(0, GBufferCount, srvs);
    m_pDeviceContext->PSSetSamplers(0, 1, &m_pSamplerLinear);

    m_pDeviceContext->DrawIndexed(m_QuadIndexCount, 0, 0);

    // Unbind SRVs
    ID3D11ShaderResourceView* nullSRVs[GBufferCount] = { nullptr, nullptr, nullptr };
    m_pDeviceContext->PSSetShaderResources(0, GBufferCount, nullSRVs);

    m_pSwapChain->Present(0, 0);
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
    ReleaseGBuffer();

    SAFE_RELEASE(m_pDepthDSV);
    SAFE_RELEASE(m_pDepthTexture);

    SAFE_RELEASE(m_pBackBufferRTV);

    SAFE_RELEASE(m_pSwapChain);
    SAFE_RELEASE(m_pDeviceContext);
    SAFE_RELEASE(m_pDevice);
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
        HR_T(m_pDevice->CreateBuffer(&bd, nullptr, &m_pCBGeometry));

        bd.ByteWidth = sizeof(CBLight);
        HR_T(m_pDevice->CreateBuffer(&bd, nullptr, &m_pCBLight));
    }

    // Scene matrices
    m_World = Matrix::Identity;

    // Put camera a bit back (camera itself is driven by GameApp input too)
    m_Camera.Reset();
    m_Camera.m_Position = Vector3(0.0f, 2.0f, -6.0f);
    m_Camera.m_Rotation = Vector3(0.0f, 0.0f, 0.0f);
    m_Camera.Update(0.0f);
    m_Camera.GetViewMatrix(m_View);

    m_Projection = Matrix::CreatePerspectiveFieldOfView(XM_PIDIV4, m_ClientWidth / (FLOAT)m_ClientHeight, 0.01f, 100.0f);

    return true;
}

void TutorialApp::UninitScene()
{
    SAFE_RELEASE(m_pCBLight);
    SAFE_RELEASE(m_pCBGeometry);

    SAFE_RELEASE(m_pSamplerLinear);

    SAFE_RELEASE(m_pQuadIB);
    SAFE_RELEASE(m_pQuadVB);
    SAFE_RELEASE(m_pQuadInputLayout);
    SAFE_RELEASE(m_pLightPS);
    SAFE_RELEASE(m_pQuadVS);

    SAFE_RELEASE(m_pCubeIB);
    SAFE_RELEASE(m_pCubeVB);
    SAFE_RELEASE(m_pCubeInputLayout);
    SAFE_RELEASE(m_pGBufferPS);
    SAFE_RELEASE(m_pGBufferVS);
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
        nullptr, 0, D3D11_SDK_VERSION, &swapDesc, &m_pSwapChain, &m_pDevice, nullptr, &m_pDeviceContext));

    ID3D11Texture2D* backBuffer = nullptr;
    HR_T(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer));
    HR_T(m_pDevice->CreateRenderTargetView(backBuffer, nullptr, &m_pBackBufferRTV));
    SAFE_RELEASE(backBuffer);

    return true;
}

bool TutorialApp::CreateDepthBuffer()
{
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

    HR_T(m_pDevice->CreateTexture2D(&descDepth, nullptr, &m_pDepthTexture));
    HR_T(m_pDevice->CreateDepthStencilView(m_pDepthTexture, nullptr, &m_pDepthDSV));

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
        { DXGI_FORMAT_R8G8B8A8_UNORM },
        { DXGI_FORMAT_R16G16B16A16_FLOAT },
        { DXGI_FORMAT_R16G16B16A16_FLOAT },
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

        HR_T(m_pDevice->CreateTexture2D(&td, nullptr, &m_pGBufferTextures[i]));
        HR_T(m_pDevice->CreateRenderTargetView(m_pGBufferTextures[i], nullptr, &m_pGBufferRTVs[i]));
        HR_T(m_pDevice->CreateShaderResourceView(m_pGBufferTextures[i], nullptr, &m_pGBufferSRVs[i]));
    }

    return true;
}

void TutorialApp::ReleaseGBuffer()
{
    for (int i = 0; i < GBufferCount; ++i)
    {
        SAFE_RELEASE(m_pGBufferSRVs[i]);
        SAFE_RELEASE(m_pGBufferRTVs[i]);
        SAFE_RELEASE(m_pGBufferTextures[i]);
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
    HR_T(m_pDevice->CreateBuffer(&vbDesc, &vbData, &m_pCubeVB));
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
    HR_T(m_pDevice->CreateBuffer(&ibDesc, &ibData, &m_pCubeIB));

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
    HR_T(m_pDevice->CreateBuffer(&vbDesc, &vbData, &m_pQuadVB));
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
    HR_T(m_pDevice->CreateBuffer(&ibDesc, &ibData, &m_pQuadIB));

    return true;
}

bool TutorialApp::CreateShaders()
{
    // Geometry pass
    {
        ID3DBlob* vsBlob = nullptr;
        HR_T(CompileShaderFromFile(L"../Shaders/15_GBufferVS.hlsl", "main", "vs_4_0", &vsBlob));

        D3D11_INPUT_ELEMENT_DESC layout[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        HR_T(m_pDevice->CreateInputLayout(layout, ARRAYSIZE(layout), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_pCubeInputLayout));
        HR_T(m_pDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_pGBufferVS));
        SAFE_RELEASE(vsBlob);

        ID3DBlob* psBlob = nullptr;
        HR_T(CompileShaderFromFile(L"../Shaders/15_GBufferPS.hlsl", "main", "ps_4_0", &psBlob));
        HR_T(m_pDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_pGBufferPS));
        SAFE_RELEASE(psBlob);
    }

    // Light pass
    {
        ID3DBlob* vsBlob = nullptr;
        HR_T(CompileShaderFromFile(L"../Shaders/15_QuadVS.hlsl", "main", "vs_4_0", &vsBlob));

        D3D11_INPUT_ELEMENT_DESC layout[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        HR_T(m_pDevice->CreateInputLayout(layout, ARRAYSIZE(layout), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_pQuadInputLayout));
        HR_T(m_pDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_pQuadVS));
        SAFE_RELEASE(vsBlob);

        ID3DBlob* psBlob = nullptr;
        HR_T(CompileShaderFromFile(L"../Shaders/15_LightPassPS.hlsl", "main", "ps_4_0", &psBlob));
        HR_T(m_pDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_pLightPS));
        SAFE_RELEASE(psBlob);
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
    HR_T(m_pDevice->CreateSamplerState(&sampDesc, &m_pSamplerLinear));

    return true;
}
