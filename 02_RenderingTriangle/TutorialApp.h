#pragma once
#include <d3d11.h>
#include "../Common/GameApp.h"

class TutorialApp :
	public GameApp
{
public:
	TutorialApp(HINSTANCE hInstance);
	~TutorialApp();

	// ������ ������������ �����ϴ� �ʼ� ��ü�� �������̽�
	ID3D11Device* m_pDevice = nullptr;						// ����̽�	
	ID3D11DeviceContext* m_pDeviceContext = nullptr;		// ��� ����̽� ���ؽ�Ʈ
	IDXGISwapChain* m_pSwapChain = nullptr;					// ����ü��
	ID3D11RenderTargetView* m_pRenderTargetView = nullptr;	// ������ Ÿ�ٺ�
	// �X�� ���ٽ� �䵵 ������ ���� ������� �ʴ´�.


	// ������ ���������ο� �����ϴ� ���ҽ� ��ü�� �������̽�

	ID3D11VertexShader* m_pVertexShader = nullptr;		// ���� ���̴�.
	ID3D11PixelShader* m_pPixelShader = nullptr;		// �ȼ� ���̴�.
	ID3D11Buffer* m_pVertexBuffer = nullptr;		// ���� ����.
	ID3D11InputLayout* m_pInputLayout = nullptr;	// �Է� ���̾ƿ�.

	UINT m_VertextBufferStride = 0;
	UINT m_VertextBufferOffset = 0;

	virtual bool Initialize(UINT Width, UINT Height);
	virtual void Update();
	virtual void Render();

	bool InitD3D();
	void UninitD3D();

	bool InitScene();		// ���̴�,���ؽ�,�ε���
	void UninitScene();
};

