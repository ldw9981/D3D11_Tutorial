#pragma once

#include <windows.h>
#include "../Common/GameApp.h"

#include <d3d11.h>
#include <directxtk/SimpleMath.h>

using namespace DirectX;
using namespace DirectX::SimpleMath;

class TutorialApp : public GameApp
{
public:
    // D3D11 core
    ID3D11Device* m_pDevice = nullptr;
    ID3D11DeviceContext* m_pDeviceContext = nullptr;
    IDXGISwapChain* m_pSwapChain = nullptr;
    ID3D11RenderTargetView* m_pBackBufferRTV = nullptr;

    ID3D11Texture2D* m_pDepthTexture = nullptr;
    ID3D11DepthStencilView* m_pDepthDSV = nullptr;

    // G-Buffer (Albedo, Normal, PositionVS)
    static constexpr int GBufferCount = 3;
    ID3D11Texture2D* m_pGBufferTextures[GBufferCount] = {};
    ID3D11RenderTargetView* m_pGBufferRTVs[GBufferCount] = {};
    ID3D11ShaderResourceView* m_pGBufferSRVs[GBufferCount] = {};

    // Geometry pass (cube)
    ID3D11VertexShader* m_pGBufferVS = nullptr;
    ID3D11PixelShader* m_pGBufferPS = nullptr;
    ID3D11InputLayout* m_pCubeInputLayout = nullptr;
    ID3D11Buffer* m_pCubeVB = nullptr;
    ID3D11Buffer* m_pCubeIB = nullptr;
    UINT m_CubeVBStride = 0;
    UINT m_CubeVBOffset = 0;
    int m_CubeIndexCount = 0;

    // Light pass (fullscreen quad)
    ID3D11VertexShader* m_pQuadVS = nullptr;
    ID3D11PixelShader* m_pLightPS = nullptr;
    ID3D11InputLayout* m_pQuadInputLayout = nullptr;
    ID3D11Buffer* m_pQuadVB = nullptr;
    ID3D11Buffer* m_pQuadIB = nullptr;
    UINT m_QuadVBStride = 0;
    UINT m_QuadVBOffset = 0;
    int m_QuadIndexCount = 0;

    ID3D11SamplerState* m_pSamplerLinear = nullptr;

    // Constant buffers
    ID3D11Buffer* m_pCBGeometry = nullptr;
    ID3D11Buffer* m_pCBLight = nullptr;

    // Scene data
    Matrix m_World = Matrix::Identity;
    Matrix m_View = Matrix::Identity;
    Matrix m_Projection = Matrix::Identity;

    Vector3 m_LightPosWorld = Vector3(2.0f, 2.0f, -2.0f);
    Vector3 m_LightColor = Vector3(1.0f, 1.0f, 1.0f);
    float m_LightRadius = 6.0f;
    float m_Exposure = 1.0f;

    bool OnInitialize() override;
    void OnUninitialize();
    void OnUpdate();
    void OnRender() override;

private:
    bool InitD3D();
    void UninitD3D();
    bool InitScene();
    void UninitScene();

    bool CreateSwapChainAndBackBuffer();
    bool CreateDepthBuffer();
    bool CreateGBuffer();

    bool CreateCube();
    bool CreateQuad();
    bool CreateShaders();
    bool CreateStates();

    void ReleaseGBuffer();
};
