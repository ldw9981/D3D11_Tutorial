

#include "TutorialApp.h"
#include "../Common/Helper.h"
#include <d3dcompiler.h>
#include "CubeObject.h"
#include <imgui.h>


#define RTTR_DLL
#include <rttr/registration>

#pragma comment (lib, "d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")

#ifdef _DEBUG
#pragma comment(lib, "rttr_core_d.lib")
#else 
#pragma comment(lib, "rttr_core.lib")
#endif

// ?뺤젏 援ъ“泥?
struct Vertex
{
	Vector3 Pos;		// ?꾩튂 ?뺣낫.
	Vector3 Normal;		// 踰뺤꽑 ?뺣낫.
};

struct ConstantBuffer
{
	Matrix mWorld;
	Matrix mView;
	Matrix mProjection;
	Vector4 vLightDir;
	Vector4 vLightColor;
	Vector4 vMaterialColor;
};

bool TutorialApp::OnInitialize()
{
	if (!InitD3D())
		return false;

	if (!InitScene())
		return false;

	if (!InitImGui())
		return false;

	return true;
}

void TutorialApp::OnUninitialize()
{
	UninitImGui();
	UninitScene();
	UninitD3D();
}

void TutorialApp::OnUpdate()
{
	float t = GameTimer::m_Instance->TotalTime();
	float dt = GameTimer::m_Instance->DeltaTime();

	// 移대찓???낅뜲?댄듃
	m_Camera.Update(dt);

	// 紐⑤뱺 GameObject AABB ?낅뜲?댄듃
	for (GameObject* obj : m_World.GetGameObjects())
	{
		if (obj)
			obj->UpdateAABB();
	}

	m_Camera.GetViewMatrix(m_View);
}

void TutorialApp::OnRender()
{
	float color[4] = { 0.0f, 0.5f, 0.5f, 1.0f };

	// ?뚮뜑?寃??ㅼ젙
	ID3D11RenderTargetView* rtvs[] = { m_pRenderTargetView.Get() };
	m_pDeviceContext->OMSetRenderTargets(1, rtvs, m_pDepthStencilView.Get());

	// Clear
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView.Get(), color);
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	// 紐⑤뱺 GameObject ?뚮뜑留?
	for (GameObject* obj : m_World.GetGameObjects())
	{
		if (!obj)
			continue;

		// AABB ?낅뜲?댄듃
		//obj->UpdateAABB();

		// CubeObject留??뚮뜑留?
		CubeObject* cube = dynamic_cast<CubeObject*>(obj);
		if (!cube)
			continue;

		// ?곸닔 踰꾪띁 ?낅뜲?댄듃
		ConstantBuffer cb;
		cb.mWorld = cube->GetWorldMatrix().Transpose();
		cb.mView = XMMatrixTranspose(m_View);
		cb.mProjection = XMMatrixTranspose(m_Projection);
		cb.vLightDir = m_LightDir;
		cb.vLightColor = m_LightColor;
		cb.vMaterialColor = cube->m_Color;
		m_pDeviceContext->UpdateSubresource(m_pConstantBuffer.Get(), 0, nullptr, &cb, 0, 0);

		// ?뚮뜑 ?먮툕
		m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		ID3D11Buffer* vbs[] = { m_pVertexBuffer.Get() };
		m_pDeviceContext->IASetVertexBuffers(0, 1, vbs, &m_VertexBufferStride, &m_VertexBufferOffset);
		m_pDeviceContext->IASetInputLayout(m_pInputLayout.Get());
		m_pDeviceContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
		m_pDeviceContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0);
		ID3D11Buffer* cbs[] = { m_pConstantBuffer.Get() };
		m_pDeviceContext->VSSetConstantBuffers(0, 1, cbs);
		m_pDeviceContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0);
		m_pDeviceContext->PSSetConstantBuffers(0, 1, cbs);

		m_pDeviceContext->DrawIndexed(m_nIndices, 0, 0);
	}

	// AABB ?붾쾭洹??뚮뜑留?(?먮툕 ?댄썑, ImGui ?댁쟾)
	// ?좏깮???ㅻ툕?앺듃????긽 ?쒖떆, Show AABB媛 耳쒖졇?덉쑝硫?紐⑤뱺 ?ㅻ툕?앺듃 ?쒖떆
	if (m_bShowAABB || m_pSelectedObject != nullptr)
	{
		// ?뚮뜑?寃??ㅼ떆 ?ㅼ젙 (ImGui媛 蹂寃쏀뻽?????덉쓬)
		m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), m_pDepthStencilView.Get());

		// DebugDraw??BasicEffect ?ㅼ젙
		DebugDraw::g_BatchEffect->SetWorld(Matrix::Identity);
		DebugDraw::g_BatchEffect->SetView(m_View);
		DebugDraw::g_BatchEffect->SetProjection(m_Projection);
		DebugDraw::g_BatchEffect->Apply(m_pDeviceContext.Get());

		// InputLayout ?ㅼ젙
		m_pDeviceContext->IASetInputLayout(DebugDraw::g_pBatchInputLayout.Get());

		// 釉붾젋???ㅽ뀒?댄듃 ?ㅼ젙
		m_pDeviceContext->OMSetBlendState(DebugDraw::g_States->AlphaBlend(), nullptr, 0xFFFFFFFF);
		m_pDeviceContext->OMSetDepthStencilState(DebugDraw::g_States->DepthNone(), 0);
		m_pDeviceContext->RSSetState(DebugDraw::g_States->CullNone());

		DebugDraw::g_Batch->Begin();

		for (GameObject* obj : m_World.GetGameObjects())
		{
			if (!obj)
				continue;

			// Show AABB媛 爰쇱졇?덉쑝硫??좏깮???ㅻ툕?앺듃留?洹몃━湲?
			if (!m_bShowAABB && obj != m_pSelectedObject)
				continue;

			// ?좏깮???ㅻ툕?앺듃??諛앹? 珥덈줉?? ?꾨땲硫??대몢??珥덈줉??
			XMVECTOR color = (obj == m_pSelectedObject) ?
				XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f) : XMVectorSet(0.0f, 0.2f, 0.0f, 0.7f);

			// DebugDraw??BoundingBox ?뚮뜑留??⑥닔 ?ъ슜
			DebugDraw::Draw(DebugDraw::g_Batch.get(), obj->m_AABB, color);
		}

		DebugDraw::g_Batch->End();

		// ?뚮뜑 ?ㅽ뀒?댄듃 蹂듭썝
		m_pDeviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
		m_pDeviceContext->OMSetDepthStencilState(nullptr, 0);
		m_pDeviceContext->RSSetState(nullptr);
	}

	// RTTR濡?GameObject 硫ㅻ쾭瑜?議고쉶??ImGui UI瑜??먮룞 ?앹꽦?섎뒗 ?⑥닔 ?몄텧
	RenderImGuiCubeRTTR();

	// ?덉씠 ?붾쾭洹??뚮뜑留?(ImGui ?댄썑??洹몃젮???꾩뿉 ?쒖떆)
	if (m_bShowDebugRay)
	{
		// ?뚮뜑?寃??ㅼ떆 ?ㅼ젙 (ImGui媛 蹂寃쏀뻽?????덉쓬)
		m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), m_pDepthStencilView.Get());

		// DebugDraw??BasicEffect ?ㅼ젙
		DebugDraw::g_BatchEffect->SetWorld(Matrix::Identity);
		DebugDraw::g_BatchEffect->SetView(m_View);
		DebugDraw::g_BatchEffect->SetProjection(m_Projection);
		DebugDraw::g_BatchEffect->Apply(m_pDeviceContext.Get());

		// InputLayout ?ㅼ젙
		m_pDeviceContext->IASetInputLayout(DebugDraw::g_pBatchInputLayout.Get());

		// 釉붾젋???ㅽ뀒?댄듃 ?ㅼ젙 (?뚰뙆 釉붾젋??
		m_pDeviceContext->OMSetBlendState(DebugDraw::g_States->AlphaBlend(), nullptr, 0xFFFFFFFF);
		m_pDeviceContext->OMSetDepthStencilState(DebugDraw::g_States->DepthNone(), 0);
		m_pDeviceContext->RSSetState(DebugDraw::g_States->CullNone());

		DebugDraw::g_Batch->Begin();

		Vector3 rayStart = m_DebugRayOrigin;
		Vector3 rayEnd = m_DebugRayOrigin + m_DebugRayDirection * 100.0f;

		// 鍮④컙?????몃???洹몃씪?곗씠???덉씠
		VertexPositionColor v1(rayStart, Vector4(1.0f, 0.0f, 0.0f, 1.0f));
		VertexPositionColor v2(rayEnd, Vector4(1.0f, 1.0f, 0.0f, 1.0f));

		DebugDraw::g_Batch->DrawLine(v1, v2);

		DebugDraw::g_Batch->End();

		// ?뚮뜑 ?ㅽ뀒?댄듃 蹂듭썝
		m_pDeviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
		m_pDeviceContext->OMSetDepthStencilState(nullptr, 0);
		m_pDeviceContext->RSSetState(nullptr);
	}

	// Present
	m_pSwapChain->Present(0, 0);
}

bool TutorialApp::InitD3D()
{
	HRESULT hr = 0;

	// ?ㅼ솑泥댁씤 ?띿꽦 ?ㅼ젙 援ъ“泥??앹꽦.
	DXGI_SWAP_CHAIN_DESC swapDesc = {};
	swapDesc.BufferCount = 1;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.OutputWindow = m_hWnd;
	swapDesc.Windowed = true;
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapDesc.BufferDesc.Width = m_ClientWidth;
	swapDesc.BufferDesc.Height = m_ClientHeight;
	swapDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapDesc.SampleDesc.Count = 1;
	swapDesc.SampleDesc.Quality = 0;

	UINT creationFlags = 0;
#ifdef _DEBUG
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// ?붾컮?댁뒪, ?ㅼ솑泥댁씤, ?붾컮?댁뒪 而⑦뀓?ㅽ듃 ?앹꽦.
	HR_T(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, creationFlags, NULL, NULL,
		D3D11_SDK_VERSION, &swapDesc, m_pSwapChain.GetAddressOf(), m_pDevice.GetAddressOf(), NULL, m_pDeviceContext.GetAddressOf()));

	// ?뚮뜑?寃잙럭 ?앹꽦 (諛깅쾭?쇰? ?댁슜?섎뒗 ?뚮뜑?寃잙럭)
	ComPtr<ID3D11Texture2D> pBackBufferTexture;
	HR_T(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)pBackBufferTexture.GetAddressOf()));
	HR_T(m_pDevice->CreateRenderTargetView(pBackBufferTexture.Get(), NULL, m_pRenderTargetView.GetAddressOf()));

	ID3D11RenderTargetView* rtvs[] = { m_pRenderTargetView.Get() };
	m_pDeviceContext->OMSetRenderTargets(1, rtvs, NULL);

	// 酉고룷???ㅼ젙
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)m_ClientWidth;
	viewport.Height = (float)m_ClientHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	m_pDeviceContext->RSSetViewports(1, &viewport);

	// 源딆씠&?ㅽ뀗??踰꾪띁 ?앹꽦
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

	ComPtr<ID3D11Texture2D> textureDepthStencil;
	HR_T(m_pDevice->CreateTexture2D(&descDepth, nullptr, textureDepthStencil.GetAddressOf()));

	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	HR_T(m_pDevice->CreateDepthStencilView(textureDepthStencil.Get(), &descDSV, m_pDepthStencilView.GetAddressOf()));

	ID3D11RenderTargetView* rtvs2[] = { m_pRenderTargetView.Get() };
	m_pDeviceContext->OMSetRenderTargets(1, rtvs2, m_pDepthStencilView.Get());

	return true;
}

void TutorialApp::UninitD3D()
{
	// ComPtr ?ъ슜?쇰줈 ?먮룞 ?댁젣
}

bool TutorialApp::InitScene()
{
	HRESULT hr = 0;
	ID3D10Blob* errorMessage = nullptr;

	// 踰꾪뀓???뺤쓽 (?먮툕)
	Vertex vertices[] =
	{
		// ?욌㈃ (Z-)
		{ Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.0f, 0.0f, -1.0f) },
		{ Vector3(-0.5f,  0.5f, -0.5f), Vector3(0.0f, 0.0f, -1.0f) },
		{ Vector3( 0.5f,  0.5f, -0.5f), Vector3(0.0f, 0.0f, -1.0f) },
		{ Vector3( 0.5f, -0.5f, -0.5f), Vector3(0.0f, 0.0f, -1.0f) },

		// ?룸㈃ (Z+)
		{ Vector3(-0.5f, -0.5f, 0.5f), Vector3(0.0f, 0.0f, 1.0f) },
		{ Vector3( 0.5f, -0.5f, 0.5f), Vector3(0.0f, 0.0f, 1.0f) },
		{ Vector3( 0.5f,  0.5f, 0.5f), Vector3(0.0f, 0.0f, 1.0f) },
		{ Vector3(-0.5f,  0.5f, 0.5f), Vector3(0.0f, 0.0f, 1.0f) },

		// ?쇱そ硫?(X-)
		{ Vector3(-0.5f, -0.5f,  0.5f), Vector3(-1.0f, 0.0f, 0.0f) },
		{ Vector3(-0.5f,  0.5f,  0.5f), Vector3(-1.0f, 0.0f, 0.0f) },
		{ Vector3(-0.5f,  0.5f, -0.5f), Vector3(-1.0f, 0.0f, 0.0f) },
		{ Vector3(-0.5f, -0.5f, -0.5f), Vector3(-1.0f, 0.0f, 0.0f) },

		// ?ㅻⅨ履쎈㈃ (X+)
		{ Vector3(0.5f, -0.5f, -0.5f), Vector3(1.0f, 0.0f, 0.0f) },
		{ Vector3(0.5f,  0.5f, -0.5f), Vector3(1.0f, 0.0f, 0.0f) },
		{ Vector3(0.5f,  0.5f,  0.5f), Vector3(1.0f, 0.0f, 0.0f) },
		{ Vector3(0.5f, -0.5f,  0.5f), Vector3(1.0f, 0.0f, 0.0f) },

		// ?쀫㈃ (Y+)
		{ Vector3(-0.5f, 0.5f, -0.5f), Vector3(0.0f, 1.0f, 0.0f) },
		{ Vector3(-0.5f, 0.5f,  0.5f), Vector3(0.0f, 1.0f, 0.0f) },
		{ Vector3( 0.5f, 0.5f,  0.5f), Vector3(0.0f, 1.0f, 0.0f) },
		{ Vector3( 0.5f, 0.5f, -0.5f), Vector3(0.0f, 1.0f, 0.0f) },

		// ?꾨옯硫?(Y-)
		{ Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.0f, -1.0f, 0.0f) },
		{ Vector3( 0.5f, -0.5f, -0.5f), Vector3(0.0f, -1.0f, 0.0f) },
		{ Vector3( 0.5f, -0.5f,  0.5f), Vector3(0.0f, -1.0f, 0.0f) },
		{ Vector3(-0.5f, -0.5f,  0.5f), Vector3(0.0f, -1.0f, 0.0f) },
	};

	// 踰꾪뀓??踰꾪띁 ?앹꽦
	D3D11_BUFFER_DESC bd = {};
	bd.ByteWidth = sizeof(Vertex) * ARRAYSIZE(vertices);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA vbData = {};
	vbData.pSysMem = vertices;
	HR_T(m_pDevice->CreateBuffer(&bd, &vbData, m_pVertexBuffer.GetAddressOf()));

	m_VertexBufferStride = sizeof(Vertex);
	m_VertexBufferOffset = 0;

	// ?몃뜳???뺤쓽
	WORD indices[] =
	{
		0, 1, 2,  0, 2, 3,    // ?욌㈃
		4, 5, 6,  4, 6, 7,    // ?룸㈃
		8, 9, 10, 8, 10, 11,  // ?쇱そ硫?
		12, 13, 14, 12, 14, 15, // ?ㅻⅨ履쎈㈃
		16, 17, 18, 16, 18, 19, // ?쀫㈃
		20, 21, 22, 20, 22, 23  // ?꾨옯硫?
	};

	m_nIndices = ARRAYSIZE(indices);

	// ?몃뜳??踰꾪띁 ?앹꽦
	bd = {};
	bd.ByteWidth = sizeof(WORD) * ARRAYSIZE(indices);
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA ibData = {};
	ibData.pSysMem = indices;
	HR_T(m_pDevice->CreateBuffer(&bd, &ibData, m_pIndexBuffer.GetAddressOf()));

	// Input Layout ?앹꽦
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	ComPtr<ID3D10Blob> vertexShaderBuffer;
	HR_T(CompileShaderFromFile(L"../Shaders/97_VertexShader.hlsl", "main", "vs_4_0", vertexShaderBuffer.GetAddressOf()));
	HR_T(m_pDevice->CreateInputLayout(layout, ARRAYSIZE(layout),
		vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), m_pInputLayout.GetAddressOf()));

	// 踰꾪뀓???곗씠???앹꽦
	HR_T(m_pDevice->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), NULL, m_pVertexShader.GetAddressOf()));

	// ?쎌? ?곗씠???앹꽦
	ComPtr<ID3D10Blob> pixelShaderBuffer;
	HR_T(CompileShaderFromFile(L"../Shaders/97_PixelShader.hlsl", "main", "ps_4_0", pixelShaderBuffer.GetAddressOf()));
	HR_T(m_pDevice->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
		pixelShaderBuffer->GetBufferSize(), NULL, m_pPixelShader.GetAddressOf()));

	// ?곸닔 踰꾪띁 ?앹꽦
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	HR_T(m_pDevice->CreateBuffer(&bd, nullptr, m_pConstantBuffer.GetAddressOf()));

	// 珥덇린媛??ㅼ젙
	XMVECTOR Eye = XMVectorSet(0.0f, 2.0f, -5.0f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	m_View = XMMatrixLookAtLH(Eye, At, Up);
	m_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, m_ClientWidth / (FLOAT)m_ClientHeight, 0.1f, 1000.0f);

	// ?뚯뒪?몄슜 Cube ?앹꽦
	CubeObject* cube = m_World.CreateGameObject<CubeObject>();
	cube->m_Name = "TestCube";
	cube->m_Position = Vector3(0.0f, 0.0f, 0.0f);
	m_pSelectedObject = cube;

	// RTTR???깅줉??GameObject ?뚯깮 ??낅뱾??媛?몄????ㅻ툕?앺듃 ?붾젅?몄뿉 ?쒖떆
	using namespace rttr;
	type baseType = type::get<GameObject>();
	for (auto& t : type::get_types())
	{
		// GameObject?먯꽌 ?뚯깮?섏뿀怨? ?ъ씤??李몄“ ??낆씠 ?꾨땲硫? ?대옒????낆씤 寃껊쭔 異붽?
		if (t.is_derived_from(baseType) &&
			t != baseType &&
			!t.is_pointer() &&
			!t.is_array() &&
			t.is_class())
		{
			std::string typeName = t.get_name().to_string();
			// ?대쫫??'*'??'&'媛 ?ы븿??寃쎌슦???쒖쇅
			if (typeName.find('*') == std::string::npos &&
				typeName.find('&') == std::string::npos)
			{
				m_AvailableObjectTypes.push_back(typeName);
			}
		}
	}

	// 移대찓??珥덇린??
	m_Camera.m_Position = Vector3(0.0f, 2.0f, -10.0f);
	m_Camera.m_Rotation = Vector3(0.0f, 0.0f, 0.0f);

	// DebugDraw 珥덇린??
	HR_T(DebugDraw::Initialize(m_pDevice, m_pDeviceContext));

	return true;
}

void TutorialApp::UninitScene()
{
	// DebugDraw ?댁젣
	DebugDraw::Uninitialize();

	// ComPtr ?ъ슜?쇰줈 ?먮룞 ?댁젣
}


void TutorialApp::RenderImGuiCubeRTTR()
{
	m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), nullptr);

	//////////////////////////////////////////////////////////////////////////
	// ImGui Rendering
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// 硫붾돱 諛?
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Save", "Ctrl+S"))
			{
				SaveScene();
			}
			if (ImGui::MenuItem("Load", "Ctrl+O"))
			{
				LoadScene();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Exit", "Alt+F4"))
			{
				PostQuitMessage(0);
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Debug"))
		{
			ImGui::MenuItem("Show Ray", nullptr, &m_bShowDebugRay);
			ImGui::MenuItem("Show AABB", nullptr, &m_bShowAABB);
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	// ?먯쑀濡?쾶 ?吏곸씠??李쎈뱾
	// World Hierarchy (珥덇린 ?꾩튂留??ㅼ젙, ?댄썑 ?ъ슜?먭? ?먯쑀濡?쾶 ?대룞 媛??
	ImGui::SetNextWindowPos(ImVec2(10, 30), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);
	ImGui::Begin("World Hierarchy");
	RenderWorldHierarchyContent();
	ImGui::End();

	// Object Palette
	ImGui::SetNextWindowPos(ImVec2(10, 450), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);
	ImGui::Begin("Object Palette");
	RenderObjectPaletteContent();
	ImGui::End();

	// Inspector
	ImGui::SetNextWindowPos(ImVec2(static_cast<float>(m_ClientWidth - 410), 30), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(400, 700), ImGuiCond_FirstUseEver);
	ImGui::Begin("Inspector");
	RenderInspectorContent();
	ImGui::End();

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

bool TutorialApp::InitImGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui::StyleColorsDark();

	if(!ImGui_ImplWin32_Init(m_hWnd))
		return false;

	if(!ImGui_ImplDX11_Init(m_pDevice.Get(), m_pDeviceContext.Get()))
		return false;

	return true;
}

void TutorialApp::UninitImGui()
{
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

void TutorialApp::OnInputProcess(const Keyboard::State& KeyState, const Keyboard::KeyboardStateTracker& KeyTracker,
	const Mouse::State& MouseState, const Mouse::ButtonStateTracker& MouseTracker)
{
	// 移대찓?쇰줈 ?낅젰 ?꾨떖
	m_Camera.OnInputProcess(KeyState, KeyTracker, MouseState, MouseTracker);

	// Ray Picking: 留덉슦???대┃?쇰줈 ?ㅻ툕?앺듃 ?좏깮
	if (MouseTracker.leftButton == Mouse::ButtonStateTracker::PRESSED)
	{
		// ImGui 李쎌씠 留덉슦?ㅻ? 罹≪쿂?섏? ?딆븯???뚮쭔 泥섎━ (3D 酉고룷???대┃留?泥섎━)
		if (!ImGui::GetIO().WantCaptureMouse)
		{
			// 留덉슦???ㅽ겕由?醫뚰몴瑜?NDC濡?蹂??
			float x = (2.0f * MouseState.x) / m_ClientWidth - 1.0f;
			float y = 1.0f - (2.0f * MouseState.y) / m_ClientHeight;

			// NDC瑜?酉?怨듦컙?쇰줈 蹂??
			Matrix invProj = m_Projection.Invert();
			Vector4 rayClip(x, y, 1.0f, 1.0f);  // Near plane (z = 1.0f in NDC)
			Vector4 rayEye = Vector4::Transform(rayClip, invProj);
			rayEye.z = 1.0f;  // Forward direction
			rayEye.w = 0.0f;  // Direction vector (not a point)

			// 酉?怨듦컙???붾뱶 怨듦컙?쇰줈 蹂??
			Matrix invView = m_View.Invert();
			Vector4 rayWorld4 = Vector4::Transform(rayEye, invView);
			Vector3 rayDir(rayWorld4.x, rayWorld4.y, rayWorld4.z);
			rayDir.Normalize();

			// Ray ?앹꽦 (移대찓???꾩튂?먯꽌 ?붾뱶 諛⑺뼢?쇰줈)
			Ray ray(m_Camera.m_Position, rayDir);

			// ?붾쾭洹??덉씠 ?뺣낫 ???(Show Ray媛 耳쒖졇?덉쓣 ?뚮쭔)
			if (m_bShowDebugRay)
			{
				m_DebugRayOrigin = m_Camera.m_Position;
				m_DebugRayDirection = rayDir;
			}

			// GameWorld?먯꽌 Ray 異⑸룎 寃??
			float hitDistance = 0.0f;
			GameObject* hitObject = m_World.RayCast(ray, &hitDistance);

			// ?좏깮???ㅻ툕?앺듃 ?낅뜲?댄듃
			m_pSelectedObject = hitObject;
		}
	}
}

void TutorialApp::SaveScene()
{
	// ?뚯씪 ????ㅼ씠?쇰줈洹?
	OPENFILENAMEA ofn = {};
	char szFile[260] = {};

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = m_hWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "JSON Files (*.json)\0*.json\0All Files (*.*)\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = "json";

	if (GetSaveFileNameA(&ofn) != TRUE)
		return; // ?ъ슜?먭? 痍⑥냼??

	std::string filename = szFile;

	// GameWorld瑜??뚯씪?????
	if (m_World.SaveToFile(filename))
	{
		MessageBoxA(m_hWnd, "Scene saved successfully!", "Save", MB_OK | MB_ICONINFORMATION);
	}
	else
	{
		MessageBoxA(m_hWnd, "Failed to save scene!", "Error", MB_OK | MB_ICONERROR);
	}
}

void TutorialApp::LoadScene()
{
	// ?뚯씪 ?닿린 ?ㅼ씠?쇰줈洹?
	OPENFILENAMEA ofn = {};
	char szFile[260] = {};

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = m_hWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "JSON Files (*.json)\0*.json\0All Files (*.*)\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileNameA(&ofn) != TRUE)
		return; // ?ъ슜?먭? 痍⑥냼??

	std::string filename = szFile;

	// GameWorld瑜??뚯씪?먯꽌 濡쒕뱶
	if (m_World.LoadFromFile(filename))
	{
		// 泥?踰덉㎏ 媛앹껜瑜??좏깮
		if (m_World.GetCount() > 0)
		{
			m_pSelectedObject = m_World.GetGameObjects()[0];
		}
		else
		{
			m_pSelectedObject = nullptr;
		}

		MessageBoxA(m_hWnd, "Scene loaded successfully!", "Load", MB_OK | MB_ICONINFORMATION);
	}
	else
	{
		MessageBoxA(m_hWnd, "Failed to load scene! File not found.", "Error", MB_OK | MB_ICONERROR);
	}
}

void TutorialApp::RenderObjectPaletteContent()
{
	ImGui::Text("Available Objects:");
	ImGui::Separator();

	// ?ъ슜 媛?ν븳 ?ㅻ툕?앺듃 ???紐⑸줉 ?쒖떆
	for (const std::string& typeName : m_AvailableObjectTypes)
	{
		ImGui::Button(typeName.c_str(), ImVec2(-1, 30));

		// ?쒕옒洹??뚯뒪 ?ㅼ젙
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
		{
			// ?섏씠濡쒕뱶濡?????대쫫 ?꾨떖
			ImGui::SetDragDropPayload("GAMEOBJECT_TYPE", typeName.c_str(), typeName.size() + 1);
			ImGui::Text("Create %s", typeName.c_str());
			ImGui::EndDragDropSource();
		}
	}
}

void TutorialApp::RenderWorldHierarchyContent()
{
	ImGui::Text("Scene Objects (%zu)", m_World.GetCount());
	ImGui::Separator();

	// ?꾩껜 李??곸뿭???쒕∼ ?寃잛쑝濡??ㅼ젙
	ImVec2 contentSize = ImGui::GetContentRegionAvail();

	// InvisibleButton?쇰줈 ?꾩껜 ?곸뿭???쒕∼ ?寃잛쑝濡?留뚮벀
	ImGui::InvisibleButton("##HierarchyDropZone", contentSize);

	// ?쒕∼ ?寃??ㅼ젙
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GAMEOBJECT_TYPE"))
		{
			const char* typeName = (const char*)payload->Data;
			GameObject* newObj = m_World.CreateGameObjectByTypeName(typeName);
			if (newObj)
			{
				// 移대찓???뺣㈃(Z異?+10) ?꾩튂???앹꽦
				Vector3 forward = Vector3(0.0f, 0.0f, 1.0f);
				Matrix rotationMatrix = Matrix::CreateFromYawPitchRoll(m_Camera.m_Rotation.y, m_Camera.m_Rotation.x, m_Camera.m_Rotation.z);
				forward = Vector3::Transform(forward, rotationMatrix);
				newObj->m_Position = m_Camera.m_Position + forward * 10.0f;

				m_pSelectedObject = newObj;

				CubeObject* cube = dynamic_cast<CubeObject*>(newObj);
				if (cube)
				{
					static int cubeCount = 0;
					cube->m_Name = "Cube_" + std::to_string(cubeCount++);
				}
			}
		}
		ImGui::EndDragDropTarget();
	}

	// 由ъ뒪?몃? InvisibleButton ?꾩뿉 寃뱀퀜 洹몃━湲?
	ImVec2 listPos = ImGui::GetItemRectMin();
	ImGui::SetCursorScreenPos(listPos);

	ImGui::BeginChild("HierarchyList", contentSize, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground);

	// 紐⑤뱺 GameObject 紐⑸줉 ?쒖떆
	const auto& objects = m_World.GetGameObjects();
	for (size_t i = 0; i < objects.size(); ++i)
	{
		GameObject* obj = objects[i];
		if (!obj)
			continue;

		// ?좏깮???ㅻ툕?앺듃???섏씠?쇱씠??
		bool isSelected = (obj == m_pSelectedObject);

		// ?ㅻ툕?앺듃 ?대쫫 媛?몄삤湲?(CubeObject??寃쎌슦)
		std::string objectName = "GameObject";
		CubeObject* cube = dynamic_cast<CubeObject*>(obj);
		if (cube && !cube->m_Name.empty())
		{
			objectName = cube->m_Name;
		}
		else
		{
			// RTTR濡?????대쫫 媛?몄삤湲?
			using namespace rttr;
			type t = type::get(*obj);
			objectName = t.get_name().to_string() + "_" + std::to_string(i);
		}

		// Selectable ??ぉ?쇰줈 ?쒖떆
		if (ImGui::Selectable(objectName.c_str(), isSelected))
		{
			m_pSelectedObject = obj;
		}

		// ?고겢由?硫붾돱
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete"))
			{
				m_World.DestroyGameObject(obj);
				if (m_pSelectedObject == obj)
					m_pSelectedObject = nullptr;
			}
			ImGui::EndPopup();
		}
	}

	// 由ъ뒪?멸? 鍮꾩뼱?덉쓣 ???덈궡 硫붿떆吏
	if (objects.empty())
	{
		ImGui::TextDisabled("(Empty - Drag objects here)");
	}

	ImGui::EndChild();
}

void TutorialApp::RenderInspectorContent()
{
	using namespace rttr;

	// ?좏깮??媛앹껜媛 ?놁쑝硫?硫붿떆吏 ?쒖떆
	if (!m_pSelectedObject)
	{
		ImGui::Text("No GameObject selected");
		return;
	}

	type t = type::get(*m_pSelectedObject);
	ImGui::Text("Type: %s", t.get_name().to_string().c_str());
	ImGui::Separator();

	for (auto& prop : t.get_properties())
	{
		variant value = prop.get_value(*m_pSelectedObject);
		std::string name = prop.get_name().to_string();
		if (value.is_type<float>())
		{
			float v = value.get_value<float>();
			ImGui::InputFloat(name.c_str(), &v, 0.0f, 0.0f, "%.3f");
			if (ImGui::IsItemDeactivatedAfterEdit())
				prop.set_value(*m_pSelectedObject, v);
		}
		else if (value.is_type<std::string>())
		{
			char buf[256] = {};
			strncpy_s(buf, value.get_value<std::string>().c_str(), sizeof(buf) - 1);
			if (ImGui::InputText(name.c_str(), buf, sizeof(buf), ImGuiInputTextFlags_EnterReturnsTrue))
				prop.set_value(*m_pSelectedObject, std::string(buf));
		}
		else if (value.is_type<Vector3>())
		{
			Vector3 v = value.get_value<Vector3>();
			float arr[3] = { v.x, v.y, v.z };
			ImGui::InputFloat3(name.c_str(), arr, "%.3f");
			if (ImGui::IsItemDeactivatedAfterEdit())
			{
				v.x = arr[0]; v.y = arr[1]; v.z = arr[2];
				prop.set_value(*m_pSelectedObject, v);
			}
		}
		else if (value.is_type<Vector4>())
		{
			Vector4 v = value.get_value<Vector4>();
			float arr[4] = { v.x, v.y, v.z, v.w };
			if (ImGui::ColorEdit4(name.c_str(), arr))
			{
				v.x = arr[0]; v.y = arr[1]; v.z = arr[2]; v.w = arr[3];
				prop.set_value(*m_pSelectedObject, v);
			}
		}

		// ?ㅻ챸 硫뷀??곗씠??媛?몄삤湲?
		auto desc_var = prop.get_metadata("desc");
		if (desc_var.is_valid() && ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("%s", desc_var.to_string().c_str());
		}
	}
}

Vector3 TutorialApp::GetWorldPositionFromMouse(const ImVec2& mousePos)
{
	// ?ㅽ겕由?醫뚰몴瑜?NDC(Normalized Device Coordinates)濡?蹂??
	float x = (2.0f * mousePos.x) / m_ClientWidth - 1.0f;
	float y = 1.0f - (2.0f * mousePos.y) / m_ClientHeight;

	// NDC瑜?酉?怨듦컙?쇰줈 蹂??
	Matrix invProj = m_Projection.Invert();
	Vector4 rayClip(x, y, -1.0f, 1.0f);
	Vector4 rayView = Vector4::Transform(rayClip, invProj);
	rayView.z = -1.0f;
	rayView.w = 0.0f;

	// 酉?怨듦컙???붾뱶 怨듦컙?쇰줈 蹂??
	Matrix invView = m_View.Invert();
	Vector4 rayWorld4 = Vector4::Transform(rayView, invView);
	Vector3 rayWorld(rayWorld4.x, rayWorld4.y, rayWorld4.z);
	rayWorld.Normalize();

	// 移대찓???꾩튂?먯꽌 Ray ?앹꽦
	Vector3 camPos = m_Camera.m_Position;

	// Y=0 ?됰㈃怨쇱쓽 援먯감??怨꾩궛 (諛붾떏??諛곗튂)
	if (rayWorld.y != 0.0f)
	{
		float t = -camPos.y / rayWorld.y;
		if (t > 0.0f)
		{
			return camPos + rayWorld * t;
		}
	}

	// 湲곕낯媛? 移대찓???꾨갑 5?좊떅
	return camPos + rayWorld * 5.0f;
}