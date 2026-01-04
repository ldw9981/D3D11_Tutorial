#pragma once
#include "../Common/GameApp.h"
#include <dxgi1_4.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <directxtk/SimpleMath.h>
#include <string>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

#pragma comment (lib, "d3d11.lib")
#pragma comment(lib,"dxgi.lib")

using namespace DirectX::SimpleMath;
using namespace Microsoft::WRL;
using namespace std;

struct Vertex
{
	Vector3 position;
	Vector4 color;
};

struct ConstantBuffer
{
	Matrix world;
	Matrix view;
	Matrix projection;
};

enum class BillboardType
{
	Identity = 0,           // Identity 행렬 적용
	YAxisLocked = 1,    // 연기/불꽃 - Y축 기준으로만 회전하여 카메라 위치를 바라보도록 한다.
	Spherical = 2,      // 스파크 - Spherical	X,Y,Z축 모두 회전 사용하여 카메라 위치를 바라보도록 한다.
	ScreenAligned = 3   // UI 이펙트 - Screen aligned  카메라의 화면과 항상 완전히 평행하게 정렬하여 정면으로 보입
};

class TutorialApp : public GameApp
{
public:
	ComPtr<IDXGIFactory4> m_pDXGIFactory;
	ComPtr<IDXGIAdapter3> m_pDXGIAdapter;

	// 렌더링 파이프라인 필수 객체
	ID3D11Device* m_pDevice = nullptr;
	ID3D11DeviceContext* m_pDeviceContext = nullptr;
	IDXGISwapChain* m_pSwapChain = nullptr;
	ID3D11RenderTargetView* m_pRenderTargetView = nullptr;

	// Quad 렌더링용 리소스
	ID3D11Buffer* m_pVertexBuffer = nullptr;
	ID3D11Buffer* m_pIndexBuffer = nullptr;
	ID3D11Buffer* m_pConstantBuffer = nullptr;
	ID3D11VertexShader* m_pVertexShader = nullptr;
	ID3D11PixelShader* m_pPixelShader = nullptr;
	ID3D11InputLayout* m_pInputLayout = nullptr;

	// 변환 행렬
	Matrix m_World;
	Matrix m_View;
	Matrix m_Projection;

	// 빌보드 정렬(회전) 행렬
	Matrix m_BillboardMatrix = Matrix::Identity;

	// 빌보드 변환 정보
	Vector3 m_BillboardPosition = Vector3::Zero;	// 빌보드 위치

	// 내부 계산용(외부에서 관리할 필요 없음)
	Vector3 m_BillboardPrevPosition = Vector3::Zero;
	Vector3 m_BillboardVelocity = Vector3::Zero;

	Vector4 m_ClearColor = Vector4(0.2f, 0.2f, 0.3f, 1.0f);

	BillboardType m_CurrentBillboardType = BillboardType::Identity;

	bool OnInitialize() override;
	void OnUninitialize() override;
	void OnUpdate() override;
	void OnRender() override;

	bool InitD3D();
	void UninitD3D();

	bool CreateQuadGeometry();
	bool CreateShaders();

	virtual LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};
