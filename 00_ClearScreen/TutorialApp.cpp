#include "framework.h"
#include "TutorialApp.h"

#pragma comment (lib, "d3d11.lib")

TutorialApp::TutorialApp(HINSTANCE hInstance)
:GameApp(hInstance)
{

}

TutorialApp::~TutorialApp()
{
	UninitD3D();
}

bool TutorialApp::Initialize(UINT Width, UINT Height)
{
	__super::Initialize(Width, Height);

	if (!InitD3D())
		return false;

	return true;
}

void TutorialApp::Update()
{

}

void TutorialApp::Render()
{
	float color[4] = { 0.0f, 0.5f, 0.5f, 1.0f };

	// ȭ�� ĥ�ϱ�.
	pDeviceContext->ClearRenderTargetView(pRenderTargetView, color);

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
	swapDesc.BufferDesc.Width = m_ClientSize.cx;
	swapDesc.BufferDesc.Height = m_ClientSize.cy;
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