#pragma once
#include <windows.h>
#include "../Common/GameApp.h"
#include <d3d11.h>
#include <directxtk/SimpleMath.h>
#include <wrl/client.h>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

using Microsoft::WRL::ComPtr;
using namespace DirectX::SimpleMath;
using namespace DirectX;

class TutorialApp :
    public GameApp
{
public:
	// Direct3D 오브젝트를 생성하는 필수 객체와 인터페이스
	ComPtr<ID3D11Device> m_pDevice;
	ComPtr<ID3D11DeviceContext> m_pDeviceContext;
	ComPtr<IDXGISwapChain> m_pSwapChain;
	ComPtr<ID3D11RenderTargetView> m_pRenderTargetView;
	ComPtr<ID3D11DepthStencilView> m_pDepthStencilView;

	// Anti-Aliasing Mode
	enum AAMode
	{
		AA_NONE = 0,
		AA_MSAA = 1,
		AA_FXAA = 2
	};

	// MSAA Resources
	ComPtr<ID3D11Texture2D> m_pMSAATexture;
	ComPtr<ID3D11RenderTargetView> m_pMSAARenderTargetView;
	ComPtr<ID3D11Texture2D> m_pMSAADepthStencil;
	ComPtr<ID3D11DepthStencilView> m_pMSAADepthStencilView;

	// FXAA Resources
	ComPtr<ID3D11Texture2D> m_pSceneTexture;
	ComPtr<ID3D11RenderTargetView> m_pSceneRenderTargetView;
	ComPtr<ID3D11ShaderResourceView> m_pSceneShaderResourceView;
	ComPtr<ID3D11Buffer> m_pFullscreenQuadVB;
	ComPtr<ID3D11VertexShader> m_pFullscreenVS;
	ComPtr<ID3D11PixelShader> m_pFXAAPS;
	ComPtr<ID3D11InputLayout> m_pFullscreenInputLayout;
	ComPtr<ID3D11SamplerState> m_pLinearSampler;

	// 그리기에 파이프라인에 적용하는 리소스 객체와 인터페이스
	ComPtr<ID3D11Buffer> m_pVertexBuffer;
	ComPtr<ID3D11Buffer> m_pIndexBuffer;
	ComPtr<ID3D11VertexShader> m_pVertexShader;
	ComPtr<ID3D11PixelShader> m_pPixelShader;
	ComPtr<ID3D11InputLayout> m_pInputLayout;
	ComPtr<ID3D11Buffer> m_pCBNeverChanges;
	ComPtr<ID3D11Buffer> m_pCBChangeOnResize;
	ComPtr<ID3D11Buffer> m_pCBChangesEveryFrame;

	// Anti-Aliasing Options
	int m_AAMode = AA_NONE; // 0=None, 1=MSAA, 2=FXAA
	int m_MSAASampleCount = 4; // 1, 2, 4, 8

	// 그리기에 파이프라인에 적용하는 정보
	UINT m_VertexBufferStride = 0;
	UINT m_VertexBufferOffset = 0;
	int m_nIndexCount = 0;
	Matrix m_World1;
	Matrix m_World2;
	Matrix m_World3;
	Matrix m_View;
	Matrix m_Projection;
	Vector4 m_vMeshColor1 = {1.0f, 0.0f, 0.0f, 1.0f}; // Red
	Vector4 m_vMeshColor2 = {0.0f, 1.0f, 0.0f, 1.0f}; // Green
	Vector4 m_vMeshColor3 = {0.0f, 0.0f, 1.0f, 1.0f}; // Blue
	Vector4 m_vClearColor = {0.0f, 0.0f, 0.0f, 1.0f}; // Background clear color
	Vector3 m_vLightDir = {0.0f, 0.0f, 1.0f};
	Vector3 m_vLightColor = {1.0f, 1.0f, 1.0f};
	
	bool m_bRotateAnimation = false;

	bool OnInitialize() override;
	void OnUninitialize() override;
	void OnUpdate() override;
	void OnRender() override;

	bool InitD3D();
	void UninitD3D();

	bool InitScene();
	void UninitScene();

	bool InitMSAA();
	void UninitMSAA();

	bool InitFXAA();
	void UninitFXAA();

	bool InitImGUI();
	void UninitImGUI();
	void RenderImGUI();

	virtual LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) override;
};
