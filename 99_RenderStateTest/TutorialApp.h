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

	// 그리기에 파이프라인에 적용하는 리소스 객체와 인터페이스
	ComPtr<ID3D11Buffer> m_pVertexBuffer;
	ComPtr<ID3D11Buffer> m_pIndexBuffer;
	ComPtr<ID3D11VertexShader> m_pVertexShader;
	ComPtr<ID3D11PixelShader> m_pPixelShader;
	ComPtr<ID3D11InputLayout> m_pInputLayout;
	ComPtr<ID3D11Buffer> m_pCBNeverChanges;
	ComPtr<ID3D11Buffer> m_pCBChangeOnResize;
	ComPtr<ID3D11Buffer> m_pCBChangesEveryFrame;

	// Render States
	ComPtr<ID3D11RasterizerState> m_pRasterizerState;
	ComPtr<ID3D11DepthStencilState> m_pDepthStencilState;
	ComPtr<ID3D11BlendState> m_pBlendState;

	// Rasterizer State Options
	int m_FillMode = 0; // 0=Solid, 1=Wireframe
	int m_CullMode = 1; // 0=None, 1=Back, 2=Front
	bool m_FrontCounterClockwise = false;
	bool m_DepthClipEnable = true;
	bool m_ScissorEnable = false;

	// Depth Stencil State Options
	bool m_DepthEnable = true;
	int m_DepthWriteMask = 1; // 0=Zero, 1=All
	int m_DepthFunc = 1; // 0=Never, 1=Less, 2=Equal, 3=LessEqual, 4=Greater, 5=NotEqual, 6=GreaterEqual, 7=Always
	bool m_StencilEnable = false;

	// Blend State Options
	bool m_BlendEnable = false;
	int m_SrcBlend = 5; // 5=SrcAlpha
	int m_DestBlend = 6; // 6=InvSrcAlpha
	int m_BlendOp = 1; // 1=Add, 2=Subtract, 3=RevSubtract, 4=Min, 5=Max
	bool m_AlphaToCoverageEnable = false;

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
	Vector4 m_vClearColor = {0.0f, 0.5f, 0.5f, 1.0f}; // Background clear color
	Vector3 m_vLightDir = {0.0f, 0.0f, 1.0f};
	Vector3 m_vLightColor = {1.0f, 1.0f, 1.0f};
	
	bool m_bRotateAnimation = false;
	int m_DrawOrder[3] = {0, 1, 2}; // 0=Cube1, 1=Cube2, 2=Cube3
	
	bool m_bWireframe = false;

	bool OnInitialize() override;
	void OnUninitialize() override;
	void OnUpdate() override;
	void OnRender() override;

	bool InitD3D();
	void UninitD3D();

	bool InitScene();
	void UninitScene();
	
	bool InitRenderStates();
	void UninitRenderStates();
	void UpdateRasterizerState();
	void UpdateDepthStencilState();
	void UpdateBlendState();

	bool InitImGUI();
	void UninitImGUI();
	void RenderImGUI();

	virtual LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) override;
};
