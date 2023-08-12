#pragma once
#include <windows.h>
#include "../Common/GameApp.h"
#include <d3d11.h>
#include <directxtk/SimpleMath.h>


using namespace DirectX::SimpleMath;
using namespace DirectX;

class TutorialApp :
    public GameApp
{
public:
	TutorialApp(HINSTANCE hInstance);
	~TutorialApp();

	// ������ ������������ �����ϴ� �ʼ� ��ü�� �������̽�
	ID3D11Device* m_pDevice = nullptr;			// ����̽�
	ID3D11DeviceContext* m_pDeviceContext = nullptr;	// ��� ����̽� ���ؽ�Ʈ
	IDXGISwapChain* m_pSwapChain = nullptr;		// ����ü��
	ID3D11RenderTargetView* m_pRenderTargetView = nullptr;	// ������ Ÿ�ٺ�
	ID3D11DepthStencilView* m_pDepthStencilView = nullptr;	// ����/���ٽ� ��

	// ������ ���������ο� �����ϴ� ���ҽ� ��ü�� �������̽�
	ID3D11Buffer* m_pVertexBuffer = nullptr;		// ���� ����.
	ID3D11Buffer* m_pIndexBuffer = nullptr;
	ID3D11VertexShader* m_pVertexShader = nullptr;		// ���� ���̴�.
	ID3D11PixelShader* m_pPixelShader = nullptr;		// �ȼ� ���̴�.
	ID3D11PixelShader* m_pPixelShaderSolid = nullptr;		// �ȼ� ���̴�.
	ID3D11InputLayout* m_pInputLayout = nullptr;	// �Է� ���̾ƿ�.
	ID3D11Buffer* m_pConstantBuffer = nullptr;	// ��� ����.
	ID3D11ShaderResourceView* m_pTextureRV = nullptr;
	ID3D11SamplerState* m_pSamplerLinear = nullptr;

	int m_nIndices = 0;							// �ε��� ����.
	Matrix                m_World;				// ������ǥ�� �������� ��ȯ�� ���� ���.
	Matrix                m_View;				// ����ǥ�� �������� ��ȯ�� ���� ���.
	Matrix                m_Projection;			// ������ġ��ǥ��( Normalized Device Coordinate) �������� ��ȯ�� ���� ���.
	Vector4              m_vMeshColor = {0.7f, 0.7f, 0.7f, 1.0f};

	virtual bool Initialize(UINT Width, UINT Height);
	virtual void Update();
	virtual void Render();

	bool InitD3D();			
	void UninitD3D();

	bool InitScene();		// ���̴�,���ؽ�,�ε���
	void UninitScene();		 
};
