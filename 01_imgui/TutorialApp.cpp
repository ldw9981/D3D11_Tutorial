#include "TutorialApp.h"
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

#pragma comment (lib, "d3d11.lib")

TutorialApp::TutorialApp(HINSTANCE hInstance)
:GameApp(hInstance)
{

}

TutorialApp::~TutorialApp()
{
	UninitImGUI();
	UninitD3D();	
}

bool TutorialApp::Initialize(UINT Width, UINT Height)
{
	__super::Initialize(Width, Height);

	if(!InitD3D())
		return false;
	
	if (!InitImGUI())
		return false;

	return true;
}

void TutorialApp::Update()
{

}

void TutorialApp::Render()
{

	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	
	{
		ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.
		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)	
		ImGui::ColorEdit3("clear color", (float*)&m_ClearColor); // Edit 3 floats representing a color			
		ImGui::End();
	}



	ImGui::Render();
	const float clear_color_with_alpha[4] = { m_ClearColor.x , m_ClearColor.y , m_ClearColor.z, m_ClearColor.w };
	this->pDeviceContext->OMSetRenderTargets(1, &this->pRenderTargetView, nullptr);
	this->pDeviceContext->ClearRenderTargetView(this->pRenderTargetView, clear_color_with_alpha);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());


	// ����ü�� ��ü.
	pSwapChain->Present(0, 0);
}

bool TutorialApp::InitD3D()
{
	// �����.
	HRESULT hr;

	// ����ü�� �Ӽ� ���� ����ü ����.
	DXGI_SWAP_CHAIN_DESC swapDesc;
	ZeroMemory(&swapDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	swapDesc.BufferCount = 1;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.OutputWindow = m_hWnd;	// ����ü�� ����� â �ڵ� ��.
	swapDesc.Windowed = true;		// â ��� ���� ����.
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	// �����(�ؽ�ó)�� ����/���� ũ�� ����.
	swapDesc.BufferDesc.Width = m_ClientWidth;
	swapDesc.BufferDesc.Height = m_ClientHeight;
	// ȭ�� �ֻ��� ����.
	swapDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapDesc.BufferDesc.RefreshRate.Denominator = 1;
	// ���ø� ���� ����.
	swapDesc.SampleDesc.Count = 1;
	swapDesc.SampleDesc.Quality = 0;

	// ��ġ �� ����ü�� ����.
	hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, NULL, NULL,
		D3D11_SDK_VERSION, &swapDesc, &pSwapChain, &pDevice,
		NULL, &pDeviceContext);

	if (FAILED(hr))
	{
		MessageBox(NULL, L"��ġ ���� ����.", L"����.", MB_OK);
		return false;
	}

	// �����(�ؽ�ó).
	ID3D11Texture2D* pBackBufferTexture;
	hr = pSwapChain->GetBuffer(NULL,
		__uuidof(ID3D11Texture2D),
		(void**)&pBackBufferTexture);

	if (FAILED(hr))
	{
		MessageBox(NULL, L"����� ���� ����.", L"����.", MB_OK);
		return false;
	}

	// ���� Ÿ�� ����.
	hr = pDevice->CreateRenderTargetView(
		pBackBufferTexture, NULL, &pRenderTargetView);

	if (FAILED(hr))
	{
		MessageBox(NULL, L"���� Ÿ�� ���� ����.", L"����.", MB_OK);
		return false;
	}

	// ���� Ÿ�� ����.
	pDeviceContext->OMSetRenderTargets(1, &pRenderTargetView, NULL);

	// ����� �ؽ�ó ����.
	if (pBackBufferTexture)
	{
		pBackBufferTexture->Release();
		pBackBufferTexture = NULL;
	}
	return true;
}

void TutorialApp::UninitD3D()
{
	// Cleanup DirectX
	if (pDevice)
	{
		pDevice->Release();
		pDevice = NULL;
	}

	if (pDeviceContext)
	{
		pDeviceContext->Release();
		pDeviceContext = NULL;
	}

	if (pSwapChain)
	{
		pSwapChain->Release();
		pSwapChain = NULL;
	}

	if (pRenderTargetView)
	{
		pRenderTargetView->Release();
		pRenderTargetView = NULL;
	}
}

bool TutorialApp::InitImGUI()
{
	/*
		ImGui �ʱ�ȭ.
	*/
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(m_hWnd);
	ImGui_ImplDX11_Init(this->pDevice, this->pDeviceContext);

	//
	return true;
}

void TutorialApp::UninitImGUI()
{
	// Cleanup
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK TutorialApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
		return true;

    return __super::WndProc(hWnd, message, wParam, lParam);
}