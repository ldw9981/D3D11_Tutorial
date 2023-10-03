#pragma once
#include "../Common/GameApp.h"
#include <d3d11.h>
#include <imgui.h>

#include <directxtk/SimpleMath.h>
using namespace DirectX::SimpleMath;

class TutorialApp :
    public GameApp
{
public:
	TutorialApp(HINSTANCE hInstance);
	~TutorialApp();

	// ������ ������������ �����ϴ� �ʼ� ��ü�� �������̽�
	ID3D11Device* m_pDevice = nullptr;						// ����̽�	
	ID3D11DeviceContext* m_pDeviceContext = nullptr;		// ��� ����̽� ���ؽ�Ʈ
	IDXGISwapChain* m_pSwapChain = nullptr;					// ����ü��
	ID3D11RenderTargetView* m_pRenderTargetView = nullptr;	// ������ Ÿ�ٺ�
	// �X�� ���ٽ� �䵵 ������ ���� ������� �ʴ´�.

	Vector4 m_ClearColor = Vector4(0.45f, 0.55f, 0.60f, 1.00f);

	bool m_show_another_window = false;
	bool m_show_demo_window = true;
	float m_f;
	int m_counter;

	virtual bool Initialize(UINT Width, UINT Height);
	virtual void Update();
	virtual void Render();

	bool InitD3D();
	void UninitD3D();

	bool InitImGUI();
	void UninitImGUI();

	virtual LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

