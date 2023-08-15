#include "TutorialApp.h"
#include "../Common/Helper.h"
#include <d3dcompiler.h>
#include <Directxtk/DDSTextureLoader.h>

#pragma comment (lib, "d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")


// ���� ����.
struct Vertex
{
	Vector3 Pos;		// ���� ��ġ ����.
	Vector2 Tex;
};

struct CBNeverChanges
{
	Matrix mView;
};

struct CBChangeOnResize
{
	Matrix mProjection;
};

struct CBChangesEveryFrame
{
	Matrix mWorld;
	Vector4 vMeshColor;
};

TutorialApp::TutorialApp(HINSTANCE hInstance)
	:GameApp(hInstance)
{

}

TutorialApp::~TutorialApp()
{
	UninitScene();
	UninitD3D();
}

bool TutorialApp::Initialize(UINT Width, UINT Height)
{
	__super::Initialize(Width, Height);

	if (!InitD3D())
		return false;

	if (!InitScene())
		return false;

	return true;
}

void TutorialApp::Update()
{
	__super::Update();
	float t = GameTimer::m_Instance->TotalTime();
	// Rotate cube around the origin
	m_World = XMMatrixRotationY(t);

	// Modify the color
	m_vMeshColor.x = (sinf(t * 1.0f) + 1.0f) * 0.5f;
	m_vMeshColor.y = (cosf(t * 3.0f) + 1.0f) * 0.5f;
	m_vMeshColor.z = (sinf(t * 5.0f) + 1.0f) * 0.5f;
}

void TutorialApp::Render()
{
	float color[4] = { 0.0f, 0.5f, 0.5f, 1.0f };

	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, color);
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);


	// Render the cube
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_VertexBufferStride, &m_VertexBufferOffset);
	m_pDeviceContext->IASetInputLayout(m_pInputLayout);
	m_pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	m_pDeviceContext->VSSetShader(m_pVertexShader, nullptr, 0);
	m_pDeviceContext->VSSetConstantBuffers(0, 1, &m_pCBNeverChanges);
	m_pDeviceContext->VSSetConstantBuffers(1, 1, &m_pCBChangeOnResize);
	m_pDeviceContext->VSSetConstantBuffers(2, 1, &m_pCBChangesEveryFrame);
	m_pDeviceContext->PSSetShader(m_pPixelShader, nullptr, 0);
	m_pDeviceContext->PSSetConstantBuffers(2, 1, &m_pCBChangesEveryFrame);
	m_pDeviceContext->PSSetShaderResources(0, 1, &m_pTextureRV);
	m_pDeviceContext->PSSetSamplers(0, 1, &m_pSamplerLinear);

	// Update matrix variables and lighting variables
	CBChangesEveryFrame cb;
	cb.mWorld = XMMatrixTranspose(m_World);
	cb.vMeshColor = m_vMeshColor;
	m_pDeviceContext->UpdateSubresource(m_pCBChangesEveryFrame, 0, nullptr, &cb, 0, 0);

	m_pDeviceContext->DrawIndexed(m_nIndices, 0, 0);

	//
	// Present our back buffer to our front buffer
	//
	m_pSwapChain->Present(0, 0);
}

bool TutorialApp::InitD3D()
{
	HRESULT hr = 0;	// �����.

	// ����ü�� �Ӽ� ���� ����ü ����.
	DXGI_SWAP_CHAIN_DESC swapDesc = {};
	swapDesc.BufferCount = 1;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.OutputWindow = m_hWnd;	// ����ü�� ����� â �ڵ� ��.
	swapDesc.Windowed = true;		// â ��� ���� ����.
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
	// 1. ��ġ ����.   2.����ü�� ����. 3.��ġ ���ؽ�Ʈ ����.
	HR_T(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, creationFlags, NULL, NULL,
		D3D11_SDK_VERSION, &swapDesc, &m_pSwapChain, &m_pDevice, NULL, &m_pDeviceContext));

	// 4. ����Ÿ�ٺ� ����.  (����۸� �̿��ϴ� ����Ÿ�ٺ�)	
	ID3D11Texture2D* pBackBufferTexture = nullptr;
	HR_T(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBufferTexture));
	HR_T(m_pDevice->CreateRenderTargetView(pBackBufferTexture, NULL, &m_pRenderTargetView));  // �ؽ�ó�� ���� ���� ����
	SAFE_RELEASE(pBackBufferTexture);	//�ܺ� ���� ī��Ʈ�� ���ҽ�Ų��.
	// ���� Ÿ���� ���� ��� ���������ο� ���ε��մϴ�.
	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, NULL);

	//5. ����Ʈ ����.	
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)m_ClientWidth;
	viewport.Height = (float)m_ClientHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	m_pDeviceContext->RSSetViewports(1, &viewport);

	//6. �X��&���ٽ� �� ����
	D3D11_TEXTURE2D_DESC descDepth = {};
	descDepth.Width = m_ClientWidth;
	descDepth.Height = m_ClientHeight;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;

	ID3D11Texture2D* textureDepthStencil = nullptr;
	HR_T(m_pDevice->CreateTexture2D(&descDepth, nullptr, &textureDepthStencil));

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	HR_T(m_pDevice->CreateDepthStencilView(textureDepthStencil, &descDSV, &m_pDepthStencilView));
	SAFE_RELEASE(textureDepthStencil);

	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);
	return true;
}

void TutorialApp::UninitD3D()
{
	// Cleanup DirectX
	SAFE_RELEASE(m_pDevice);
	SAFE_RELEASE(m_pDeviceContext);
	SAFE_RELEASE(m_pSwapChain);
	SAFE_RELEASE(m_pRenderTargetView);
}

// 1. Render() ���� ���������ο� ���ε��� ���ؽ� ���۹� ���� ���� �غ�
// 2. Render() ���� ���������ο� ���ε��� InputLayout ���� 	
// 3. Render() ���� ���������ο� ���ε���  ���ؽ� ���̴� ����
// 4. Render() ���� ���������ο� ���ε��� �ε��� ���� ����
// 5. Render() ���� ���������ο� ���ε��� �ȼ� ���̴� ����
// 6. Render() ���� ���������ο� ���ε��� ��� ���� ����
// 7. Render() ���� ���������ο� ���ε��� ���̴� ���ҽ��� ���÷� ����
bool TutorialApp::InitScene()
{
	HRESULT hr=0; // �����.
	// 1. Render() ���� ���������ο� ���ε��� ���ؽ� ���۹� ���� ���� �غ�
	// Local or Object or Model Space
	Vertex vertices[] =
	{
		{ Vector3(-1.0f, 1.0f, -1.0f), Vector2(1.0f, 0.0f) },
		{ Vector3(1.0f, 1.0f, -1.0f), Vector2(0.0f, 0.0f) },
		{ Vector3(1.0f, 1.0f, 1.0f), Vector2(0.0f, 1.0f) },
		{ Vector3(-1.0f, 1.0f, 1.0f), Vector2(1.0f, 1.0f) },

		{ Vector3(-1.0f, -1.0f, -1.0f), Vector2(0.0f, 0.0f) },
		{ Vector3(1.0f, -1.0f, -1.0f), Vector2(1.0f, 0.0f) },
		{ Vector3(1.0f, -1.0f, 1.0f), Vector2(1.0f, 1.0f) },
		{ Vector3(-1.0f, -1.0f, 1.0f), Vector2(0.0f, 1.0f) },

		{ Vector3(-1.0f, -1.0f, 1.0f), Vector2(0.0f, 1.0f) },
		{ Vector3(-1.0f, -1.0f, -1.0f), Vector2(1.0f, 1.0f) },
		{ Vector3(-1.0f, 1.0f, -1.0f), Vector2(1.0f, 0.0f) },
		{ Vector3(-1.0f, 1.0f, 1.0f), Vector2(0.0f, 0.0f) },

		{ Vector3(1.0f, -1.0f, 1.0f), Vector2(1.0f, 1.0f) },
		{ Vector3(1.0f, -1.0f, -1.0f), Vector2(0.0f, 1.0f) },
		{ Vector3(1.0f, 1.0f, -1.0f), Vector2(0.0f, 0.0f) },
		{ Vector3(1.0f, 1.0f, 1.0f), Vector2(1.0f, 0.0f) },

		{ Vector3(-1.0f, -1.0f, -1.0f), Vector2(0.0f, 1.0f) },
		{ Vector3(1.0f, -1.0f, -1.0f), Vector2(1.0f, 1.0f) },
		{ Vector3(1.0f, 1.0f, -1.0f), Vector2(1.0f, 0.0f) },
		{ Vector3(-1.0f, 1.0f, -1.0f), Vector2(0.0f, 0.0f) },

		{ Vector3(-1.0f, -1.0f, 1.0f), Vector2(1.0f, 1.0f) },
		{ Vector3(1.0f, -1.0f, 1.0f), Vector2(0.0f, 1.0f) },
		{ Vector3(1.0f, 1.0f, 1.0f), Vector2(0.0f, 0.0f) },
		{ Vector3(-1.0f, 1.0f, 1.0f), Vector2(1.0f, 0.0f) },
	};
	D3D11_BUFFER_DESC bd = {};
	bd.ByteWidth = sizeof(Vertex) * ARRAYSIZE(vertices);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA vbData = {};
	vbData.pSysMem = vertices;
	HR_T( m_pDevice->CreateBuffer(&bd, &vbData, &m_pVertexBuffer));
		
	m_VertexBufferStride = sizeof(Vertex);	// ���ؽ� ���� ����
	m_VertexBufferOffset = 0;

	// 2. Render() ���� ���������ο� ���ε��� InputLayout ���� 	
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	ID3DBlob* vertexShaderBuffer = nullptr;
	HR_T(CompileShaderFromFile(L"BasicVertexShader.hlsl", "main", "vs_4_0", &vertexShaderBuffer));
	HR_T(m_pDevice->CreateInputLayout(layout, ARRAYSIZE(layout),
		vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_pInputLayout));

	// 3. Render() ���� ���������ο� ���ε���  ���ؽ� ���̴� ����
	HR_T(m_pDevice->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), NULL, &m_pVertexShader));
	SAFE_RELEASE(vertexShaderBuffer);

	// 4. Render() ���� ���������ο� ���ε��� �ε��� ���� ����
	WORD indices[] =
	{
		3,1,0, 2,1,3,
		6,4,5, 7,4,6,
		11,9,8, 10,9,11,
		14,12,13, 15,12,14,
		19,17,16, 18,17,19,
		22,20,21, 23,20,22
	};	
	m_nIndices = ARRAYSIZE(indices);	// �ε��� ���� ����.
	bd = {};
	bd.ByteWidth = sizeof(WORD) * ARRAYSIZE(indices);
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA ibData = {};
	ibData.pSysMem = indices;
	HR_T(m_pDevice->CreateBuffer(&bd, &ibData, &m_pIndexBuffer));


	// 5. Render() ���� ���������ο� ���ε��� �ȼ� ���̴� ����
	ID3D10Blob* pixelShaderBuffer = nullptr;
	HR_T(CompileShaderFromFile(L"BasicPixelShader.hlsl", "main", "ps_4_0", &pixelShaderBuffer));
	HR_T(m_pDevice->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
		pixelShaderBuffer->GetBufferSize(), NULL, &m_pPixelShader));
	SAFE_RELEASE(pixelShaderBuffer);

	// 6. Render() ���� ���������ο� ���ε��� ��� ���� ����
	bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(CBNeverChanges);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	HR_T( m_pDevice->CreateBuffer(&bd, nullptr,&m_pCBNeverChanges));

	bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(CBChangeOnResize);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	HR_T(m_pDevice->CreateBuffer(&bd, nullptr, &m_pCBChangeOnResize));

	bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(CBChangesEveryFrame);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	HR_T(m_pDevice->CreateBuffer(&bd, nullptr, &m_pCBChangesEveryFrame));


	// 7. Render() ���� ���������ο� ���ε��� ���̴� ���ҽ��� ���÷� ����
	HR_T( CreateDDSTextureFromFile(m_pDevice, L"seafloor.dds", nullptr, &m_pTextureRV));
	
	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	HR_T( m_pDevice->CreateSamplerState(&sampDesc, &m_pSamplerLinear));

	// World,View,Projection �ʱⰪ ����
	m_World = XMMatrixIdentity();

	XMVECTOR Eye = XMVectorSet(0.0f, 3.0f, -6.0f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	m_View = XMMatrixLookAtLH(Eye, At, Up);
	m_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, m_ClientWidth / (FLOAT)m_ClientHeight, 0.01f, 100.0f);
	
	
	// Constant Buffer �ʱ�ȭ
	CBNeverChanges cbNeverChanges;
	cbNeverChanges.mView = XMMatrixTranspose(m_View);
	m_pDeviceContext->UpdateSubresource(m_pCBNeverChanges, 0, nullptr, &cbNeverChanges, 0, 0);
		
	CBChangeOnResize cbChangesOnResize;
	cbChangesOnResize.mProjection = XMMatrixTranspose(m_Projection);
	m_pDeviceContext->UpdateSubresource(m_pCBChangeOnResize, 0, nullptr, &cbChangesOnResize, 0, 0);
	return true;
}

void TutorialApp::UninitScene()
{
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pVertexShader);
	SAFE_RELEASE(m_pPixelShader);
	SAFE_RELEASE(m_pInputLayout);
	SAFE_RELEASE(m_pIndexBuffer);
	SAFE_RELEASE(m_pDepthStencilView);
}
