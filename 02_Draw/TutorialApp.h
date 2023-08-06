#pragma once
#include "../Framework/GameApp.h"
#include "../imgui/imgui.h"

class TutorialApp :
    public GameApp
{
public:
	TutorialApp(HINSTANCE hInstance);
	~TutorialApp();

	// DirectX 변수.
	ID3D11Device* pDevice = nullptr;
	ID3D11DeviceContext* pDeviceContext = nullptr;
	IDXGISwapChain* pSwapChain = nullptr;
	ID3D11RenderTargetView* pRenderTargetView = nullptr;

	ID3D11Buffer* vertexBuffer;		// 정점 버퍼.
	ID3D11VertexShader* vertexShader;		// 정점 셰이더.
	ID3D11PixelShader* pixelShader;			// 픽셀 셰이더.
	ID3DBlob* vertexShaderBuffer;		// 정점 셰이더 버퍼.
	ID3DBlob* pixelShaderBuffer;		// 픽셀 셰이더 버퍼.
	ID3D11InputLayout* vertexInputLayout;	// 입력 레이아웃.


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

	bool InitScene();		// 쉐이더,버텍스,인덱스
	void UninitScene();		 

	virtual LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

