#include "TutorialApp.h"
#include "../Common/Helper.h"

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
	swapDesc.BufferDesc.Width = m_ClientWidth;
	swapDesc.BufferDesc.Height = m_ClientHeight;
	// ȭ�� �ֻ��� ����.
	swapDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapDesc.BufferDesc.RefreshRate.Denominator = 1;
	// ���ø� ���� ����.
	swapDesc.SampleDesc.Count = 1;
	swapDesc.SampleDesc.Quality = 0;

	// 1. ��ġ �� ����ü�� ����.
	hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, NULL, NULL,
		D3D11_SDK_VERSION, &swapDesc, &pSwapChain, &pDevice, NULL, &pDeviceContext);
	if (FAILED(hr)) {
		LOG_ERROR(L"%s", GetComErrorString(hr));
		return false;
	}

	// 2. ����Ÿ�ٺ� ����.
	// ����ü���� ������ ����۸� ����ϴ�. 
	ID3D11Texture2D* pBackBufferTexture;
	hr = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBufferTexture);
	if (FAILED(hr)) {
		LOG_ERROR(L"%s", GetComErrorString(hr));
		return false;
	}
	// ����ü���� ����۸� �̿��ϴ� ����Ÿ�ٺ並 �����մϴ�.
	hr = pDevice->CreateRenderTargetView(
		pBackBufferTexture, NULL, &pRenderTargetView);
	// ����Ÿ�ٺ並 ��������Ƿ� ����� �ؽ�ó �������̽��� ���̻� �ʿ����� �ʽ��ϴ�.
	SAFE_RELEASE(pBackBufferTexture);
	if (FAILED(hr)) {
		LOG_ERROR(L"%s", GetComErrorString(hr));
		return false;
	}

	//3. ���� Ÿ���� ���� ��� ���������ο� ���ε��մϴ�.
	pDeviceContext->OMSetRenderTargets(1, &pRenderTargetView, NULL);
	return true;
}

void TutorialApp::UninitD3D()
{
	// Cleanup DirectX
	SAFE_RELEASE(pDevice);
	SAFE_RELEASE(pDeviceContext);
	SAFE_RELEASE(pSwapChain);
	SAFE_RELEASE(pRenderTargetView);
}