#pragma once
#include <windows.h>
#include "../Common/GameApp.h"
#include <d3d11.h>
#include <directxtk/SimpleMath.h>
using namespace DirectX::SimpleMath;


class TutorialApp :
    public GameApp
{
public:
	TutorialApp(HINSTANCE hInstance);
	~TutorialApp();

	// DirectX ����.
	ID3D11Device* m_pDevice = nullptr;			// ����̽�
	ID3D11DeviceContext* m_pDeviceContext = nullptr;	// ��� ����̽� ���ؽ�Ʈ
	IDXGISwapChain* m_pSwapChain = nullptr;		// ����ü��
	ID3D11RenderTargetView* m_pRenderTargetView = nullptr;	// ������ Ÿ�ٺ�

	ID3D11VertexShader* m_pVertexShader = nullptr;		// ���ؽ� ���̴�.
	ID3D11PixelShader* m_pPixelShader = nullptr;		// �ȼ� ���̴�.
	ID3D11Buffer* m_pVertexBuffer = nullptr;		// ���� ����.
	ID3D11Buffer* m_pIndexBuffer = nullptr;		// �ε��� ����.
	ID3D11InputLayout* m_pInputLayout = nullptr;	// �Է� ���̾ƿ�.
	
	int nIndices=0;							// �ε��� ����.

	virtual bool Initialize(UINT Width, UINT Height);
	virtual void Update();
	virtual void Render();

	bool InitD3D();			
	void UninitD3D();

	bool InitScene();		// ���̴�,���ؽ�,�ε���
	void UninitScene();		 
};

