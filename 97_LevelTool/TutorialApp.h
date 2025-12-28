#pragma once
#include <windows.h>
#include "../Common/GameApp.h"
#include <d3d11.h>
#include <directxtk/SimpleMath.h>
#include <wrl/client.h>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <directxtk/PrimitiveBatch.h>
#include <directxtk/VertexTypes.h>
#include <directxtk/Effects.h>
#include <directxtk/CommonStates.h>

#include "CubeObject.h"
#include "GameWorld.h"
#include "../Common/Camera.h"
#include "../Common/DebugDraw.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX::SimpleMath;
using namespace DirectX;

class TutorialApp :
	public GameApp
{
public:
	// 렌더링 파이프라인에 적용되는 주요 객체의 인터페이스
	ComPtr<ID3D11Device> m_pDevice;						// 디바이스
	ComPtr<ID3D11DeviceContext> m_pDeviceContext;		// 즉시 디바이스 컨텍스트
	ComPtr<IDXGISwapChain> m_pSwapChain;					// 스왑체인
	ComPtr<ID3D11RenderTargetView> m_pRenderTargetView;	// 렌더 타겟뷰
	ComPtr<ID3D11DepthStencilView> m_pDepthStencilView;  // 깊이 스텐실 뷰

	// 렌더링 파이프라인에 적용되는  객체들 포인터
	ComPtr<ID3D11VertexShader> m_pVertexShader;		// 버텍스 셰이더
	ComPtr<ID3D11PixelShader> m_pPixelShader;		// 픽셀 셰이더
	ComPtr<ID3D11InputLayout> m_pInputLayout;		// 입력 레이아웃.
	ComPtr<ID3D11Buffer> m_pVertexBuffer;			// 버텍스 버퍼.
	UINT m_VertexBufferStride = 0;						// 버텍스 하나의 크기.
	UINT m_VertexBufferOffset = 0;						// 버텍스 버퍼의 오프셋
	ComPtr<ID3D11Buffer> m_pIndexBuffer;				// 인덱스 버퍼.
	int m_nIndices = 0;									// 인덱스 개수.
	ComPtr<ID3D11Buffer> m_pConstantBuffer;			// 상수 버퍼.

	Matrix                m_View;				// 뷰좌표계 변환행렬.
	Matrix                m_Projection;			// 투영좌표계 변환행렬.

	XMFLOAT4 m_LightDir = XMFLOAT4(0.0f, 0.0f, 1.0f, 0.0f);	// 조명 방향
	XMFLOAT4 m_LightColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);	// 조명 색상
	XMFLOAT4 m_MaterialColor = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);	// 재질 색상


	GameWorld m_World;
	Camera m_Camera;


	GameObject* m_pSelectedObject = nullptr;


	std::vector<std::string> m_AvailableObjectTypes;


	bool m_bShowDebugRay = false;
	Vector3 m_DebugRayOrigin;
	Vector3 m_DebugRayDirection;


	bool m_bShowAllAABB = false;			// 모든 오브젝트의 AABB 표시

	bool OnInitialize() override;
	void OnUninitialize() override;
	void OnUpdate() override;
	void OnRender() override;

	bool InitD3D();
	void UninitD3D();

	bool InitScene();
	void UninitScene();

	virtual LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) override;
	bool InitImGui();
	void UninitImGui();

	void RenderImGuiCubeRTTR();
	void RenderObjectPaletteContent();
	void RenderWorldHierarchyContent();
	void RenderInspectorContent();

	void SaveScene();
	void LoadScene();

	// InputProcesser
	virtual void OnInputProcess(const Keyboard::State& KeyState, const Keyboard::KeyboardStateTracker& KeyTracker,
		const Mouse::State& MouseState, const Mouse::ButtonStateTracker& MouseTracker) override;
};
