#include "TutorialApp.h"
#include "../Common/Helper.h"
#include <d3dcompiler.h>
#include <vector>

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

#pragma comment (lib, "d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")

using namespace Microsoft::WRL;

#include <physx/characterkinematic/PxExtended.h>

using namespace physx;
// ControllerHitReport implementation
void ControllerHitReport::onShapeHit(const PxControllerShapeHit& hit)
{
	// Controller가 Dynamic Actor와 충돌했을 때 밀어내기
	PxRigidDynamic* actor = hit.actor->is<PxRigidDynamic>();
	if (actor)
	{
		// 충돌 방향 계산 (Controller에서 Actor로)
		PxExtendedVec3 cctPos = hit.controller->getPosition();
		PxVec3 worldPos((PxReal)hit.worldPos.x, (PxReal)hit.worldPos.y, (PxReal)hit.worldPos.z);
		PxVec3 controllerPos((PxReal)cctPos.x, (PxReal)cctPos.y, (PxReal)cctPos.z);
		PxVec3 dir = worldPos - controllerPos;
		dir.y = 0.0f;
		
		const PxF32 dirMagnitude = dir.magnitude();
		if (dirMagnitude > 0.001f)
		{
			dir.normalize();
			
			// 간단하고 고정된 힘으로 밀기 (PhysX 예제 방식)
			const PxF32 pushStrength = 5.0f;
			actor->addForce(dir * pushStrength, PxForceMode::eACCELERATION);
		}
	}
}

struct ConstantBuffer
{
	Matrix mWorld;
	Matrix mView;
	Matrix mProjection;
	Vector4 vLightDir;
	Vector4 vLightColor;
	Vector4 vObjectColor;
};

struct Vertex
{
	Vector3 Pos;
	Vector3 Normal;
	Vector4 Color;
};

bool TutorialApp::OnInitialize()
{
	if (!InitD3D())
		return false;

	if (!InitPhysX())
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
	UninitPhysX();
	UninitD3D();
}

void TutorialApp::OnUpdate()
{
	// GameApp에서 제공하는 타이머 사용
	float elapsedTime = m_Timer.DeltaTime();

	// 카메라 업데이트
	m_Camera.Update(elapsedTime);

	// PhysX 업데이트
	UpdatePhysX(elapsedTime);
}

void TutorialApp::OnRender()
{
	float clearColor[4] = { 0.2f, 0.2f, 0.3f, 1.0f };
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, clearColor);
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// Get camera view matrix
	m_Camera.GetViewMatrix(m_View);

	// Setup constant buffer
	ConstantBuffer cb;
	cb.vLightDir = Vector4(-0.577f, 0.577f, -0.577f, 0.0f);
	cb.vLightColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	cb.mView = XMMatrixTranspose(m_View);
	cb.mProjection = XMMatrixTranspose(m_Projection);

	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pDeviceContext->IASetInputLayout(m_pInputLayout);
	m_pDeviceContext->VSSetShader(m_pVertexShader, nullptr, 0);
	m_pDeviceContext->PSSetShader(m_pPixelShader, nullptr, 0);

	// Render Ground Box (Static)
	{
		// 바닥은 100x2x100 크기로 스케일링
		Matrix scaleMatrix = XMMatrixScaling(50.0f, 1.0f, 50.0f);
		Matrix translationMatrix = XMMatrixTranslation(m_GroundPosition.x, m_GroundPosition.y, m_GroundPosition.z);
		m_World = scaleMatrix * translationMatrix;
		cb.mWorld = XMMatrixTranspose(m_World);
		cb.vLightColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		cb.vObjectColor = Vector4(0.5f, 0.5f, 0.5f, 1.0f); // 회색
		m_pDeviceContext->UpdateSubresource(m_pConstantBuffer, 0, nullptr, &cb, 0, 0);
		m_pDeviceContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
		m_pDeviceContext->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);

		m_pDeviceContext->IASetVertexBuffers(0, 1, &m_pCubeVertexBuffer, &m_VertexBufferStride, &m_VertexBufferOffset);
		m_pDeviceContext->IASetIndexBuffer(m_pCubeIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
		m_pDeviceContext->DrawIndexed(m_nCubeIndices, 0, 0);
	}

	// 라이트 색상 원래대로 복원
	cb.vLightColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

	// === Dynamic Actors 렌더링 (일반 색상) ===

	// Render Dynamic Cube
	{
		m_World = XMMatrixTranslation(m_DynamicCubePosition.x, m_DynamicCubePosition.y, m_DynamicCubePosition.z);
		cb.mWorld = XMMatrixTranspose(m_World);
		cb.vLightColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		cb.vObjectColor = Vector4(1.0f, 0.0f, 0.0f, 1.0f); // 빨강색
		m_pDeviceContext->UpdateSubresource(m_pConstantBuffer, 0, nullptr, &cb, 0, 0);
		m_pDeviceContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
		m_pDeviceContext->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);

		m_pDeviceContext->IASetVertexBuffers(0, 1, &m_pCubeVertexBuffer, &m_VertexBufferStride, &m_VertexBufferOffset);
		m_pDeviceContext->IASetIndexBuffer(m_pCubeIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
		m_pDeviceContext->DrawIndexed(m_nCubeIndices, 0, 0);
	}

	// Render Dynamic Sphere
	{
		m_World = XMMatrixTranslation(m_DynamicSpherePosition.x, m_DynamicSpherePosition.y, m_DynamicSpherePosition.z);
		cb.mWorld = XMMatrixTranspose(m_World);
		cb.vLightColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		cb.vObjectColor = Vector4(0.0f, 1.0f, 0.0f, 1.0f); // 초록색
		m_pDeviceContext->UpdateSubresource(m_pConstantBuffer, 0, nullptr, &cb, 0, 0);
		m_pDeviceContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
		m_pDeviceContext->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);

		m_pDeviceContext->IASetVertexBuffers(0, 1, &m_pSphereVertexBuffer, &m_VertexBufferStride, &m_VertexBufferOffset);
		m_pDeviceContext->IASetIndexBuffer(m_pSphereIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
		m_pDeviceContext->DrawIndexed(m_nSphereIndices, 0, 0);
	}

	// === Static Actors 렌더링 (회색) ===

	// Render Static Cube
	{
		m_World = XMMatrixTranslation(m_StaticCubePosition.x, m_StaticCubePosition.y, m_StaticCubePosition.z);
		cb.mWorld = XMMatrixTranspose(m_World);
		cb.vLightColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		cb.vObjectColor = Vector4(0.6f, 0.6f, 0.6f, 1.0f); // 회색
		m_pDeviceContext->UpdateSubresource(m_pConstantBuffer, 0, nullptr, &cb, 0, 0);
		m_pDeviceContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
		m_pDeviceContext->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);

		m_pDeviceContext->IASetVertexBuffers(0, 1, &m_pCubeVertexBuffer, &m_VertexBufferStride, &m_VertexBufferOffset);
		m_pDeviceContext->IASetIndexBuffer(m_pCubeIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
		m_pDeviceContext->DrawIndexed(m_nCubeIndices, 0, 0);
	}

	// Render Static Sphere
	{
		m_World = XMMatrixTranslation(m_StaticSpherePosition.x, m_StaticSpherePosition.y, m_StaticSpherePosition.z);
		cb.mWorld = XMMatrixTranspose(m_World);
		cb.vLightColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		cb.vObjectColor = Vector4(0.6f, 0.6f, 0.6f, 1.0f); // 회색
		m_pDeviceContext->UpdateSubresource(m_pConstantBuffer, 0, nullptr, &cb, 0, 0);
		m_pDeviceContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
		m_pDeviceContext->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);

		m_pDeviceContext->IASetVertexBuffers(0, 1, &m_pSphereVertexBuffer, &m_VertexBufferStride, &m_VertexBufferOffset);
		m_pDeviceContext->IASetIndexBuffer(m_pSphereIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
		m_pDeviceContext->DrawIndexed(m_nSphereIndices, 0, 0);
	}

	// Render Capsule (Character Controller position)
	{
		m_World = XMMatrixTranslation(m_CapsulePosition.x, m_CapsulePosition.y, m_CapsulePosition.z);
		cb.mWorld = XMMatrixTranspose(m_World);
		cb.vObjectColor = Vector4(0.0f, 1.0f, 0.0f, 1.0f); // 녹색
		m_pDeviceContext->UpdateSubresource(m_pConstantBuffer, 0, nullptr, &cb, 0, 0);
		m_pDeviceContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
		m_pDeviceContext->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);

		m_pDeviceContext->IASetVertexBuffers(0, 1, &m_pCapsuleVertexBuffer, &m_VertexBufferStride, &m_VertexBufferOffset);
		m_pDeviceContext->IASetIndexBuffer(m_pCapsuleIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
		m_pDeviceContext->DrawIndexed(m_nCapsuleIndices, 0, 0);
	}

	// ImGui 렌더링
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// PhysX 오브젝트 위치 표시
	ImGui::Begin("PhysX Debug Info", nullptr, ImGuiWindowFlags_None);
	ImGui::Text("DeltaTime: %.4f sec (%.1f FPS)", m_Timer.DeltaTime(), 1.0f / m_Timer.DeltaTime());
	ImGui::Separator();

	// CUDA 지원 정보
	ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "=== GPU Acceleration ===");
	if (m_pCudaContextManager)
	{
		ImGui::Text("CUDA: Supported");
		ImGui::Text("CUDA Context Manager: Active");
	}
	else
	{
		ImGui::Text("CUDA: Not Available");
	}
	ImGui::Separator();

	// Ground
	ImGui::Text("Ground: (%.2f, %.2f, %.2f) [Static 100x2x100]", m_GroundPosition.x, m_GroundPosition.y, m_GroundPosition.z);
	ImGui::Separator();

	// Dynamic Actors
	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "=== Dynamic Actors ===");
	ImGui::Text("Dynamic Cube: (%.2f, %.2f, %.2f)", m_DynamicCubePosition.x, m_DynamicCubePosition.y, m_DynamicCubePosition.z);
	ImGui::Text("Dynamic Sphere: (%.2f, %.2f, %.2f)", m_DynamicSpherePosition.x, m_DynamicSpherePosition.y, m_DynamicSpherePosition.z);

	// Static Actors
	ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "=== Static Actors ===");
	ImGui::Text("Static Cube: (%.2f, %.2f, %.2f)", m_StaticCubePosition.x, m_StaticCubePosition.y, m_StaticCubePosition.z);
	ImGui::Text("Static Sphere: (%.2f, %.2f, %.2f)", m_StaticSpherePosition.x, m_StaticSpherePosition.y, m_StaticSpherePosition.z);

	// Capsule Controller
	ImGui::Separator();
	ImGui::Text("Capsule: (%.2f, %.2f, %.2f)", m_CapsulePosition.x, m_CapsulePosition.y, m_CapsulePosition.z);
	ImGui::Separator();

	// 속도 정보 (Dynamic Rigid Body만)
	ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "=== Velocities ===");
	if (m_pDynamicCubeActor)
	{
		PxVec3 cubeVel = m_pDynamicCubeActor->getLinearVelocity();
		ImGui::Text("Dynamic Cube Vel: (%.2f, %.2f, %.2f)", cubeVel.x, cubeVel.y, cubeVel.z);
	}
	if (m_pDynamicSphereActor)
	{
		PxVec3 sphereVel = m_pDynamicSphereActor->getLinearVelocity();
		ImGui::Text("Dynamic Sphere Vel: (%.2f, %.2f, %.2f)", sphereVel.x, sphereVel.y, sphereVel.z);
	}

	ImGui::End();

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	m_pSwapChain->Present(0, 0);
}

bool TutorialApp::InitD3D()
{
	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferCount = 1;
	sd.BufferDesc.Width = m_ClientWidth;
	sd.BufferDesc.Height = m_ClientHeight;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = m_hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevel;
	HR_T(D3D11CreateDeviceAndSwapChain(
		NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags,
		NULL, 0, D3D11_SDK_VERSION, &sd, &m_pSwapChain,
		&m_pDevice, &featureLevel, &m_pDeviceContext));

	// Create render target view
	ID3D11Texture2D* pBackBuffer = nullptr;
	HR_T(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer));
	HR_T(m_pDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_pRenderTargetView));
	SAFE_RELEASE(pBackBuffer);

	// Create depth stencil texture
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

	ID3D11Texture2D* pDepthStencil = nullptr;
	HR_T(m_pDevice->CreateTexture2D(&descDepth, nullptr, &pDepthStencil));
	HR_T(m_pDevice->CreateDepthStencilView(pDepthStencil, nullptr, &m_pDepthStencilView));
	SAFE_RELEASE(pDepthStencil);

	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

	// Setup viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)m_ClientWidth;
	vp.Height = (FLOAT)m_ClientHeight;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	m_pDeviceContext->RSSetViewports(1, &vp);

	// Rasterizer State 생성
	D3D11_RASTERIZER_DESC rasterDesc = {};
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.CullMode = D3D11_CULL_BACK;  // 뒷면 제거
	rasterDesc.FrontCounterClockwise = FALSE;
	rasterDesc.DepthClipEnable = TRUE;

	ID3D11RasterizerState* pRasterizerState = nullptr;
	HR_T(m_pDevice->CreateRasterizerState(&rasterDesc, &pRasterizerState));
	m_pDeviceContext->RSSetState(pRasterizerState);
	SAFE_RELEASE(pRasterizerState);

	return true;
}

void TutorialApp::UninitD3D()
{
	SAFE_RELEASE(m_pSamplerLinear);
	SAFE_RELEASE(m_pDepthStencilView);
	SAFE_RELEASE(m_pRenderTargetView);
	SAFE_RELEASE(m_pSwapChain);
	SAFE_RELEASE(m_pDeviceContext);
	SAFE_RELEASE(m_pDevice);
}

bool TutorialApp::InitScene()
{
	// Create constant buffer
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	HR_T(m_pDevice->CreateBuffer(&bd, nullptr, &m_pConstantBuffer));

	// Create geometries
	CreateCube();
	CreateSphere();
	CreateCapsule();

	// Setup projection matrix
	m_World = XMMatrixIdentity();
	m_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, m_ClientWidth / (FLOAT)m_ClientHeight, 0.1f, 1000.0f);

	// Setup camera - 위치와 회전 설정
	m_Camera.m_Position = Vector3(0.0f, 5.0f, -15.0f);
	// Pitch를 약간 아래로 설정 (약 18도, 라디안으로 약 0.31)
	m_Camera.m_Rotation = Vector3(0.3f, 0.0f, 0.0f);

	return true;
}

void TutorialApp::UninitScene()
{
	SAFE_RELEASE(m_pConstantBuffer);
	SAFE_RELEASE(m_pInputLayout);
	SAFE_RELEASE(m_pPixelShader);
	SAFE_RELEASE(m_pVertexShader);

	SAFE_RELEASE(m_pCubeIndexBuffer);
	SAFE_RELEASE(m_pCubeVertexBuffer);
	SAFE_RELEASE(m_pSphereIndexBuffer);
	SAFE_RELEASE(m_pSphereVertexBuffer);
	SAFE_RELEASE(m_pCapsuleIndexBuffer);
	SAFE_RELEASE(m_pCapsuleVertexBuffer);
}

void TutorialApp::CreateCube()
{
	Vertex vertices[] =
	{
		// Front face (Red)
		{ Vector3(-1.0f,  1.0f, -1.0f), Vector3(0.0f, 0.0f, -1.0f), Vector4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ Vector3( 1.0f,  1.0f, -1.0f), Vector3(0.0f, 0.0f, -1.0f), Vector4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ Vector3( 1.0f, -1.0f, -1.0f), Vector3(0.0f, 0.0f, -1.0f), Vector4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ Vector3(-1.0f, -1.0f, -1.0f), Vector3(0.0f, 0.0f, -1.0f), Vector4(1.0f, 0.0f, 0.0f, 1.0f) },

		// Back face (Green)
		{ Vector3(-1.0f,  1.0f, 1.0f), Vector3(0.0f, 0.0f, 1.0f), Vector4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ Vector3( 1.0f,  1.0f, 1.0f), Vector3(0.0f, 0.0f, 1.0f), Vector4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ Vector3( 1.0f, -1.0f, 1.0f), Vector3(0.0f, 0.0f, 1.0f), Vector4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ Vector3(-1.0f, -1.0f, 1.0f), Vector3(0.0f, 0.0f, 1.0f), Vector4(0.0f, 1.0f, 0.0f, 1.0f) },

		// Top face (Blue)
		{ Vector3(-1.0f, 1.0f, -1.0f), Vector3(0.0f, 1.0f, 0.0f), Vector4(0.0f, 0.0f, 1.0f, 1.0f) },
		{ Vector3( 1.0f, 1.0f, -1.0f), Vector3(0.0f, 1.0f, 0.0f), Vector4(0.0f, 0.0f, 1.0f, 1.0f) },
		{ Vector3( 1.0f, 1.0f,  1.0f), Vector3(0.0f, 1.0f, 0.0f), Vector4(0.0f, 0.0f, 1.0f, 1.0f) },
		{ Vector3(-1.0f, 1.0f,  1.0f), Vector3(0.0f, 1.0f, 0.0f), Vector4(0.0f, 0.0f, 1.0f, 1.0f) },

		// Bottom face (Yellow)
		{ Vector3(-1.0f, -1.0f, -1.0f), Vector3(0.0f, -1.0f, 0.0f), Vector4(1.0f, 1.0f, 0.0f, 1.0f) },
		{ Vector3( 1.0f, -1.0f, -1.0f), Vector3(0.0f, -1.0f, 0.0f), Vector4(1.0f, 1.0f, 0.0f, 1.0f) },
		{ Vector3( 1.0f, -1.0f,  1.0f), Vector3(0.0f, -1.0f, 0.0f), Vector4(1.0f, 1.0f, 0.0f, 1.0f) },
		{ Vector3(-1.0f, -1.0f,  1.0f), Vector3(0.0f, -1.0f, 0.0f), Vector4(1.0f, 1.0f, 0.0f, 1.0f) },

		// Left face (Cyan)
		{ Vector3(-1.0f,  1.0f,  1.0f), Vector3(-1.0f, 0.0f, 0.0f), Vector4(0.0f, 1.0f, 1.0f, 1.0f) },
		{ Vector3(-1.0f,  1.0f, -1.0f), Vector3(-1.0f, 0.0f, 0.0f), Vector4(0.0f, 1.0f, 1.0f, 1.0f) },
		{ Vector3(-1.0f, -1.0f, -1.0f), Vector3(-1.0f, 0.0f, 0.0f), Vector4(0.0f, 1.0f, 1.0f, 1.0f) },
		{ Vector3(-1.0f, -1.0f,  1.0f), Vector3(-1.0f, 0.0f, 0.0f), Vector4(0.0f, 1.0f, 1.0f, 1.0f) },

		// Right face (Magenta)
		{ Vector3(1.0f,  1.0f,  1.0f), Vector3(1.0f, 0.0f, 0.0f), Vector4(1.0f, 0.0f, 1.0f, 1.0f) },
		{ Vector3(1.0f,  1.0f, -1.0f), Vector3(1.0f, 0.0f, 0.0f), Vector4(1.0f, 0.0f, 1.0f, 1.0f) },
		{ Vector3(1.0f, -1.0f, -1.0f), Vector3(1.0f, 0.0f, 0.0f), Vector4(1.0f, 0.0f, 1.0f, 1.0f) },
		{ Vector3(1.0f, -1.0f,  1.0f), Vector3(1.0f, 0.0f, 0.0f), Vector4(1.0f, 0.0f, 1.0f, 1.0f) },
	};

	D3D11_BUFFER_DESC bd = {};
	bd.ByteWidth = sizeof(vertices);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = vertices;
	HR_T(m_pDevice->CreateBuffer(&bd, &initData, &m_pCubeVertexBuffer));

	m_VertexBufferStride = sizeof(Vertex);
	m_VertexBufferOffset = 0;

	WORD indices[] =
	{
		0, 1, 2,  0, 2, 3,   // Front
		4, 6, 5,  4, 7, 6,   // Back
		8, 10, 9,  8, 11, 10, // Top
		12, 13, 14,  12, 14, 15, // Bottom
		16, 17, 18,  16, 18, 19, // Left
		20, 22, 21,  20, 23, 22  // Right
	};

	m_nCubeIndices = ARRAYSIZE(indices);

	bd = {};
	bd.ByteWidth = sizeof(indices);
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	initData.pSysMem = indices;
	HR_T(m_pDevice->CreateBuffer(&bd, &initData, &m_pCubeIndexBuffer));

	// Create shaders and input layout (only once)
	if (m_pVertexShader == nullptr)
	{
		ID3D10Blob* vertexShaderBuffer = nullptr;
		HR_T(CompileShaderFromFile(L"../Shaders/96_BasicVertexShader.hlsl", "main", "vs_4_0", &vertexShaderBuffer));
		HR_T(m_pDevice->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
			vertexShaderBuffer->GetBufferSize(), NULL, &m_pVertexShader));

		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		HR_T(m_pDevice->CreateInputLayout(layout, ARRAYSIZE(layout),
			vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_pInputLayout));
		SAFE_RELEASE(vertexShaderBuffer);

		ID3D10Blob* pixelShaderBuffer = nullptr;
		HR_T(CompileShaderFromFile(L"../Shaders/96_BasicPixelShader.hlsl", "main", "ps_4_0", &pixelShaderBuffer));
		HR_T(m_pDevice->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
			pixelShaderBuffer->GetBufferSize(), NULL, &m_pPixelShader));
		SAFE_RELEASE(pixelShaderBuffer);
	}
}

void TutorialApp::CreateSphere()
{
	const float radius = 1.0f;
	const int sliceCount = 32;
	const int stackCount = 16;

	std::vector<Vertex> vertices;
	std::vector<WORD> indices;

	// North pole
	vertices.push_back({ Vector3(0.0f, radius, 0.0f), Vector3(0.0f, 1.0f, 0.0f), Vector4(1.0f, 0.5f, 0.0f, 1.0f) });

	float phiStep = XM_PI / stackCount;
	float thetaStep = 2.0f * XM_PI / sliceCount;

	for (int i = 1; i < stackCount; i++)
	{
		float phi = i * phiStep;
		for (int j = 0; j <= sliceCount; j++)
		{
			float theta = j * thetaStep;
			Vector3 pos;
			pos.x = radius * sinf(phi) * cosf(theta);
			pos.y = radius * cosf(phi);
			pos.z = radius * sinf(phi) * sinf(theta);

			Vector3 normal = pos;
			normal.Normalize();

			vertices.push_back({ pos, normal, Vector4(1.0f, 0.5f, 0.0f, 1.0f) });
		}
	}

	// South pole
	vertices.push_back({ Vector3(0.0f, -radius, 0.0f), Vector3(0.0f, -1.0f, 0.0f), Vector4(1.0f, 0.5f, 0.0f, 1.0f) });

	// North pole cap
	for (int i = 1; i <= sliceCount; i++)
	{
		indices.push_back(0);
		indices.push_back(i + 1);
		indices.push_back(i);
	}

	// Stacks
	int baseIndex = 1;
	int ringVertexCount = sliceCount + 1;
	for (int i = 0; i < stackCount - 2; i++)
	{
		for (int j = 0; j < sliceCount; j++)
		{
			indices.push_back(baseIndex + i * ringVertexCount + j);
			indices.push_back(baseIndex + i * ringVertexCount + j + 1);
			indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);

			indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);
			indices.push_back(baseIndex + i * ringVertexCount + j + 1);
			indices.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
		}
	}

	// South pole cap
	int southPoleIndex = (int)vertices.size() - 1;
	baseIndex = southPoleIndex - ringVertexCount;
	for (int i = 0; i < sliceCount; i++)
	{
		indices.push_back(southPoleIndex);
		indices.push_back(baseIndex + i);
		indices.push_back(baseIndex + i + 1);
	}

	D3D11_BUFFER_DESC bd = {};
	bd.ByteWidth = sizeof(Vertex) * (UINT)vertices.size();
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = vertices.data();
	HR_T(m_pDevice->CreateBuffer(&bd, &initData, &m_pSphereVertexBuffer));

	m_nSphereIndices = (int)indices.size();

	bd = {};
	bd.ByteWidth = sizeof(WORD) * (UINT)indices.size();
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	initData.pSysMem = indices.data();
	HR_T(m_pDevice->CreateBuffer(&bd, &initData, &m_pSphereIndexBuffer));
}

void TutorialApp::CreateCapsule()
{
	const float radius = 1.0f;
	const float halfHeight = 1.0f; // Cylinder height / 2
	const int sliceCount = 32;
	const int stackCount = 8; // Per hemisphere

	std::vector<Vertex> vertices;
	std::vector<WORD> indices;

	// Top hemisphere
	vertices.push_back({ Vector3(0.0f, radius + halfHeight, 0.0f), Vector3(0.0f, 1.0f, 0.0f), Vector4(0.5f, 0.0f, 1.0f, 1.0f) });

	float phiStep = (XM_PI / 2.0f) / stackCount; // Only half sphere
	float thetaStep = 2.0f * XM_PI / sliceCount;

	// Top hemisphere vertices
	for (int i = 1; i <= stackCount; i++)
	{
		float phi = i * phiStep;
		for (int j = 0; j <= sliceCount; j++)
		{
			float theta = j * thetaStep;
			Vector3 pos;
			pos.x = radius * sinf(phi) * cosf(theta);
			pos.y = halfHeight + radius * cosf(phi);
			pos.z = radius * sinf(phi) * sinf(theta);

			Vector3 normal = Vector3(pos.x, pos.y - halfHeight, pos.z);
			normal.Normalize();

			vertices.push_back({ pos, normal, Vector4(0.5f, 0.0f, 1.0f, 1.0f) });
		}
	}

	// Bottom hemisphere vertices
	for (int i = 1; i <= stackCount; i++)
	{
		float phi = i * phiStep;
		for (int j = 0; j <= sliceCount; j++)
		{
			float theta = j * thetaStep;
			Vector3 pos;
			pos.x = radius * sinf(phi) * cosf(theta);
			pos.y = -halfHeight - radius * cosf(phi);
			pos.z = radius * sinf(phi) * sinf(theta);

			Vector3 normal = Vector3(pos.x, pos.y + halfHeight, pos.z);
			normal.Normalize();

			vertices.push_back({ pos, normal, Vector4(0.5f, 0.0f, 1.0f, 1.0f) });
		}
	}

	// Bottom pole
	vertices.push_back({ Vector3(0.0f, -radius - halfHeight, 0.0f), Vector3(0.0f, -1.0f, 0.0f), Vector4(0.5f, 0.0f, 1.0f, 1.0f) });

	// Top cap indices
	for (int i = 1; i <= sliceCount; i++)
	{
		indices.push_back(0);
		indices.push_back(i + 1);
		indices.push_back(i);
	}

	// Top hemisphere stacks
	int baseIndex = 1;
	int ringVertexCount = sliceCount + 1;
	for (int i = 0; i < stackCount - 1; i++)
	{
		for (int j = 0; j < sliceCount; j++)
		{
			indices.push_back(baseIndex + i * ringVertexCount + j);
			indices.push_back(baseIndex + i * ringVertexCount + j + 1);
			indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);

			indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);
			indices.push_back(baseIndex + i * ringVertexCount + j + 1);
			indices.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
		}
	}

	// Cylinder middle section (connecting top and bottom hemispheres)
	int topRingStart = baseIndex + (stackCount - 1) * ringVertexCount;
	int bottomRingStart = topRingStart + ringVertexCount;
	for (int j = 0; j < sliceCount; j++)
	{
		indices.push_back(topRingStart + j);
		indices.push_back(topRingStart + j + 1);
		indices.push_back(bottomRingStart + j);

		indices.push_back(bottomRingStart + j);
		indices.push_back(topRingStart + j + 1);
		indices.push_back(bottomRingStart + j + 1);
	}

	// Bottom hemisphere stacks
	baseIndex = bottomRingStart;
	for (int i = 0; i < stackCount - 1; i++)
	{
		for (int j = 0; j < sliceCount; j++)
		{
			indices.push_back(baseIndex + i * ringVertexCount + j);
			indices.push_back(baseIndex + i * ringVertexCount + j + 1);
			indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);

			indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);
			indices.push_back(baseIndex + i * ringVertexCount + j + 1);
			indices.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
		}
	}

	// Bottom cap indices
	int southPoleIndex = (int)vertices.size() - 1;
	baseIndex = southPoleIndex - ringVertexCount;
	for (int i = 0; i < sliceCount; i++)
	{
		indices.push_back(southPoleIndex);
		indices.push_back(baseIndex + i);
		indices.push_back(baseIndex + i + 1);
	}

	D3D11_BUFFER_DESC bd = {};
	bd.ByteWidth = sizeof(Vertex) * (UINT)vertices.size();
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = vertices.data();
	HR_T(m_pDevice->CreateBuffer(&bd, &initData, &m_pCapsuleVertexBuffer));

	m_nCapsuleIndices = (int)indices.size();

	bd = {};
	bd.ByteWidth = sizeof(WORD) * (UINT)indices.size();
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	initData.pSysMem = indices.data();
	HR_T(m_pDevice->CreateBuffer(&bd, &initData, &m_pCapsuleIndexBuffer));
}

void TutorialApp::OnInputProcess(const Keyboard::State& KeyState, const Keyboard::KeyboardStateTracker& KeyTracker,
	const Mouse::State& MouseState, const Mouse::ButtonStateTracker& MouseTracker)
{
	// 카메라에 입력 전달
	m_Camera.OnInputProcess(KeyState, KeyTracker, MouseState, MouseTracker);
}

LRESULT CALLBACK TutorialApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return __super::WndProc(hWnd, message, wParam, lParam);
}

bool TutorialApp::InitPhysX()
{
	// PhysX Foundation 생성
	m_pFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_Allocator, m_ErrorCallback);
	if (!m_pFoundation)
	{
		LOG_ERRORA("PxCreateFoundation failed!");
		return false;
	}

	// PhysX Visual Debugger (PVD) 설정 (선택사항)
	m_pPvd = PxCreatePvd(*m_pFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
	m_pPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

	// PhysX 생성
	m_pPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_pFoundation, PxTolerancesScale(), true, m_pPvd);
	if (!m_pPhysics)
	{
		LOG_ERRORA("PxCreatePhysics failed!");
		return false;
	}

	// CUDA 활성화 시도
	PxCudaContextManagerDesc cudaContextManagerDesc;
	m_pCudaContextManager = PxCreateCudaContextManager(*m_pFoundation, cudaContextManagerDesc, PxGetProfilerCallback());
	if (m_pCudaContextManager)
	{
		if (m_pCudaContextManager->contextIsValid())
		{
			LOG_MESSAGEA("CUDA Context created successfully!");
		}
		else
		{
			LOG_MESSAGEA("CUDA Context is invalid!");
			m_pCudaContextManager->release();
			m_pCudaContextManager = nullptr;
		}
	}
	else
	{
		LOG_MESSAGEA("CUDA is not available. Using CPU simulation.");
	}

	// Scene Descriptor 설정
	PxSceneDesc sceneDesc(m_pPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	m_pDispatcher = PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher = m_pDispatcher;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;

	// CUDA 사용 설정
	if (m_pCudaContextManager)
	{
		sceneDesc.cudaContextManager = m_pCudaContextManager;
		sceneDesc.flags |= PxSceneFlag::eENABLE_GPU_DYNAMICS;
		sceneDesc.flags |= PxSceneFlag::eENABLE_PCM;
		sceneDesc.broadPhaseType = PxBroadPhaseType::eGPU;
		sceneDesc.gpuMaxNumPartitions = 8;
		LOG_MESSAGEA("GPU acceleration enabled for PhysX scene.");
	}

	// Scene 생성
	m_pScene = m_pPhysics->createScene(sceneDesc);
	if (!m_pScene)
	{
		LOG_ERRORA("createScene failed!");
		return false;
	}

	// Material 생성 (staticFriction, dynamicFriction, restitution)
	m_pMaterial = m_pPhysics->createMaterial(0.5f, 0.5f, 0.6f);

	// Ground Box 생성 (Static Rigid Body) - 큰 박스 형태의 바닥
	// 크기: 50x1x50 (half-extents이므로 실제 크기는 100x2x100)
	// 위치: Y=-1.0 (바닥면이 Y=0에 위치)
	PxTransform groundTransform(PxVec3(0.0f, -1.0f, 0.0f));
	PxBoxGeometry groundGeometry(50.0f, 1.0f, 50.0f);
	m_pGroundBox = PxCreateStatic(*m_pPhysics, groundTransform, groundGeometry, *m_pMaterial);
	m_pScene->addActor(*m_pGroundBox);

	// === Dynamic Actors (물리 시뮬레이션 적용) ===

	// Dynamic Cube Actor 생성
	PxTransform dynamicCubeTransform(PxVec3(-5.0f, 50.0f, 0.0f));
	PxBoxGeometry cubeGeometry(1.0f, 1.0f, 1.0f);
	m_pDynamicCubeActor = PxCreateDynamic(*m_pPhysics, dynamicCubeTransform, cubeGeometry, *m_pMaterial, 10.0f);
	m_pDynamicCubeActor->setAngularDamping(0.5f);
	m_pScene->addActor(*m_pDynamicCubeActor);

	// Dynamic Sphere Actor 생성
	PxTransform dynamicSphereTransform(PxVec3(0.0f,50.0f, 0.0f));
	PxSphereGeometry sphereGeometry(1.0f);
	m_pDynamicSphereActor = PxCreateDynamic(*m_pPhysics, dynamicSphereTransform, sphereGeometry, *m_pMaterial, 10.0f);
	m_pDynamicSphereActor->setAngularDamping(0.5f);
	m_pScene->addActor(*m_pDynamicSphereActor);

	// === Static Actors (고정된 물체) ===

	// Static Cube Actor 생성
	PxTransform staticCubeTransform(PxVec3(-5.0f, 2.0f, -5.0f));
	m_pStaticCubeActor = PxCreateStatic(*m_pPhysics, staticCubeTransform, cubeGeometry, *m_pMaterial);
	m_pScene->addActor(*m_pStaticCubeActor);

	// Static Sphere Actor 생성
	PxTransform staticSphereTransform(PxVec3(0.0f, 2.0f, -5.0f));
	m_pStaticSphereActor = PxCreateStatic(*m_pPhysics, staticSphereTransform, sphereGeometry, *m_pMaterial);
	m_pScene->addActor(*m_pStaticSphereActor);

	// Character Controller Manager 생성
	m_pControllerManager = PxCreateControllerManager(*m_pScene);

	// Capsule Character Controller 생성
	PxCapsuleControllerDesc capsuleDesc;
	capsuleDesc.height = 2.0f;
	capsuleDesc.radius = 1.0f;
	capsuleDesc.material = m_pMaterial;
	capsuleDesc.position = PxExtendedVec3(5.0f, 3.0f, 0.0f);
	capsuleDesc.climbingMode = PxCapsuleClimbingMode::eEASY;
	capsuleDesc.slopeLimit = cosf(XM_PIDIV4);  // 45도
	capsuleDesc.contactOffset = 0.1f;
	capsuleDesc.stepOffset = 0.5f;
	capsuleDesc.density = 10.0f;
	capsuleDesc.reportCallback = &m_ControllerHitReport;

	m_pCapsuleController = m_pControllerManager->createController(capsuleDesc);
	if (!m_pCapsuleController)
	{
		LOG_ERRORA("createController failed!");
		return false;
	}

	// Capsule Controller의 Actor 가져와서 Dynamic Actor와 충돌하도록 설정
	PxRigidDynamic* capsuleActor = m_pCapsuleController->getActor();
	if (capsuleActor)
	{
		// 모든 Shape에 대해 충돌 플래그 설정
		PxShape* shapes[1];
		PxU32 shapeCount = capsuleActor->getShapes(shapes, 1);
		for (PxU32 i = 0; i < shapeCount; i++)
		{
			PxShape* shape = shapes[i];
			// Simulation과 Query 모두 활성화
			shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
			shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
		}
	}

	// 초기 위치 설정
	m_GroundPosition = Vector3(0.0f, -1.0f, 0.0f);
	m_DynamicCubePosition = Vector3(-5.0f, 5.0f, 0.0f);
	m_DynamicSpherePosition = Vector3(0.0f, 5.0f, 0.0f);
	m_StaticCubePosition = Vector3(-5.0f, 2.0f, -5.0f);
	m_StaticSpherePosition = Vector3(0.0f, 2.0f, -5.0f);
	m_CapsulePosition = Vector3(5.0f, 3.0f, 0.0f);

	return true;
}

void TutorialApp::UninitPhysX()
{
	if (m_pCapsuleController)
		m_pCapsuleController->release();

	if (m_pControllerManager)
		m_pControllerManager->release();

	if (m_pScene)
		m_pScene->release();

	if (m_pDispatcher)
		m_pDispatcher->release();

	if (m_pCudaContextManager)
		m_pCudaContextManager->release();

	if (m_pPhysics)
		m_pPhysics->release();

	if (m_pPvd)
	{
		PxPvdTransport* transport = m_pPvd->getTransport();
		m_pPvd->release();
		if (transport)
			transport->release();
	}

	if (m_pFoundation)
		m_pFoundation->release();
}

void TutorialApp::UpdatePhysX(float deltaTime)
{
	if (!m_pScene)
		return;

	// deltaTime이 너무 크면 제한 (프레임 드랍 방지)
	if (deltaTime > 0.1f)
		deltaTime = 0.1f;

	// Capsule Character Controller 이동 처리
	PxVec3 displacement(0.0f, 0.0f, 0.0f);
	float moveSpeed = 5.0f;

	// 카메라의 forward와 right 벡터 가져오기
	Vector3 forward = m_Camera.GetForward();
	Vector3 right = m_Camera.GetRight();

	// Y축 이동 제거 (수평 이동만)
	forward.y = 0.0f;
	forward.Normalize();
	right.y = 0.0f;
	right.Normalize();

	// 화살표 키 입력으로 이동 벡터 계산
	if (GetAsyncKeyState(VK_UP) & 0x8000)
		displacement += PxVec3(forward.x, 0, forward.z) * moveSpeed * deltaTime;
	if (GetAsyncKeyState(VK_DOWN) & 0x8000)
		displacement -= PxVec3(forward.x, 0, forward.z) * moveSpeed * deltaTime;
	if (GetAsyncKeyState(VK_LEFT) & 0x8000)
		displacement -= PxVec3(right.x, 0, right.z) * moveSpeed * deltaTime;
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
		displacement += PxVec3(right.x, 0, right.z) * moveSpeed * deltaTime;

	// 점프 처리 (스페이스바)
	if (GetAsyncKeyState(VK_SPACE) & 0x8000)
	{
		if (m_IsGrounded)
		{
			m_VerticalVelocity = 7.0f; // 점프 초기 속도
			m_IsGrounded = false;
		}
	}

	// 중력 적용
	const float gravity = -9.81f;
	m_VerticalVelocity += gravity * deltaTime;
	displacement.y = m_VerticalVelocity * deltaTime;

	// Controller 이동 (Hit Report 콜백이 자동으로 Dynamic Actor를 밀어냄)
	PxControllerFilters filters;
	PxControllerCollisionFlags collisionFlags = m_pCapsuleController->move(displacement, 0.001f, deltaTime, filters);

	// 바닥에 닿았는지 체크
	if (collisionFlags & PxControllerCollisionFlag::eCOLLISION_DOWN)
	{
		m_IsGrounded = true;
		m_VerticalVelocity = 0.0f;
	}

	// PhysX 시뮬레이션 업데이트 (고정 타임스텝 사용)
	const float fixedTimeStep = 1.0f / 60.0f;  // 60 FPS 고정
	m_pScene->simulate(fixedTimeStep);
	m_pScene->fetchResults(true);

	// Actor 위치 동기화
	// Dynamic Actors (물리 시뮬레이션 적용되므로 위치 업데이트)
	if (m_pDynamicCubeActor)
	{
		PxTransform transform = m_pDynamicCubeActor->getGlobalPose();
		m_DynamicCubePosition = Vector3(transform.p.x, transform.p.y, transform.p.z);
	}

	if (m_pDynamicSphereActor)
	{
		PxTransform transform = m_pDynamicSphereActor->getGlobalPose();
		m_DynamicSpherePosition = Vector3(transform.p.x, transform.p.y, transform.p.z);
	}

	// Static Actors는 위치가 변하지 않으므로 동기화 불필요
	// m_StaticCubePosition, m_StaticSpherePosition은 초기값 유지

	if (m_pCapsuleController)
	{
		PxExtendedVec3 pos = m_pCapsuleController->getPosition();
		m_CapsulePosition = Vector3((float)pos.x, (float)pos.y, (float)pos.z);
	}
}

bool TutorialApp::InitImGUI()
{
	// ImGui 초기화
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// ImGui 스타일 설정
	ImGui::StyleColorsDark();

	// ImGui Win32 및 DX11 백엔드 초기화
	ImGui_ImplWin32_Init(m_hWnd);
	ImGui_ImplDX11_Init(m_pDevice, m_pDeviceContext);

	return true;
}

void TutorialApp::UninitImGUI()
{
	// ImGui 정리
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}
