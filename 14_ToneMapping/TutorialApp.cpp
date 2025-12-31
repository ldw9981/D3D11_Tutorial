#include "TutorialApp.h"
#include "../Common/Helper.h"
#include <d3dcompiler.h>
#include <vector>

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <dxgi1_4.h>	// swapchain3
#include <dxgi1_6.h>	// swapchain3

#pragma comment (lib, "d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")


using namespace Microsoft::WRL;

struct ConstantBuffer
{
	Matrix mWorld;
	Matrix mView;
	Matrix mProjection;

	Vector4 vLightDir[2];
	Vector4 vLightColor[2];
	Vector4 vOutputColor;
	float monitorMaxNits=100.0f;
	float gExposure=1.0f;
	int gUseWideGamut=0;		// 넓은 색역 사용 여부
	int gUseToneMapping=1;		// 톤매핑 적용 여부
	float gReferenceWhiteNit=230.0f; // SDR 기준 화이트 포인트 (nits)
	float padding1;
	float padding2;
	float padding3;
};

bool TutorialApp::OnInitialize()
{
	if (!InitD3D())
		return false;

	if (!InitScene())
		return false;

	if (!InitImGUI())
		return false;

	return true;
}

void TutorialApp::OnUninitialize()
{
	UninitImGUI();
	UninitScene();
	UninitD3D();
}

void TutorialApp::OnUpdate()
{
	float t = XMConvertToRadians(m_rotationAngle);	
	
	m_LightDirsEvaluated[0] = m_InitialLightDirs[0];

	// Rotate the second light around the origin
	XMMATRIX mRotate = XMMatrixRotationY(t);
	XMVECTOR vLightDir = XMLoadFloat4(&m_InitialLightDirs[1]);
	vLightDir = XMVector3Transform(vLightDir, mRotate);
	XMStoreFloat4(&m_LightDirsEvaluated[1], vLightDir);


	m_Camera.GetViewMatrix(m_View);
}

void TutorialApp::OnRender()
{
	float color[4] = { 0.0f, 0.5f, 0.5f, 1.0f };
	
	m_pDeviceContext->ClearRenderTargetView(m_pHdrRenderTargetView, color);
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	m_pDeviceContext->OMSetRenderTargets(1, &m_pHdrRenderTargetView, m_pDepthStencilView);

	// Update matrix variables and lighting variables
	ConstantBuffer cb1;
	cb1.mWorld = XMMatrixTranspose(m_World);
	cb1.mView = XMMatrixTranspose(m_View);
	cb1.mProjection = XMMatrixTranspose(m_Projection);
	cb1.vLightDir[0] = m_LightDirsEvaluated[0];
	cb1.vLightDir[1] = m_LightDirsEvaluated[1];
	cb1.vLightColor[0] = Vector4(m_LightColors[0]) * m_LightIntensity[0];
	cb1.vLightColor[0].w = 0.0f;

	cb1.vLightColor[1] = Vector4(m_LightColors[1]) * m_LightIntensity[1];
	cb1.vLightColor[1].w = 0.0f;
	cb1.vOutputColor = XMFLOAT4(0, 0, 0, 0);
	cb1.monitorMaxNits = m_MonitorMaxNits;
	cb1.gExposure = m_Exposure;
	cb1.gUseWideGamut = (m_CurrColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020) ?  1 : 0;	// 넓은 색역 사용 여부 전달
	cb1.gUseToneMapping = m_UseToneMapping ? 1 : 0;	// 톤매핑 적용 여부 전달
	cb1.gReferenceWhiteNit = m_ReferenceWhiteNit;	// 기준 화이트 포인트 전달

	m_pDeviceContext->UpdateSubresource(m_pLightConstantBuffer, 0, nullptr, &cb1, 0, 0);

	
	// Render cube to HDR render target
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pDeviceContext->IASetVertexBuffers(0, 1, &m_pCubeVertexBuffer, &m_CubeVertexBufferStride, &m_CubeVertexBufferOffset);
	m_pDeviceContext->IASetInputLayout(m_pCubeInputLayout);
	m_pDeviceContext->IASetIndexBuffer(m_pCubeIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	m_pDeviceContext->VSSetShader(m_pCubeVertexShader, nullptr, 0);
	m_pDeviceContext->VSSetConstantBuffers(0, 1, &m_pLightConstantBuffer);
	m_pDeviceContext->PSSetShader(m_pCubePixelShader, nullptr, 0);
	m_pDeviceContext->PSSetConstantBuffers(0, 1, &m_pLightConstantBuffer);
	m_pDeviceContext->DrawIndexed(m_nCubeIndices, 0, 0);	

	// Render each light	
	for (int m = 0; m < 2; m++)
	{
		XMMATRIX mLight = XMMatrixTranslationFromVector(5.0f * XMLoadFloat4(&m_LightDirsEvaluated[m]));
		XMMATRIX mLightScale = XMMatrixScaling(0.2f, 0.2f, 0.2f);
		mLight = mLightScale * mLight;

		// Update the world variable to reflect the current light
		cb1.mWorld = XMMatrixTranspose(mLight);
		cb1.vOutputColor = cb1.vLightColor[m];

		m_pDeviceContext->UpdateSubresource(m_pLightConstantBuffer, 0, nullptr, &cb1, 0, 0);

		m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_pDeviceContext->IASetVertexBuffers(0, 1, &m_pCubeVertexBuffer, &m_CubeVertexBufferStride, &m_CubeVertexBufferOffset);
		m_pDeviceContext->IASetInputLayout(m_pCubeInputLayout);
		m_pDeviceContext->IASetIndexBuffer(m_pCubeIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
		m_pDeviceContext->VSSetShader(m_pCubeVertexShader, nullptr, 0);
		m_pDeviceContext->VSSetConstantBuffers(0, 1, &m_pLightConstantBuffer);
		m_pDeviceContext->PSSetShader(m_pSolidPixelShader, nullptr, 0);
		m_pDeviceContext->PSSetConstantBuffers(0, 1, &m_pLightConstantBuffer);
		m_pDeviceContext->DrawIndexed(m_nCubeIndices, 0, 0);
	}


	// Render quad to back buffer with tone mapping	
	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, nullptr);
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, color);
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pDeviceContext->IASetVertexBuffers(0, 1, &m_pQuadVertexBuffer, &m_QuadVertexBufferStride, &m_QuadVertexBufferOffset);
	m_pDeviceContext->IASetInputLayout(m_pQuadInputLayout);
	m_pDeviceContext->IASetIndexBuffer(m_pQuadIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	m_pDeviceContext->VSSetShader(m_pQuadVertexShader, nullptr, 0);
	
	switch (m_CurrFormat)
	{	
	case DXGI_FORMAT_R10G10B10A2_UNORM:
		m_pDeviceContext->PSSetShader(m_pPS_ToneMappingHDR, nullptr, 0);
		break;
	default:
		m_pDeviceContext->PSSetShader(m_pPS_ToneMappingLDR, nullptr, 0);
		break;
	}

	

	m_pDeviceContext->PSSetShaderResources(0, 1, &m_pHdrShaderResourceView);
	m_pDeviceContext->PSSetSamplers(0, 1, &m_pSamplerLinear);
	m_pDeviceContext->DrawIndexed(m_nQuadIndices, 0, 0);
	// Unbind the HDR texture from the pixel shader
	ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
	m_pDeviceContext->PSSetShaderResources(0, 1, nullSRV);




	RenderImGUI();

	m_pSwapChain->Present(0, 0);
}

bool TutorialApp::InitD3D()
{
	DXGI_FORMAT format;
	DXGI_COLOR_SPACE_TYPE resultColorSpace;
	m_isHDRSupported = CheckHDRSupport(m_MonitorMaxNits, format, resultColorSpace);

	if(m_isHDRSupported && !m_forceLDR)
		CreateSwapChainAndBackBuffer(DXGI_FORMAT_R10G10B10A2_UNORM); // HDR
	else
		CreateSwapChainAndBackBuffer(DXGI_FORMAT_R8G8B8A8_UNORM); // LDR

	// Color Space 선택
	if (m_forceP709)
		resultColorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;
	SelectColorSpace(resultColorSpace);
	

	// 렌더 타겟을 최종 출력 파이프라인에 바인딩합니다.
	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, NULL);

	//5. 뷰포트 설정.	
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)m_ClientWidth;
	viewport.Height = (float)m_ClientHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	m_pDeviceContext->RSSetViewports(1, &viewport);

	//6. 뎊스&스텐실 뷰 생성
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
	ComPtr<ID3D11Texture2D> depthStencilTex;
	HR_T(m_pDevice->CreateTexture2D(&descDepth, nullptr, depthStencilTex.GetAddressOf()));	
	
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	HR_T(m_pDevice->CreateDepthStencilView(depthStencilTex.Get(), &descDSV, &m_pDepthStencilView));

	
	//Create HDR Render Target and its view
	D3D11_TEXTURE2D_DESC td = {};
	td.Width = static_cast<UINT>(m_ClientWidth);
	td.Height = static_cast<UINT>(m_ClientHeight);
	td.MipLevels = 1;
	td.ArraySize = 1;
	td.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	td.SampleDesc.Count = 1;   // MSAA 없음
	td.SampleDesc.Quality = 0;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	HR_T(m_pDevice->CreateTexture2D(&td, nullptr, &m_pHdrRenderTarget));
	HR_T(m_pDevice->CreateRenderTargetView(m_pHdrRenderTarget, nullptr, &m_pHdrRenderTargetView));
	HR_T(m_pDevice->CreateShaderResourceView(m_pHdrRenderTarget, nullptr, &m_pHdrShaderResourceView));
	

	// Create the sample state
	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	HR_T(m_pDevice->CreateSamplerState(&sampDesc, &m_pSamplerLinear));
	return true;
}

//  DXGI_FORMAT_R8G8B8A8_UNORM : LDR
//  DXGI_FORMAT_R10G10B10A2_UNORM   : HDR
void TutorialApp::CreateSwapChainAndBackBuffer(DXGI_FORMAT format)
{
	m_CurrFormat = format;	

	HRESULT hr = 0;	// 결과값.

	// 스왑체인 속성 설정 구조체 생성.
	DXGI_SWAP_CHAIN_DESC swapDesc = {};
	swapDesc.BufferCount = 2;
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.OutputWindow = m_hWnd;	// 스왑체인 출력할 창 핸들 값.
	swapDesc.Windowed = true;		// 창 모드 여부 설정.
	swapDesc.BufferDesc.Format = m_CurrFormat;
	// 백버퍼(텍스처)의 가로/세로 크기 설정.
	swapDesc.BufferDesc.Width = m_ClientWidth;
	swapDesc.BufferDesc.Height = m_ClientHeight;
	// 화면 주사율 설정.
	swapDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapDesc.BufferDesc.RefreshRate.Denominator = 1;
	// 샘플링 관련 설정.
	swapDesc.SampleDesc.Count = 1;
	swapDesc.SampleDesc.Quality = 0;

	UINT creationFlags = 0;
#ifdef _DEBUG
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	HR_T(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, creationFlags, NULL, NULL,
		D3D11_SDK_VERSION, &swapDesc, &m_pSwapChain, &m_pDevice, NULL, &m_pDeviceContext));

	// 4. 렌더타겟뷰 생성.  (백버퍼를 이용하는 렌더타겟뷰)	
	ComPtr<ID3D11Texture2D> backBufferTexture;		
	HR_T(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)backBufferTexture.GetAddressOf()));
	HR_T(m_pDevice->CreateRenderTargetView(backBufferTexture.Get(), NULL, &m_pRenderTargetView));  // 텍스처는 내부 참조 증가

}

bool TutorialApp::CheckHDRSupport(float& outMaxNits, DXGI_FORMAT& outFormat, DXGI_COLOR_SPACE_TYPE& outColorSpace)
{
	ComPtr<IDXGIFactory4> pFactory;
	HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&pFactory));
	if (FAILED(hr))
	{
		LOG_ERRORA("ERROR: DXGI Factory 생성 실패.\n");
		return false;
	}
	// 2. 주 그래픽 어댑터 (0번) 열거
	ComPtr<IDXGIAdapter1> pAdapter;
	UINT adapterIndex = 0;
	while (pFactory->EnumAdapters1(adapterIndex, &pAdapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC1 desc;
		pAdapter->GetDesc1(&desc);

		// WARP 어댑터(소프트웨어)를 건너뛰고 주 어댑터만 사용하도록 선택할 수 있습니다.
		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			adapterIndex++;
			pAdapter.Reset();
			continue;
		}
		break;
	}

	if (!pAdapter)
	{
		LOG_ERRORA("ERROR: 유효한 하드웨어 어댑터를 찾을 수 없습니다.\n");
		return false;
	}	

	// 3. 주 모니터 출력 (0번) 열거
	ComPtr<IDXGIOutput> pOutput;
	hr = pAdapter->EnumOutputs(0, &pOutput); // 0번 출력
	if (FAILED(hr))
	{
		LOG_ERRORA("ERROR: 주 모니터 출력(Output 0)을 찾을 수 없습니다.\n");
		return false;
	}

	// 4. HDR 정보를 얻기 위해 IDXGIOutput6으로 쿼리
	ComPtr<IDXGIOutput6> pOutput6;
	hr = pOutput.As(&pOutput6);
	if (FAILED(hr))
	{
		printf("INFO: IDXGIOutput6 인터페이스를 얻을 수 없습니다. HDR 정보를 얻을 수 없습니다.\n");
		outMaxNits = 100.0f;
		outFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		return false;
	}

	// 5. DXGI_OUTPUT_DESC1에서 HDR 정보 확인
	DXGI_OUTPUT_DESC1 desc1 = {};
	hr = pOutput6->GetDesc1(&desc1);
	if (FAILED(hr))
	{
		printf("ERROR: GetDesc1 호출 실패.\n");
		return false;
	}

	// 모니터의 색역 정보 저장 (CIE xy 좌표)
	m_RedPrimary[0] = desc1.RedPrimary[0];
	m_RedPrimary[1] = desc1.RedPrimary[1];
	m_GreenPrimary[0] = desc1.GreenPrimary[0];
	m_GreenPrimary[1] = desc1.GreenPrimary[1];
	m_BluePrimary[0] = desc1.BluePrimary[0];
	m_BluePrimary[1] = desc1.BluePrimary[1];

	// HDR 색 공간 활성화 검사
	bool isHDRColorSpace = (desc1.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020);
	outMaxNits = (float)desc1.MaxLuminance ;

	// OS가 HDR을 켰을 때 MaxLuminance는 100 Nits(SDR 기준)를 초과합니다.
	bool isHDRActive = outMaxNits > 100.0f;

	// 모니터 색역 분석 및 판단
	// 참고 색역 좌표:
	// Rec.709:  Red(0.640, 0.330), Green(0.300, 0.600), Blue(0.150, 0.060)
	// DCI-P3:   Red(0.680, 0.320), Green(0.265, 0.690), Blue(0.150, 0.060)
	// Rec.2020: Red(0.708, 0.292), Green(0.170, 0.797), Blue(0.131, 0.046)

	float redX = m_RedPrimary[0];
	float greenY = m_GreenPrimary[1];

	// 넓은 색역 판단: DCI-P3 수준 이상 (Red Primary x > 0.66 또는 Green Primary y > 0.68)
	m_isWideGamutSupported = (redX > 0.66f) || (greenY > 0.68f);

	// Wide Gamut 여부에 따라 Reference White Nit 설정
	m_ReferenceWhiteNit = m_isWideGamutSupported ? 400.0f : 230.0f;

	if (isHDRColorSpace && isHDRActive)
	{
		// 최종 판단: HDR 지원 및 OS 활성화
		outFormat = DXGI_FORMAT_R10G10B10A2_UNORM; // HDR 포맷 설정

		printf("========== Monitor Color Gamut Info ==========\n");
		printf("MaxLuminance: %.1f nits\n", outMaxNits);
		printf("MinLuminance: %.4f nits\n", desc1.MinLuminance);
		printf("\n[Color Primaries (CIE xy)]\n");
		printf("  Red   Primary: (%.4f, %.4f)\n", m_RedPrimary[0], m_RedPrimary[1]);
		printf("  Green Primary: (%.4f, %.4f)\n", m_GreenPrimary[0], m_GreenPrimary[1]);
		printf("  Blue  Primary: (%.4f, %.4f)\n", m_BluePrimary[0], m_BluePrimary[1]);
		printf("\n[Reference Standards]\n");
		printf("  Rec.709:  Red(0.640, 0.330), Green(0.300, 0.600)\n");
		printf("  DCI-P3:   Red(0.680, 0.320), Green(0.265, 0.690)\n");
		printf("  Rec.2020: Red(0.708, 0.292), Green(0.170, 0.797)\n");
		printf("\n[Analysis]\n");

		if (m_isWideGamutSupported)
		{
			printf("  Color Gamut: WIDE GAMUT (DCI-P3 or wider)\n");
			printf("  Recommendation: Rec.2020 색 공간 사용 가능\n");
			outColorSpace = DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
		}
		else
		{
			printf("  Color Gamut: STANDARD GAMUT (sRGB/Rec.709)\n");
			printf("  Recommendation: Rec.709 유지 권장 (색 왜곡 방지)\n");
			outColorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;
		}

		printf("\nFormat: R10G10B10A2_UNORM (HDR)\n");
		printf("==============================================\n");

		return true;
	}
	else
	{
		// HDR 지원 안함 또는 OS에서 비활성화
		outMaxNits = 100.0f; // SDR 기본값
		outFormat = DXGI_FORMAT_R8G8B8A8_UNORM; // SDR 포맷 설정
		outColorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;
		m_isWideGamutSupported = false;
		m_ReferenceWhiteNit = 230.0f; // SDR 기본값

		printf("INFO: HDR 비활성화됨\n");
		printf("  MaxNits: 100.0 (SDR 기본값)\n");
		printf("  Format: R8G8B8A8_UNORM (LDR)\n");

		return false;
	}
	return true;
}

void TutorialApp::SelectColorSpace(DXGI_COLOR_SPACE_TYPE colorSpace)
{
	m_CurrColorSpace = colorSpace;
	

	ComPtr<IDXGISwapChain3> swapChain3;
	HR_T(m_pSwapChain->QueryInterface(__uuidof(IDXGISwapChain3), (void**)&swapChain3));
	HR_T(swapChain3->SetColorSpace1(m_CurrColorSpace));
}

void TutorialApp::UninitD3D()
{
	SAFE_RELEASE(m_pHdrShaderResourceView);
	SAFE_RELEASE(m_pHdrRenderTargetView);
	SAFE_RELEASE(m_pHdrRenderTarget);
		
	SAFE_RELEASE(m_pDepthStencilView);
	SAFE_RELEASE(m_pRenderTargetView);
	SAFE_RELEASE(m_pDevice);
	SAFE_RELEASE(m_pDeviceContext);
	SAFE_RELEASE(m_pSwapChain);
	
}

bool TutorialApp::InitScene()
{
	HRESULT hr=0; // 결과값.
	ID3D10Blob* errorMessage = nullptr;	 // 에러 메시지를 저장할 버퍼.		
	
	// 라이트 상수 버퍼 생성	
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	HR_T(m_pDevice->CreateBuffer(&bd, nullptr, &m_pLightConstantBuffer));



	CreateCube();
	CreateQuad();

	// FOV 초기값설정
	m_World = XMMatrixIdentity();
	XMVECTOR Eye = XMVectorSet(0.0f, 4.0f, -10.0f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	m_View = XMMatrixLookAtLH(Eye, At, Up);
	m_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, m_ClientWidth / (FLOAT)m_ClientHeight, 1.0f, 10000.0f);
	return true;
}

void TutorialApp::UninitScene()
{
	SAFE_RELEASE(m_pLightConstantBuffer);

	SAFE_RELEASE(m_pCubeVertexBuffer);
	SAFE_RELEASE(m_pCubeVertexShader);
	SAFE_RELEASE(m_pCubePixelShader);
	SAFE_RELEASE(m_pCubeInputLayout);
	SAFE_RELEASE(m_pCubeIndexBuffer);

	SAFE_RELEASE(m_pQuadVertexBuffer);
	SAFE_RELEASE(m_pQuadVertexShader);
	SAFE_RELEASE(m_pPS_ToneMappingHDR);
	SAFE_RELEASE(m_pPS_ToneMappingLDR);
	SAFE_RELEASE(m_pQuadInputLayout);
	SAFE_RELEASE(m_pQuadIndexBuffer);
	
}

void TutorialApp::CreateQuad()
{
	HRESULT hr = 0; // 결과값.
	ID3D10Blob* errorMessage = nullptr;	 // 에러 메시지를 저장할 버퍼.	
	// 정점 선언.
	struct QuadVertex
	{
		Vector3 position;		// Normalized Device coordinate position
		Vector2 uv;				// Texture coordinate position

		QuadVertex(float x, float y, float z ,float u,float v) : position(x, y, z), uv(u,v) {}	
		QuadVertex(Vector3 p, Vector2 u) : position(p), uv(u) { }
	};

	QuadVertex QuadVertices[] =
	{
		QuadVertex(Vector3(-1.0f,  1.0f, 1.0f), Vector2(0.0f,0.0f)),	// Left Top 
		QuadVertex(Vector3(1.0f,  1.0f, 1.0f), Vector2(1.0f, 0.0f)),	// Right Top
		QuadVertex(Vector3(-1.0f, -1.0f, 1.0f), Vector2(0.0f, 1.0f)),	// Left Bottom
		QuadVertex(Vector3(1.0f, -1.0f, 1.0f), Vector2(1.0f, 1.0f))		// Right Bottom
	};

	D3D11_BUFFER_DESC vbDesc = {};
	vbDesc.ByteWidth = sizeof(QuadVertex) * ARRAYSIZE(QuadVertices);
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.Usage = D3D11_USAGE_DEFAULT;
	D3D11_SUBRESOURCE_DATA vbData = {};
	vbData.pSysMem = QuadVertices;	// 배열 데이터 할당.
	HR_T(m_pDevice->CreateBuffer(&vbDesc, &vbData, &m_pQuadVertexBuffer));
	m_QuadVertexBufferStride = sizeof(QuadVertex);		// 버텍스 버퍼 정보
	m_QuadVertexBufferOffset = 0;
	
	// InputLayout 생성 	
	D3D11_INPUT_ELEMENT_DESC layout[] = // 입력 레이아웃.
	{   // SemanticName , SemanticIndex , Format , InputSlot , AlignedByteOffset , InputSlotClass , InstanceDataStepRate	
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	ID3D10Blob* vertexShaderBuffer = nullptr;
	HR_T(CompileShaderFromFile(L"../Shaders/14_QuadVS.hlsl", "main", "vs_4_0", &vertexShaderBuffer));
	HR_T(m_pDevice->CreateInputLayout(layout, ARRAYSIZE(layout),
		vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_pQuadInputLayout));

	// 버텍스 셰이더 생성
	HR_T(m_pDevice->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), NULL, &m_pQuadVertexShader));
	SAFE_RELEASE(vertexShaderBuffer);	// 버퍼 해제.

	// 인덱스 버퍼 생성
	WORD indices[] =
	{
		0, 1, 2,
		2, 1, 3
	};
	m_nQuadIndices = ARRAYSIZE(indices);	// 인덱스 개수 저장.
	D3D11_BUFFER_DESC ibDesc = {};
	ibDesc.ByteWidth = sizeof(WORD) * ARRAYSIZE(indices);
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibDesc.Usage = D3D11_USAGE_DEFAULT;
	D3D11_SUBRESOURCE_DATA ibData = {};
	ibData.pSysMem = indices;
	HR_T(m_pDevice->CreateBuffer(&ibDesc, &ibData, &m_pQuadIndexBuffer));

	// 픽셀 셰이더 생성
	ID3D10Blob* pixelShaderBuffer = nullptr;


	HR_T(CompileShaderFromFile(L"../Shaders/14_ToneMappingPS_LDR.hlsl", "main", "ps_4_0", &pixelShaderBuffer));
	HR_T(m_pDevice->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
		pixelShaderBuffer->GetBufferSize(), NULL, &m_pPS_ToneMappingLDR));
	SAFE_RELEASE(pixelShaderBuffer);	// 픽셀 셰이더 버퍼 더이상 필요없음.

	HR_T(CompileShaderFromFile(L"../Shaders/14_ToneMappingPS_HDR.hlsl", "main", "ps_4_0", &pixelShaderBuffer));
	HR_T(m_pDevice->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
		pixelShaderBuffer->GetBufferSize(), NULL, &m_pPS_ToneMappingHDR));
	SAFE_RELEASE(pixelShaderBuffer);	// 픽셀 셰이더 버퍼 더이상 필요없음.
}

void TutorialApp::CreateCube()
{
	// 정점 선언.
	struct CubeVertex
	{
		Vector3 Pos;		// 정점 위치 정보.
		Vector3 Normal;
	};

	HRESULT hr = 0; // 결과값.
	ID3D10Blob* errorMessage = nullptr;	 // 에러 메시지를 저장할 버퍼.
	CubeVertex vertices[] =
	{
		{ Vector3(-1.0f, 1.0f, -1.0f),	Vector3(0.0f, 1.0f, 0.0f) },// Normal Y +
		{ Vector3(1.0f, 1.0f, -1.0f),	Vector3(0.0f, 1.0f, 0.0f) },
		{ Vector3(1.0f, 1.0f, 1.0f),	Vector3(0.0f, 1.0f, 0.0f) },
		{ Vector3(-1.0f, 1.0f, 1.0f),	Vector3(0.0f, 1.0f, 0.0f) },

		{ Vector3(-1.0f, -1.0f, -1.0f), Vector3(0.0f, -1.0f, 0.0f) },// Normal Y -
		{ Vector3(1.0f, -1.0f, -1.0f),	Vector3(0.0f, -1.0f, 0.0f) },
		{ Vector3(1.0f, -1.0f, 1.0f),	Vector3(0.0f, -1.0f, 0.0f) },
		{ Vector3(-1.0f, -1.0f, 1.0f),	Vector3(0.0f, -1.0f, 0.0f) },

		{ Vector3(-1.0f, -1.0f, 1.0f),	Vector3(-1.0f, 0.0f, 0.0f) },//	Normal X -
		{ Vector3(-1.0f, -1.0f, -1.0f), Vector3(-1.0f, 0.0f, 0.0f) },
		{ Vector3(-1.0f, 1.0f, -1.0f),	Vector3(-1.0f, 0.0f, 0.0f) },
		{ Vector3(-1.0f, 1.0f, 1.0f),	Vector3(-1.0f, 0.0f, 0.0f) },

		{ Vector3(1.0f, -1.0f, 1.0f),	Vector3(1.0f, 0.0f, 0.0f) },// Normal X +
		{ Vector3(1.0f, -1.0f, -1.0f),	Vector3(1.0f, 0.0f, 0.0f) },
		{ Vector3(1.0f, 1.0f, -1.0f),	Vector3(1.0f, 0.0f, 0.0f) },
		{ Vector3(1.0f, 1.0f, 1.0f),	Vector3(1.0f, 0.0f, 0.0f) },

		{ Vector3(-1.0f, -1.0f, -1.0f), Vector3(0.0f, 0.0f, -1.0f) }, // Normal Z -
		{ Vector3(1.0f, -1.0f, -1.0f),	Vector3(0.0f, 0.0f, -1.0f) },
		{ Vector3(1.0f, 1.0f, -1.0f),	Vector3(0.0f, 0.0f, -1.0f) },
		{ Vector3(-1.0f, 1.0f, -1.0f),	Vector3(0.0f, 0.0f, -1.0f) },

		{ Vector3(-1.0f, -1.0f, 1.0f),	Vector3(0.0f, 0.0f, 1.0f) },// Normal Z +
		{ Vector3(1.0f, -1.0f, 1.0f),	Vector3(0.0f, 0.0f, 1.0f) },
		{ Vector3(1.0f, 1.0f, 1.0f),	Vector3(0.0f, 0.0f, 1.0f) },
		{ Vector3(-1.0f, 1.0f, 1.0f),	Vector3(0.0f, 0.0f, 1.0f) },
	};

	// 버텍스 버퍼 생성.
	D3D11_BUFFER_DESC bd = {};
	bd.ByteWidth = sizeof(CubeVertex) * ARRAYSIZE(vertices);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA vbData = {};
	vbData.pSysMem = vertices;
	HR_T(m_pDevice->CreateBuffer(&bd, &vbData, &m_pCubeVertexBuffer));

	// 버텍스 버퍼 바인딩.
	m_CubeVertexBufferStride = sizeof(CubeVertex);
	m_CubeVertexBufferOffset = 0;


	//  InputLayout 생성
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	ID3D10Blob* vertexShaderBuffer = nullptr;
	HR_T(CompileShaderFromFile(L"../shaders/14_BasicVertexShader.hlsl", "main", "vs_4_0", &vertexShaderBuffer));
	HR_T(m_pDevice->CreateInputLayout(layout, ARRAYSIZE(layout),
		vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_pCubeInputLayout));

	//버텍스 셰이더 생성
	HR_T(m_pDevice->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), NULL, &m_pCubeVertexShader));

	SAFE_RELEASE(vertexShaderBuffer);


	// 인덱스 버퍼 생성
	WORD indices[] =
	{
		3,1,0, 2,1,3,
		6,4,5, 7,4,6,
		11,9,8, 10,9,11,
		14,12,13, 15,12,14,
		19,17,16, 18,17,19,
		22,20,21, 23,20,22
	};

	// 인덱스 개수 저장.
	m_nCubeIndices = ARRAYSIZE(indices);

	bd = {};
	bd.ByteWidth = sizeof(WORD) * ARRAYSIZE(indices);
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA ibData = {};
	ibData.pSysMem = indices;
	HR_T(m_pDevice->CreateBuffer(&bd, &ibData, &m_pCubeIndexBuffer));

	// Cube 픽셀 셰이더 생성
	ID3D10Blob* pixelShaderBuffer = nullptr;
	HR_T(CompileShaderFromFile(L"../shaders/14_BasicPixelShader.hlsl", "main", "ps_4_0", &pixelShaderBuffer));
	HR_T(m_pDevice->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
		pixelShaderBuffer->GetBufferSize(), NULL, &m_pCubePixelShader));
	SAFE_RELEASE(pixelShaderBuffer);

	HR_T(CompileShaderFromFile(L"../shaders/14_SolidPixelShader.hlsl", "main", "ps_4_0", &pixelShaderBuffer));
	HR_T(m_pDevice->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
		pixelShaderBuffer->GetBufferSize(), NULL, &m_pSolidPixelShader));
	SAFE_RELEASE(pixelShaderBuffer);
}

bool TutorialApp::InitImGUI()
{
	/*
		ImGui 초기화.
	*/
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();


	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(m_hWnd);
	ImGui_ImplDX11_Init(this->m_pDevice, this->m_pDeviceContext);

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

void TutorialApp::RenderImGUI()
{
	/////////////////
		//아래부터는 ImGUI
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	{
		ImGui::Begin("Hello, world!");
	
		m_isHDRSupported ? ImGui::Text("HDR Support ") : ImGui::Text("No HDR Support ");
		ImGui::SameLine();		
		ImGui::Text("Monitor Max Nits: %.1f", m_MonitorMaxNits);	
		// 색역 정보
		ImGui::Text("Color Gamut Info:");
		if (m_isHDRSupported)
		{
			ImGui::Text("  Red Primary:   (%.4f, %.4f)", m_RedPrimary[0], m_RedPrimary[1]);
			ImGui::Text("  Green Primary: (%.4f, %.4f)", m_GreenPrimary[0], m_GreenPrimary[1]);
			ImGui::Text("  Blue Primary:  (%.4f, %.4f)", m_BluePrimary[0], m_BluePrimary[1]);

			if (m_isWideGamutSupported)
				ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "  -> Wide Gamut (DCI-P3+)");
			else
				ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "  -> Standard Gamut (Rec.709)");
		}

		// 백버퍼 포맷
		if (m_CurrFormat == DXGI_FORMAT_R10G10B10A2_UNORM)
		{
			ImGui::Text("Current Format: R10G10B10A2_UNORM ");			
		}			
		else if (m_CurrFormat == DXGI_FORMAT_R8G8B8A8_UNORM)
			ImGui::Text("Current Format: R8G8B8A8_UNORM");
		else
			ImGui::Text("Current Format: unknown");
		
		if (m_forceLDR)
		{
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "(cmd -LDR)");
		}

		// ColorSpace 정보
		if (m_CurrColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020)
			ImGui::Text("Current Color Space: HDR10 (ST.2084 PQ, Rec.2020)");
		else if (m_CurrColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709)
			ImGui::Text("Current Color Space: SDR (sRGB, Rec.709)");
		else
			ImGui::Text("Current Color Space: Unknown (%d)", m_CurrColorSpace);	

		
		if (m_forceP709)
		{
			ImGui::SameLine();
			ImGui::Text("(cmd -P709");
		}

		// ColorSpace 선택
		int temp = (m_CurrColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709) ? 0 : 1;
		const char* colorSpaceItems[] = { "0: sRGB/P709", "1: Wide/P2020" };
		if (ImGui::Combo("Select ColorSpace", &temp, colorSpaceItems, IM_ARRAYSIZE(colorSpaceItems)))
		{
			if(temp == 0)
				SelectColorSpace(DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709);				
			else
				SelectColorSpace(DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020);
		}
		

		ImGui::Separator();
		ImGui::Text("Tone Mapping Settings:");
		ImGui::Checkbox("Use Tone Mapping (ACES Film)", &m_UseToneMapping);
		ImGui::DragFloat("Exposure", &m_Exposure, 0.1f, -5.0f, 5.0f);
		ImGui::DragFloat("Reference White (nits)", &m_ReferenceWhiteNit, 10.0f, 80.0f, 10000.0f);

		ImGui::Separator();
		ImGui::DragFloat("Light0 intensity", &m_LightIntensity[0], 0.01f, 0.0f, 300.0f);
		ImGui::ColorEdit3("Light0 color", (float*)&m_LightColors[0]); // Edit 3 floats representing a color
		ImGui::SliderFloat("Light1 rotation", &m_rotationAngle, 0.0f, 360.0f);
		ImGui::DragFloat("Light1 intensity", &m_LightIntensity[1], 0.01f, 0.0f, 300.0f);
		ImGui::ColorEdit3("Light1 color", (float*)&m_LightColors[1]); // Edit 3 floats representing a color	
		ImGui::End();
	}

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK TutorialApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
		return true;

	return __super::WndProc(hWnd, message, wParam, lParam);
}
