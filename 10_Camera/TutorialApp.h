#pragma once
#include <windows.h>
#include "../Common/GameApp.h"
#include <d3d11.h>
#include <directxtk/SimpleMath.h>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

using namespace DirectX::SimpleMath;
using namespace DirectX;

class TutorialApp :
	public GameApp
{
public:
	// ������ ������������ �����ϴ� �ʼ� ��ü�� �������̽� 
	ID3D11Device* m_pDevice = nullptr;						// ����̽�	
	ID3D11DeviceContext* m_pDeviceContext = nullptr;		// ��� ����̽� ���ؽ�Ʈ
	IDXGISwapChain* m_pSwapChain = nullptr;					// ����ü��
	ID3D11RenderTargetView* m_pRenderTargetView = nullptr;	// ������ Ÿ�ٺ�
	ID3D11DepthStencilView* m_pDepthStencilView = nullptr;  // ���̰� ó���� ���� �X�����ٽ� ��

	// ������ ���������ο� �����ϴ�  ��ü�� ����
	ID3D11VertexShader* m_pVertexShader = nullptr;		// ���� ���̴�.
	ID3D11PixelShader* m_pPixelShader = nullptr;		// �ȼ� ���̴�.	
	ID3D11PixelShader* m_pPixelShaderSolid = nullptr;	// �ȼ� ���̴� ����Ʈ ǥ�ÿ�.	
	ID3D11InputLayout* m_pInputLayout = nullptr;		// �Է� ���̾ƿ�.
	ID3D11Buffer* m_pVertexBuffer = nullptr;			// ���ؽ� ����.
	UINT m_VertexBufferStride = 0;						// ���ؽ� �ϳ��� ũ��.
	UINT m_VertexBufferOffset = 0;						// ���ؽ� ������ ������.
	ID3D11Buffer* m_pIndexBuffer = nullptr;				// ���ؽ� ����.
	int m_nIndices = 0;									// �ε��� ����.
	ID3D11Buffer* m_pConstantBuffer = nullptr;			// ��� ����.

	Matrix                m_World;				// ������ǥ�� �������� ��ȯ�� ���� ���.
	Matrix                m_View;				// ����ǥ�� �������� ��ȯ�� ���� ���.
	Matrix                m_Projection;			// ������ġ��ǥ��( Normalized Device Coordinate) �������� ��ȯ�� ���� ���.

	Matrix				  m_CameraInverse;		// ī�޶��� �����
	Vector3 m_RotationDegree = {0,0,0};

	XMFLOAT4 m_LightColors[2] =		// ����Ʈ ����
	{
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)
	};
	XMFLOAT4 m_InitialLightDirs[2] =	// �ʱ� ����Ʈ ����
	{
		XMFLOAT4(-0.577f, 0.577f, -0.577f, 1.0f),
		XMFLOAT4(0.0f, 0.0f, -1.0f, 1.0f),
	};	
	XMFLOAT4 m_LightDirsEvaluated[2] = {};		// ���� ����Ʈ ����

	bool OnInitialize() override;
	void OnUninitialize() override;
	void OnUpdate() override;
	void OnRender() override;

	void RenderImGUI();
	bool InitD3D();
	void UninitD3D();

	bool InitScene();		// ���̴�,���ؽ�,�ε���
	void UninitScene();

	bool InitImGUI();
	void UninitImGUI();

	virtual LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

