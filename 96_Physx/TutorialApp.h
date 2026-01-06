#pragma once
#include <windows.h>
#include "../Common/GameApp.h"
#include "../Common/Camera.h"
#include <d3d11.h>
#include <directxtk/SimpleMath.h>
#include <wrl/client.h>

// PhysX headers
#include <physx/PxPhysicsAPI.h>

using namespace Microsoft::WRL;
using namespace DirectX::SimpleMath;
using namespace DirectX;
using namespace physx;

class TutorialApp : public GameApp
{
public:
	// 렌더링 파이프라인을 구성하는 필수 객체의 인터페이스
	ID3D11Device* m_pDevice = nullptr;
	ID3D11DeviceContext* m_pDeviceContext = nullptr;
	IDXGISwapChain* m_pSwapChain = nullptr;
	ID3D11RenderTargetView* m_pRenderTargetView = nullptr;
	ID3D11DepthStencilView* m_pDepthStencilView = nullptr;
	ID3D11SamplerState* m_pSamplerLinear = nullptr;

	// Vertex & Index Buffers
	ID3D11Buffer* m_pCubeVertexBuffer = nullptr;
	ID3D11Buffer* m_pCubeIndexBuffer = nullptr;
	int m_nCubeIndices = 0;

	ID3D11Buffer* m_pSphereVertexBuffer = nullptr;
	ID3D11Buffer* m_pSphereIndexBuffer = nullptr;
	int m_nSphereIndices = 0;

	ID3D11Buffer* m_pCapsuleVertexBuffer = nullptr;
	ID3D11Buffer* m_pCapsuleIndexBuffer = nullptr;
	int m_nCapsuleIndices = 0;

	// Shaders
	ID3D11VertexShader* m_pVertexShader = nullptr;
	ID3D11PixelShader* m_pPixelShader = nullptr;
	ID3D11InputLayout* m_pInputLayout = nullptr;

	// Constant Buffer
	ID3D11Buffer* m_pConstantBuffer = nullptr;

	// Camera
	Camera m_Camera;

	// Matrices
	Matrix m_World;
	Matrix m_View;
	Matrix m_Projection;

	// Stride & Offset
	UINT m_VertexBufferStride = 0;
	UINT m_VertexBufferOffset = 0;

	// PhysX objects
	PxDefaultAllocator m_Allocator;
	PxDefaultErrorCallback m_ErrorCallback;
	PxFoundation* m_pFoundation = nullptr;
	PxPhysics* m_pPhysics = nullptr;
	PxDefaultCpuDispatcher* m_pDispatcher = nullptr;
	PxScene* m_pScene = nullptr;
	PxMaterial* m_pMaterial = nullptr;
	PxPvd* m_pPvd = nullptr;
	PxCudaContextManager* m_pCudaContextManager = nullptr;

	// PhysX Actors
	PxRigidStatic* m_pGroundBox = nullptr;

	// Dynamic Actors
	PxRigidDynamic* m_pDynamicCubeActor = nullptr;
	PxRigidDynamic* m_pDynamicSphereActor = nullptr;

	// Static Actors
	PxRigidStatic* m_pStaticCubeActor = nullptr;
	PxRigidStatic* m_pStaticSphereActor = nullptr;

	PxController* m_pCapsuleController = nullptr;
	PxControllerManager* m_pControllerManager = nullptr;

	// Actor positions
	Vector3 m_GroundPosition;
	Vector3 m_DynamicCubePosition;
	Vector3 m_DynamicSpherePosition;
	Vector3 m_StaticCubePosition;
	Vector3 m_StaticSpherePosition;
	Vector3 m_CapsulePosition;

	bool InitD3D();
	void UninitD3D();

	bool InitScene();
	void UninitScene();

	bool InitPhysX();
	void UninitPhysX();
	void UpdatePhysX(float deltaTime);

	bool InitImGUI();
	void UninitImGUI();

	void CreateCube();
	void CreateSphere();
	void CreateCapsule();

	virtual bool OnInitialize() override;
	virtual void OnUninitialize() override;
	virtual void OnUpdate() override;
	virtual void OnRender() override;
	virtual void OnInputProcess(const Keyboard::State& KeyState, const Keyboard::KeyboardStateTracker& KeyTracker,
		const Mouse::State& MouseState, const Mouse::ButtonStateTracker& MouseTracker) override;

	virtual LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) override;
};
