#pragma once
#include "../Framework/GameApp.h"
#include <imgui.h>

class TutorialApp :
    public GameApp
{
public:
	TutorialApp(HINSTANCE hInstance);
	~TutorialApp();

	// DirectX ����.
	ID3D11Device* pDevice = nullptr;
	ID3D11DeviceContext* pDeviceContext = nullptr;
	IDXGISwapChain* pSwapChain = nullptr;
	ID3D11RenderTargetView* pRenderTargetView = nullptr;

	ID3D11Buffer* vertexBuffer = nullptr;		// ���� ����.
	ID3D11VertexShader* vertexShader = nullptr;		// ���� ���̴�.
	ID3D11PixelShader* pixelShader = nullptr;		// �ȼ� ���̴�.
	ID3DBlob* vertexShaderBuffer = nullptr;		// ���� ���̴� ����.
	ID3DBlob* pixelShaderBuffer = nullptr;		// �ȼ� ���̴� ����.
	ID3D11InputLayout* vertexInputLayout = nullptr;	// �Է� ���̾ƿ�.


	// Our state
	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);


	virtual bool Initialize(UINT Width, UINT Height);
	virtual void Update();
	virtual void Render();

	bool InitD3D();			
	void UninitD3D();

	bool InitImGUI();
	void UninitImGUI();

	bool InitScene();		// ���̴�,���ؽ�,�ε���
	void UninitScene();		 

	virtual LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

