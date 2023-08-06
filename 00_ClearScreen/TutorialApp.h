#pragma once
#include "../Common/GameApp.h"
#include <d3d11.h>

class TutorialApp :
    public GameApp
{
public:
	TutorialApp(HINSTANCE hInstance);
	~TutorialApp();

	// DirectX ����.
	ID3D11Device* pDevice = nullptr;						// ����̽�	
	ID3D11DeviceContext* pDeviceContext = nullptr;			// ����̽� ���ؽ�Ʈ
	IDXGISwapChain* pSwapChain = nullptr;					// ����ü��
	ID3D11RenderTargetView* pRenderTargetView = nullptr;	// ������ Ÿ�ٺ�

	virtual bool Initialize(UINT Width, UINT Height);	// ������ ������ ���� ���� �ٸ��� �����Ƿ� ���,����,���̱⸸ �Ѵ�.
	virtual void Update();
	virtual void Render();

	bool InitD3D();
	void UninitD3D();
};

