#include "pch.h"
#include "TutorialApp.h"

#include <sstream>
#include <algorithm>

using namespace DirectX;
using namespace std;
namespace
{
	D3D11_INPUT_ELEMENT_DESC kLayout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	constexpr UINT kNumElements = ARRAYSIZE(kLayout);
}

TutorialApp::TutorialApp()
{
	SetClientSize(300, 300);
	SetWindowTitle(L"12_TextD2DRenderTarget");
}

bool TutorialApp::OnInitialize()
{
	HRESULT hrCom = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (SUCCEEDED(hrCom))
		m_ComInitialized = true;

	if (!InitD3D())
		return false;

	if (!InitD2D_D3D11_DWrite())
		return false;

	if (!InitScene())
		return false;

	return true;
}

void TutorialApp::OnUninitialize()
{
	UninitD2D_DWrite();
	UninitD3D();
	if (m_ComInitialized)
	{
		CoUninitialize();
		m_ComInitialized = false;
	}
}

void TutorialApp::OnUpdate()
{
	m_Rot += 0.005f;
	if (m_Rot > 6.28f)
		m_Rot = 0.0f;

	m_Cube1World = XMMatrixIdentity();

	XMVECTOR rotaxis = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	m_Rotation = XMMatrixRotationAxis(rotaxis, m_Rot);
	m_Translation = XMMatrixTranslation(0.0f, 0.0f, 4.0f);
	m_Cube1World = m_Translation * m_Rotation;

	m_Cube2World = XMMatrixIdentity();
	m_Rotation = XMMatrixRotationAxis(rotaxis, -m_Rot);
	m_Scale = XMMatrixScaling(1.3f, 1.3f, 1.3f);
	m_Cube2World = m_Rotation * m_Scale;
}

void TutorialApp::OnRender()
{
	float bgColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_DeviceContext->ClearRenderTargetView(m_RenderTargetView, bgColor);
	m_DeviceContext->ClearDepthStencilView(m_DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	m_DeviceContext->OMSetRenderTargets(1, &m_RenderTargetView, m_DepthStencilView);
	m_DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);

	m_DeviceContext->IASetIndexBuffer(m_SquareIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	UINT stride = sizeof(CubeVertex);
	UINT offset = 0;
	m_DeviceContext->IASetVertexBuffers(0, 1, &m_SquareVertBuffer, &stride, &offset);

	m_WVP = m_Cube1World * m_CamView * m_CamProjection;
	m_CBPerObj.WVP = XMMatrixTranspose(m_WVP);
	m_DeviceContext->UpdateSubresource(m_CBPerObjectBuffer, 0, nullptr, &m_CBPerObj, 0, 0);
	m_DeviceContext->VSSetConstantBuffers(0, 1, &m_CBPerObjectBuffer);
	m_DeviceContext->PSSetShaderResources(0, 1, &m_CubesTexture);
	m_DeviceContext->PSSetSamplers(0, 1, &m_CubesTexSamplerState);
	m_DeviceContext->RSSetState(m_CWcullMode);
	m_DeviceContext->DrawIndexed(36, 0, 0);

	m_WVP = m_Cube2World * m_CamView * m_CamProjection;
	m_CBPerObj.WVP = XMMatrixTranspose(m_WVP);
	m_DeviceContext->UpdateSubresource(m_CBPerObjectBuffer, 0, nullptr, &m_CBPerObj, 0, 0);
	m_DeviceContext->VSSetConstantBuffers(0, 1, &m_CBPerObjectBuffer);
	m_DeviceContext->PSSetShaderResources(0, 1, &m_CubesTexture);
	m_DeviceContext->PSSetSamplers(0, 1, &m_CubesTexSamplerState);
	m_DeviceContext->RSSetState(m_CWcullMode);
	m_DeviceContext->DrawIndexed(36, 0, 0);

	RenderText(L"Hello World");

	m_SwapChain->Present(0, 0);
}

LRESULT CALLBACK TutorialApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return GameApp::WndProc(hWnd, message, wParam, lParam);
}

bool TutorialApp::InitD3D()
{
	DXGI_MODE_DESC bufferDesc = {};
	bufferDesc.Width = m_ClientWidth;
	bufferDesc.Height = m_ClientHeight;
	bufferDesc.RefreshRate.Numerator = 60;
	bufferDesc.RefreshRate.Denominator = 1;
	bufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferDesc = bufferDesc;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.OutputWindow = m_hWnd;
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	IDXGIFactory1* dxgiFactory = nullptr;
	HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&dxgiFactory);
	if (FAILED(hr))
		return false;

	IDXGIAdapter1* adapter = nullptr;
	hr = dxgiFactory->EnumAdapters1(0, &adapter);
	dxgiFactory->Release();
	if (FAILED(hr))
		return false;

	UINT deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG)
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	hr = D3D11CreateDeviceAndSwapChain(
		adapter,
		D3D_DRIVER_TYPE_UNKNOWN,
		nullptr,
		deviceFlags,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&swapChainDesc,
		&m_SwapChain,
		&m_Device,
		nullptr,
		&m_DeviceContext);
	adapter->Release();
	if (FAILED(hr))
		return false;

	ID3D11Texture2D* backBuffer = nullptr;
	hr = m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
	if (FAILED(hr))	return false;
	hr = m_Device->CreateRenderTargetView(backBuffer, nullptr, &m_RenderTargetView);
	backBuffer->Release();
	if (FAILED(hr))	return false;

	D3D11_TEXTURE2D_DESC depthStencilDesc = {};
	depthStencilDesc.Width = m_ClientWidth;
	depthStencilDesc.Height = m_ClientHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	hr = m_Device->CreateTexture2D(&depthStencilDesc, nullptr, &m_DepthStencilBuffer);
	if (FAILED(hr))	return false;
	hr = m_Device->CreateDepthStencilView(m_DepthStencilBuffer, nullptr, &m_DepthStencilView);
	if (FAILED(hr))	return false;

	return true;
}

void TutorialApp::UninitD3D()
{
	if (m_SwapChain) { m_SwapChain->Release(); m_SwapChain = nullptr; }
	if (m_DeviceContext) { m_DeviceContext->Release(); m_DeviceContext = nullptr; }
	if (m_Device) { m_Device->Release(); m_Device = nullptr; }
	if (m_RenderTargetView) { m_RenderTargetView->Release(); m_RenderTargetView = nullptr; }
	if (m_DepthStencilView) { m_DepthStencilView->Release(); m_DepthStencilView = nullptr; }
	if (m_DepthStencilBuffer) { m_DepthStencilBuffer->Release(); m_DepthStencilBuffer = nullptr; }

	if (m_SquareVertBuffer) { m_SquareVertBuffer->Release(); m_SquareVertBuffer = nullptr; }
	if (m_SquareIndexBuffer) { m_SquareIndexBuffer->Release(); m_SquareIndexBuffer = nullptr; }
	if (m_VS) { m_VS->Release(); m_VS = nullptr; }
	if (m_PS) { m_PS->Release(); m_PS = nullptr; }
	if (m_VS_Buffer) { m_VS_Buffer->Release(); m_VS_Buffer = nullptr; }
	if (m_PS_Buffer) { m_PS_Buffer->Release(); m_PS_Buffer = nullptr; }
	if (m_VertLayout) { m_VertLayout->Release(); m_VertLayout = nullptr; }
	if (m_CBPerObjectBuffer) { m_CBPerObjectBuffer->Release(); m_CBPerObjectBuffer = nullptr; }
	if (m_Transparency) { m_Transparency->Release(); m_Transparency = nullptr; }
	if (m_CCWcullMode) { m_CCWcullMode->Release(); m_CCWcullMode = nullptr; }
	if (m_CWcullMode) { m_CWcullMode->Release(); m_CWcullMode = nullptr; }
	if (m_CubesTexture) { m_CubesTexture->Release(); m_CubesTexture = nullptr; }
	if (m_CubesTexSamplerState) { m_CubesTexSamplerState->Release(); m_CubesTexSamplerState = nullptr; }
}

bool TutorialApp::InitD2D_D3D11_DWrite()
{
	D3D11_TEXTURE2D_DESC sharedTexDesc = {};
	sharedTexDesc.Width = m_ClientWidth;
	sharedTexDesc.Height = m_ClientHeight;
	sharedTexDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	sharedTexDesc.MipLevels = 1;
	sharedTexDesc.ArraySize = 1;
	sharedTexDesc.SampleDesc.Count = 1;
	sharedTexDesc.Usage = D3D11_USAGE_DEFAULT;
	sharedTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

	HRESULT hr = m_Device->CreateTexture2D(&sharedTexDesc, nullptr, &m_SharedTex11);
	if (FAILED(hr))
		return false;

	D2D1_FACTORY_OPTIONS options = {};
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory1), &options, (void**)&m_D2DFactory);
	if (FAILED(hr))
		return false;

	IDXGIDevice* dxgiDevice = nullptr;
	hr = m_Device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
	if (FAILED(hr))
		return false;

	hr = m_D2DFactory->CreateDevice(dxgiDevice, &m_D2DDevice);
	dxgiDevice->Release();
	if (FAILED(hr))
		return false;

	hr = m_D2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_D2DDeviceContext);
	if (FAILED(hr))
		return false;

	IDXGISurface* sharedSurface = nullptr;
	hr = m_SharedTex11->QueryInterface(__uuidof(IDXGISurface), (void**)&sharedSurface);
	if (FAILED(hr))
		return false;

	D2D1_BITMAP_PROPERTIES1 bitmapProps = {};
	bitmapProps.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
	bitmapProps.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
	bitmapProps.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET;
	bitmapProps.dpiX = 96.0f;
	bitmapProps.dpiY = 96.0f;

	hr = m_D2DDeviceContext->CreateBitmapFromDxgiSurface(sharedSurface, &bitmapProps, &m_D2DTargetBitmap);
	sharedSurface->Release();
	if (FAILED(hr))
		return false;

	m_D2DDeviceContext->SetTarget(m_D2DTargetBitmap);

	hr = m_D2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 0.0f, 1.0f), &m_Brush);
	if (FAILED(hr))
		return false;

	hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&m_DWriteFactory));
	if (FAILED(hr))
		return false;

	hr = m_DWriteFactory->CreateTextFormat(
		L"Script",
		nullptr,
		DWRITE_FONT_WEIGHT_REGULAR,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		24.0f,
		L"en-us",
		&m_TextFormat);
	if (FAILED(hr))
		return false;

	m_TextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	m_TextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

	// Load test image (Direct2D)
	{
		const wchar_t* candidates[] = {
			L"../Resource/Tree.png",
			L"../resource/Tree.png",
			L"..\\..\\Resource\\Tree.png",
			L"..\\..\\resource\\Tree.png",
		};

		for (const wchar_t* path : candidates)
		{
			if (SUCCEEDED(CreateD2DBitmapFromFile(path, &m_TestTreeBitmap)) && m_TestTreeBitmap)
				break;
		}
	}

	return true;
}

void TutorialApp::UninitD2D_DWrite()
{
	if (m_TestTreeBitmap) { m_TestTreeBitmap->Release(); m_TestTreeBitmap = nullptr; }
	if (m_Brush) { m_Brush->Release(); m_Brush = nullptr; }
	if (m_D2DTargetBitmap) { m_D2DTargetBitmap->Release(); m_D2DTargetBitmap = nullptr; }
	if (m_D2DDeviceContext) { m_D2DDeviceContext->Release(); m_D2DDeviceContext = nullptr; }
	if (m_D2DDevice) { m_D2DDevice->Release(); m_D2DDevice = nullptr; }
	if (m_D2DFactory) { m_D2DFactory->Release(); m_D2DFactory = nullptr; }
	if (m_SharedTex11) { m_SharedTex11->Release(); m_SharedTex11 = nullptr; }
	if (m_DWriteFactory) { m_DWriteFactory->Release(); m_DWriteFactory = nullptr; }
	if (m_TextFormat) { m_TextFormat->Release(); m_TextFormat = nullptr; }
	if (m_D2DTexture) { m_D2DTexture->Release(); m_D2DTexture = nullptr; }
	if (m_D2DVertBuffer) { m_D2DVertBuffer->Release(); m_D2DVertBuffer = nullptr; }
	if (m_D2DIndexBuffer) { m_D2DIndexBuffer->Release(); m_D2DIndexBuffer = nullptr; }
}

HRESULT TutorialApp::CreateD2DBitmapFromFile(const wchar_t* fileName, ID2D1Bitmap1** outBitmap)
{
	if (!outBitmap)
		return E_INVALIDARG;
	*outBitmap = nullptr;

	if (!m_D2DDeviceContext)
		return E_FAIL;

	IWICImagingFactory* wicFactory = nullptr;
	IWICBitmapDecoder* decoder = nullptr;
	IWICBitmapFrameDecode* frame = nullptr;
	IWICFormatConverter* converter = nullptr;

	HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory2, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wicFactory));
	if (FAILED(hr))
	{
		hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wicFactory));
	}
	if (FAILED(hr))
		return hr;

	hr = wicFactory->CreateDecoderFromFilename(fileName, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder);
	if (FAILED(hr))
		goto Cleanup;

	hr = decoder->GetFrame(0, &frame);
	if (FAILED(hr))
		goto Cleanup;

	hr = wicFactory->CreateFormatConverter(&converter);
	if (FAILED(hr))
		goto Cleanup;

	hr = converter->Initialize(
		frame,
		GUID_WICPixelFormat32bppPBGRA,
		WICBitmapDitherTypeNone,
		nullptr,
		0.0,
		WICBitmapPaletteTypeMedianCut);
	if (FAILED(hr))
		goto Cleanup;

	hr = m_D2DDeviceContext->CreateBitmapFromWicBitmap(converter, nullptr, outBitmap);

Cleanup:
	if (converter) converter->Release();
	if (frame) frame->Release();
	if (decoder) decoder->Release();
	if (wicFactory) wicFactory->Release();
	return hr;
}

bool TutorialApp::InitScene()
{
	HRESULT hr = S_OK;

	// Compile shaders
	hr = CompileShaderFromFile(L"../shaders/12_EffectVS.hlsl", 0, "main", "vs_4_0", &m_VS_Buffer);
	if (FAILED(hr))
		return false;

	hr = CompileShaderFromFile(L"../shaders/12_EffectPS.hlsl", 0, "main", "ps_4_0", &m_PS_Buffer);
	if (FAILED(hr))
		return false;

	hr = m_Device->CreateVertexShader(m_VS_Buffer->GetBufferPointer(), m_VS_Buffer->GetBufferSize(), nullptr, &m_VS);
	if (FAILED(hr))
		return false;

	hr = m_Device->CreatePixelShader(m_PS_Buffer->GetBufferPointer(), m_PS_Buffer->GetBufferSize(), nullptr, &m_PS);
	if (FAILED(hr))
		return false;

	m_DeviceContext->VSSetShader(m_VS, nullptr, 0);
	m_DeviceContext->PSSetShader(m_PS, nullptr, 0);

	// Cube geometry
	CubeVertex cubeVerts[] = {
		// Front Face
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3( 1.0f,  1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3( 1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },

		// Back Face
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3( 1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3( 1.0f,  1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.0f,  1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },

		// Top Face
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, 1.0f,  1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3( 1.0f, 1.0f,  1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3( 1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },

		// Bottom Face
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3( 1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3( 1.0f, -1.0f,  1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT2(1.0f, 0.0f) },

		// Left Face
		{ XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },

		// Right Face
		{ XMFLOAT3( 1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3( 1.0f,  1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3( 1.0f,  1.0f,  1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3( 1.0f, -1.0f,  1.0f), XMFLOAT2(1.0f, 1.0f) },
	};

	DWORD cubeIndices[] = {
		0, 1, 2, 0, 2, 3,
		4, 5, 6, 4, 6, 7,
		8, 9, 10, 8, 10, 11,
		12, 13, 14, 12, 14, 15,
		16, 17, 18, 16, 18, 19,
		20, 21, 22, 20, 22, 23
	};

	D3D11_BUFFER_DESC vbd = {};
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = sizeof(CubeVertex) * 24;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vinit = {};
	vinit.pSysMem = cubeVerts;
	hr = m_Device->CreateBuffer(&vbd, &vinit, &m_SquareVertBuffer);
	if (FAILED(hr))
		return false;

	D3D11_BUFFER_DESC ibd = {};
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = sizeof(DWORD) * 36;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA iinit = {};
	iinit.pSysMem = cubeIndices;
	hr = m_Device->CreateBuffer(&ibd, &iinit, &m_SquareIndexBuffer);
	if (FAILED(hr))
		return false;

	// Input layout + pipeline state
	hr = m_Device->CreateInputLayout(kLayout, kNumElements, m_VS_Buffer->GetBufferPointer(), m_VS_Buffer->GetBufferSize(), &m_VertLayout);
	if (FAILED(hr))
		return false;

	m_DeviceContext->IASetInputLayout(m_VertLayout);
	m_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (FLOAT)m_ClientWidth;
	viewport.Height = (FLOAT)m_ClientHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	m_DeviceContext->RSSetViewports(1, &viewport);

	D3D11_BUFFER_DESC cbbd = {};
	cbbd.Usage = D3D11_USAGE_DEFAULT;
	cbbd.ByteWidth = sizeof(cbPerObject);
	cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	hr = m_Device->CreateBuffer(&cbbd, nullptr, &m_CBPerObjectBuffer);
	if (FAILED(hr))
		return false;

	// Camera
	XMVECTOR camPosition = XMVectorSet(0.0f, 3.0f, -8.0f, 0.0f);
	XMVECTOR camTarget = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR camUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	m_CamView = XMMatrixLookAtLH(camPosition, camTarget, camUp);
	m_CamProjection = XMMatrixPerspectiveFovLH(0.4f * 3.14f, (FLOAT)m_ClientWidth / (FLOAT)m_ClientHeight, 1.0f, 1000.0f);

	// Blend state for overlay
	D3D11_BLEND_DESC blendDesc = {};
	D3D11_RENDER_TARGET_BLEND_DESC rtbd = {};
	rtbd.BlendEnable = true;
	rtbd.SrcBlend = D3D11_BLEND_ONE;
	rtbd.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	rtbd.BlendOp = D3D11_BLEND_OP_ADD;
	rtbd.SrcBlendAlpha = D3D11_BLEND_ONE;
	rtbd.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	rtbd.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[0] = rtbd;

	hr = m_Device->CreateBlendState(&blendDesc, &m_Transparency);
	if (FAILED(hr))
		return false;

	// Texture + sampler
	hr = CreateTextureFromFile(m_Device, L"../resource/seafloor.dds", &m_CubesTexture);
	if (FAILED(hr))
		return false;

	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = m_Device->CreateSamplerState(&sampDesc, &m_CubesTexSamplerState);
	if (FAILED(hr))
		return false;

	// Rasterizer
	D3D11_RASTERIZER_DESC cmdesc = {};
	cmdesc.FillMode = D3D11_FILL_SOLID;
	cmdesc.CullMode = D3D11_CULL_BACK;
	cmdesc.FrontCounterClockwise = true;
	hr = m_Device->CreateRasterizerState(&cmdesc, &m_CCWcullMode);
	if (FAILED(hr))
		return false;

	cmdesc.FrontCounterClockwise = false;
	hr = m_Device->CreateRasterizerState(&cmdesc, &m_CWcullMode);
	if (FAILED(hr))
		return false;

	// D2D screen quad + SRV
	// Quad in clip space (WVP = Identity)
	CubeVertex v[] = {
		{ XMFLOAT3(-1.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f,  1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3( 1.0f,  1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3( 1.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
	};

	DWORD indices[] = { 0, 1, 2, 0, 2, 3 };

	D3D11_BUFFER_DESC d2dIndexBufferDesc = {};
	d2dIndexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d2dIndexBufferDesc.ByteWidth = sizeof(DWORD) * 6;
	d2dIndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA d2dIinit = {};
	d2dIinit.pSysMem = indices;
	hr = m_Device->CreateBuffer(&d2dIndexBufferDesc, &d2dIinit, &m_D2DIndexBuffer);
	if (FAILED(hr))
		return false;

	D3D11_BUFFER_DESC d2dVertexBufferDesc = {};
	d2dVertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d2dVertexBufferDesc.ByteWidth = sizeof(CubeVertex) * 4;
	d2dVertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA d2dVinit = {};
	d2dVinit.pSysMem = v;
	hr = m_Device->CreateBuffer(&d2dVertexBufferDesc, &d2dVinit, &m_D2DVertBuffer);
	if (FAILED(hr))
		return false;

	hr = m_Device->CreateShaderResourceView(m_SharedTex11, nullptr, &m_D2DTexture);
	if (FAILED(hr))
		return false;

	return true;
}

void TutorialApp::RenderText(const wchar_t* text)
{
	// Unbind SRV before D2D draws into the texture
	ID3D11ShaderResourceView* nullSRV = nullptr;
	m_DeviceContext->PSSetShaderResources(0, 1, &nullSRV);

	m_D2DDeviceContext->BeginDraw();
	m_D2DDeviceContext->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f));

	if (m_TestTreeBitmap)
	{
		D2D1_SIZE_F size = m_TestTreeBitmap->GetSize();
		const float maxDim = 128.0f;
		const float sx = (size.width > 0.0f) ? (maxDim / size.width) : 1.0f;
		const float sy = (size.height > 0.0f) ? (maxDim / size.height) : 1.0f;
		const float scale = std::min(1.0f, std::min(sx, sy));
		const float drawW = std::min(size.width * scale, (float)m_ClientWidth);
		const float drawH = std::min(size.height * scale, (float)m_ClientHeight);
		D2D1_RECT_F dest = D2D1::RectF(10.0f, 10.0f, 10.0f + drawW, 10.0f + drawH);
		m_D2DDeviceContext->DrawBitmap(m_TestTreeBitmap, dest, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
	}

	m_Brush->SetColor(D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f));
	D2D1_RECT_F layoutRect = D2D1::RectF(0, 0, (FLOAT)m_ClientWidth, (FLOAT)m_ClientHeight);

	m_D2DDeviceContext->DrawText(
		text,
		static_cast<UINT32>(wcslen(text)),
		m_TextFormat,
		layoutRect,
		m_Brush);

	m_D2DDeviceContext->EndDraw();

	m_DeviceContext->OMSetBlendState(m_Transparency, nullptr, 0xffffffff);

	m_DeviceContext->IASetIndexBuffer(m_D2DIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	UINT stride = sizeof(CubeVertex);
	UINT offset = 0;
	m_DeviceContext->IASetVertexBuffers(0, 1, &m_D2DVertBuffer, &stride, &offset);

	m_WVP = XMMatrixIdentity();
	m_CBPerObj.WVP = XMMatrixTranspose(m_WVP);
	m_DeviceContext->UpdateSubresource(m_CBPerObjectBuffer, 0, nullptr, &m_CBPerObj, 0, 0);
	m_DeviceContext->VSSetConstantBuffers(0, 1, &m_CBPerObjectBuffer);
	m_DeviceContext->PSSetShaderResources(0, 1, &m_D2DTexture);
	m_DeviceContext->PSSetSamplers(0, 1, &m_CubesTexSamplerState);
	m_DeviceContext->RSSetState(m_CWcullMode);
	m_DeviceContext->DrawIndexed(6, 0, 0);

	// Restore opaque blend state for next frame
	m_DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
}

HRESULT TutorialApp::CreateTextureFromFile(ID3D11Device* d3dDevice, const wchar_t* szFileName, ID3D11ShaderResourceView** textureView)
{
	HRESULT hr = DirectX::CreateDDSTextureFromFile(d3dDevice, szFileName, nullptr, textureView);
	if (FAILED(hr))
	{
		hr = DirectX::CreateWICTextureFromFile(d3dDevice, szFileName, nullptr, textureView);
	}
	return hr;
}

HRESULT TutorialApp::CompileShaderFromFile(const WCHAR* szFileName, const D3D_SHADER_MACRO* pDefines, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
	dwShaderFlags |= D3DCOMPILE_DEBUG;
	dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ID3DBlob* pErrorBlob = nullptr;
	HRESULT hr = D3DCompileFromFile(szFileName, pDefines, D3D_COMPILE_STANDARD_FILE_INCLUDE, szEntryPoint, szShaderModel,
		dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			MessageBoxA(nullptr, (char*)pErrorBlob->GetBufferPointer(), "CompileShaderFromFile", MB_OK);
			pErrorBlob->Release();
		}
		return hr;
	}

	if (pErrorBlob)
		pErrorBlob->Release();

	return S_OK;
}
