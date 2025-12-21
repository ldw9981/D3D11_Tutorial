#pragma once

#include <windows.h>
#include "../Common/GameApp.h"

#include <d3d11.h>
#include <wrl/client.h>
#include <directxtk/SimpleMath.h>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

using namespace DirectX;
using namespace DirectX::SimpleMath;
using namespace Microsoft::WRL;

enum GBUFFER_TEXTURES
{
	GBUFFER_TEXTURE_COLOR = 0,
	GBUFFER_TEXTURE_NORMAL = 1,
	GBUFFER_TEXTURE_POSITIONVS = 2,
};

class TutorialApp : public GameApp
{
public:
	// D3D11 core
	ComPtr<ID3D11Device> m_pDevice = nullptr;
	ComPtr<ID3D11DeviceContext> m_pDeviceContext = nullptr;
	ComPtr<IDXGISwapChain> m_pSwapChain = nullptr;
	ComPtr<ID3D11RenderTargetView> m_pBackBufferRTV = nullptr;

	ComPtr<ID3D11Texture2D> m_pDepthTexture = nullptr;
	ComPtr<ID3D11DepthStencilView> m_pDepthDSV = nullptr;
	ComPtr<ID3D11ShaderResourceView> m_pDepthSRV = nullptr;

	// G-Buffer (Color, Normal, PositionVS)
	static constexpr int GBufferCount = 3;
	ComPtr<ID3D11Texture2D> m_pGBufferTextures[GBufferCount] = {};
	ComPtr<ID3D11RenderTargetView> m_pGBufferRTVs[GBufferCount] = {};
	ComPtr<ID3D11ShaderResourceView> m_pGBufferSRVs[GBufferCount] = {};

	// Geometry pass (cube)
	ComPtr<ID3D11VertexShader> m_pGBufferVS = nullptr;
	ComPtr<ID3D11PixelShader> m_pGBufferPS = nullptr;
	ComPtr<ID3D11InputLayout> m_pCubeInputLayout = nullptr;
	ComPtr<ID3D11Buffer> m_pCubeVB = nullptr;
	ComPtr<ID3D11Buffer> m_pCubeIB = nullptr;
	UINT m_CubeVBStride = 0;
	UINT m_CubeVBOffset = 0;
	int m_CubeIndexCount = 0;

	// Light pass (fullscreen quad)
	ComPtr<ID3D11VertexShader> m_pDirectionLightVS = nullptr;
	ComPtr<ID3D11VertexShader> m_pPointLightVS = nullptr;
	ComPtr<ID3D11PixelShader> m_pPointLightPS = nullptr;
	ComPtr<ID3D11PixelShader> m_pDirectionLightPS = nullptr;
	ComPtr<ID3D11PixelShader> m_pSolidPS = nullptr;
	ComPtr<ID3D11InputLayout> m_pQuadInputLayout = nullptr;
	ComPtr<ID3D11Buffer> m_pQuadVB = nullptr;
	ComPtr<ID3D11Buffer> m_pQuadIB = nullptr;
	UINT m_QuadVBStride = 0;
	UINT m_QuadVBOffset = 0;
	int m_QuadIndexCount = 0;

	// Sphere for light volume
	ComPtr<ID3D11Buffer> m_pSphereVB = nullptr;
	ComPtr<ID3D11Buffer> m_pSphereIB = nullptr;
	UINT m_SphereVBStride = 0;
	UINT m_SphereVBOffset = 0;
	int m_SphereIndexCount = 0;

	ComPtr<ID3D11SamplerState> m_pSamplerLinear = nullptr;

	// Blend State
	ComPtr<ID3D11BlendState> m_pBlendStateAdditive = nullptr;

	// Depth Stencil State (for light volumes)
	ComPtr<ID3D11DepthStencilState> m_pDSStateLightVolume = nullptr;
	ComPtr<ID3D11DepthStencilState> m_pDSStateGBuffer;

	// Constant buffers
	ComPtr<ID3D11Buffer> m_pCBGeometry = nullptr;
	ComPtr<ID3D11Buffer> m_pCBPointLight = nullptr;
	ComPtr<ID3D11Buffer> m_pCBDirectionalLight = nullptr;
	ComPtr<ID3D11Buffer> m_pCBScreenSize = nullptr;

	// Scene data
	Matrix m_World = Matrix::Identity;
	Matrix m_View = Matrix::Identity;
	Matrix m_Projection = Matrix::Identity;

	// Point Lights
	static const int MAX_POINT_LIGHTS = 100;
	struct PointLightData
	{
		Vector3 position;
		Vector3 color;
		float radius;
	};
	PointLightData m_PointLights[MAX_POINT_LIGHTS];
	int m_ActiveLightCount = 50;  // Number of lights to actually render (adjustable)
	float m_GlobalLightRadiusScale = 1.0f;


	// Directional Light
	Vector3 m_DirLightDirection = Vector3(1.0f, -1.0f, 0.5f);
	Vector3 m_DirLightColor = Vector3(0.1f, 0.1f, 0.1f);
	float m_DirLightIntensity = 1.0f;

	bool m_UseDeferredRendering = true;

	bool OnInitialize() override;
	void OnUninitialize();
	void OnUpdate() override;
	void OnRender() override;

	void RenderPassGUI();
	void RenderPassGBuffer();
	void RenderPassLight();
	void RenderPassLightPosition();

	virtual LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) override;

private:
	bool InitD3D();
	void UninitD3D();
	bool InitScene();
	void UninitScene();

	bool InitImGui();
	void UninitImGui();

	bool CreateSwapChainAndBackBuffer();
	bool CreateDepthBuffer();
	bool CreateGBuffer();

	bool CreateCube();
	bool CreateQuad();
	bool CreateSphere();
	bool CreateShaders();
	bool CreateStates();

	void ReleaseGBuffer();
};
