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
#ifdef _DEBUG
	m_pDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
	SAFE_RELEASE(m_pDebug);	
#endif
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
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, color);

	// ����ü�� ��ü.
	m_pSwapChain->Present(0, 0);
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

	UINT creationFlags = 0;
#ifdef _DEBUG
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	// 1. ��ġ �� ����ü�� ����.
	hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, creationFlags, NULL, NULL,
		D3D11_SDK_VERSION, &swapDesc, &m_pSwapChain, &m_pDevice, NULL, &m_pDeviceContext);
	if (FAILED(hr)) {
		LOG_ERROR(L"%s", GetComErrorString(hr));
		return false;
	}

#ifdef _DEBUG
	hr = m_pDevice->QueryInterface(__uuidof(ID3D11Debug), (void**)&m_pDebug);
	if (FAILED(hr)) {
		LOG_ERROR(L"%s", GetComErrorString(hr));
		return false;
	}
#endif


	// 2. ����Ÿ�ٺ� ����.
	// ����ü���� ������ ����۸� ����ϴ�. 
	ID3D11Texture2D* pBackBufferTexture;
	hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBufferTexture);
	if (FAILED(hr)) {
		LOG_ERROR(L"%s", GetComErrorString(hr));
		return false;
	}
	// ����ü���� ����۸� �̿��ϴ� ����Ÿ�ٺ並 �����մϴ�.
	hr = m_pDevice->CreateRenderTargetView(
		pBackBufferTexture, NULL, &m_pRenderTargetView);
	// ����Ÿ�ٺ並 ��������Ƿ� ����� �ؽ�ó �������̽��� ���̻� �ʿ����� �ʽ��ϴ�.
	SAFE_RELEASE(pBackBufferTexture);
	if (FAILED(hr)) {
		LOG_ERROR(L"%s", GetComErrorString(hr));
		return false;
	}

	//3. ���� Ÿ���� ���� ��� ���������ο� ���ε��մϴ�.
	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, NULL);
	return true;
}

void TutorialApp::UninitD3D()
{	
	SAFE_RELEASE(m_pRenderTargetView);
	SAFE_RELEASE(m_pDeviceContext);
	SAFE_RELEASE(m_pSwapChain);
	SAFE_RELEASE(m_pDevice);
}