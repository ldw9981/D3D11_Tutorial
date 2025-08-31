#include "TutorialApp.h"
#include "../Common/Helper.h"

#include <directxtk/simplemath.h>
#include <dxgidebug.h>
#include <dxgi1_3.h>
#include <wrl/client.h>

#pragma comment (lib, "d3d11.lib")
#pragma comment(lib,"dxgi.lib")

using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;

#define USE_FLIPMODE 1			//��� �޼����� ���ַ��� Flip ��带 ����Ѵ�.

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
#if USE_FLIPMODE==1
	// Flip��忡���� �������� ���� �ʿ�
	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, NULL);
#endif
	Color color(0.0f, 0.5f, 0.5f, 1.0f);
	// ȭ�� ĥ�ϱ�.
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, color);

	// ����ü�� ��ü.
	m_pSwapChain->Present(0, 0);
}


bool TutorialApp::InitD3D()
{
	HRESULT hr = 0;

	// 1. D3D11 Device,DeviceContext ����
	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	// �׷��� ī�� �ϵ������ �������� ȣȯ�Ǵ� ���� ���� DirectX ��ɷ����� �����Ͽ� ����̹��� �۵� �Ѵ�.
	// �������̽��� Direc3D11 ������ GPU����̹��� D3D12 ����̹��� �۵��Ҽ��� �ִ�.
	D3D_FEATURE_LEVEL featureLevels[] = { // index 0���� ������� �õ��Ѵ�.
		D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0,D3D_FEATURE_LEVEL_11_1,D3D_FEATURE_LEVEL_11_0
	};
	D3D_FEATURE_LEVEL actualFeatureLevel; // ���� ��ó ������ ������ ����

	HR_T(D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		0,
		creationFlags,
		featureLevels,
		ARRAYSIZE(featureLevels),
		D3D11_SDK_VERSION,
		&m_pDevice,
		&actualFeatureLevel,
		&m_pDeviceContext
	));

	// 2. ����ü�� ������ ���� DXGI Factory ����
	UINT dxgiFactoryFlags = 0;
#ifdef _DEBUG
	dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	ComPtr<IDXGIFactory2> pFactory;
	HR_T(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&pFactory)));

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
#if USE_FLIPMODE==1
	swapChainDesc.BufferCount = 2;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
#else
	swapChainDesc.BufferCount = 1;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
#endif
	swapChainDesc.Width = m_ClientWidth;
	swapChainDesc.Height = m_ClientHeight;
	// �ϳ��� �ȼ��� ä�� RGBA �� 8��Ʈ �������� ǥ���Ǹ� 
	// Unsigned Normalized Integer 8��Ʈ ����(0~255)�ܰ踦 �ε��Ҽ������� ����ȭ�� 0.0~1.0���� �����Ͽ� ǥ���Ѵ�.
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // ���� ü���� �� ���۰� ������ ������������ ���� ��� ������� ���
	swapChainDesc.SampleDesc.Count = 1;  // ��Ƽ���ø� ��� ����
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE; // Recommended for flip models
	swapChainDesc.Stereo = FALSE;  // ���׷��� 3D �������� ��Ȱ��ȭ
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // ��ü ȭ�� ��ȯ�� ���
	swapChainDesc.Scaling = DXGI_SCALING_NONE; //  â�� ũ��� �� ������ ũ�Ⱑ �ٸ� ��. ����� ũ�⿡ �°� �����ϸ� ���� �ʴ´�.
		
	HR_T(pFactory->CreateSwapChainForHwnd(
		m_pDevice,
		m_hWnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		&m_pSwapChain
	));

	// 3. ����Ÿ�� �� ����.  (����۸� �ؽ�ó�� �̿��ϴ� ����Ÿ�ٺ�) 
	//  ���� Ÿ�� ��� "����ٰ� �׸��� �׷���"��� GPU���� �˷��ִ� ������ �ϴ� ��ü.
	ComPtr<ID3D11Texture2D> pBackBufferTexture;
	HR_T(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBufferTexture));
	HR_T(m_pDevice->CreateRenderTargetView(pBackBufferTexture.Get(), nullptr, &m_pRenderTargetView));

#if !USE_FLIPMODE
	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, nullptr);
#endif

	return true;
}

void TutorialApp::UninitD3D()
{	
	SAFE_RELEASE(m_pRenderTargetView);
	SAFE_RELEASE(m_pDeviceContext);
	SAFE_RELEASE(m_pSwapChain);
	SAFE_RELEASE(m_pDevice);
}