#pragma once


#include "../Common/GameApp.h"

#include <d3d11.h>
#include <dxgi.h>
#include <d2d1_1.h>
#include <dwrite.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <objbase.h>
#include <wincodec.h>

#include <directxtk/DDSTextureLoader.h>
#include <directxtk/WICTextureLoader.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "windowscodecs.lib")

class TutorialApp : public GameApp
{
public:
	TutorialApp();

	bool OnInitialize() override;
	void OnUninitialize() override;
	void OnUpdate() override;
	void OnRender() override;

	LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) override;

private:
	struct CubeVertex
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT2 texCoord;
	};

	struct cbPerObject
	{
		DirectX::XMMATRIX WVP;
	};

	bool InitD3D();
	void UninitD3D();
	bool InitD2D_D3D11_DWrite();
	void UninitD2D_DWrite();
	bool InitScene();
	void RenderText(const wchar_t* text);
	HRESULT CreateD2DBitmapFromFile(const wchar_t* fileName, ID2D1Bitmap1** outBitmap);
	HRESULT CreateTextureFromFile(ID3D11Device* d3dDevice, const wchar_t* szFileName, ID3D11ShaderResourceView** textureView);
	HRESULT CompileShaderFromFile(const WCHAR* szFileName, const D3D_SHADER_MACRO* pDefines, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);

private:
	// D3D11
	IDXGISwapChain* m_SwapChain = nullptr;
	ID3D11Device* m_Device = nullptr;
	ID3D11DeviceContext* m_DeviceContext = nullptr;
	ID3D11RenderTargetView* m_RenderTargetView = nullptr;
	ID3D11Texture2D* m_DepthStencilBuffer = nullptr;
	ID3D11DepthStencilView* m_DepthStencilView = nullptr;

	ID3D11Buffer* m_SquareIndexBuffer = nullptr;
	ID3D11Buffer* m_SquareVertBuffer = nullptr;
	ID3D11VertexShader* m_VS = nullptr;
	ID3D11PixelShader* m_PS = nullptr;
	ID3DBlob* m_VS_Buffer = nullptr;
	ID3DBlob* m_PS_Buffer = nullptr;
	ID3D11InputLayout* m_VertLayout = nullptr;
	ID3D11Buffer* m_CBPerObjectBuffer = nullptr;
	ID3D11BlendState* m_Transparency = nullptr;
	ID3D11RasterizerState* m_CCWcullMode = nullptr;
	ID3D11RasterizerState* m_CWcullMode = nullptr;
	ID3D11ShaderResourceView* m_CubesTexture = nullptr;
	ID3D11SamplerState* m_CubesTexSamplerState = nullptr;

	// D2D/DWrite
	ID2D1Factory1* m_D2DFactory = nullptr;
	ID2D1Device* m_D2DDevice = nullptr;
	ID2D1DeviceContext* m_D2DDeviceContext = nullptr;
	ID2D1Bitmap1* m_D2DTargetBitmap = nullptr;
	ID2D1SolidColorBrush* m_Brush = nullptr;
	IDWriteFactory* m_DWriteFactory = nullptr;
	IDWriteTextFormat* m_TextFormat = nullptr;
	ID2D1Bitmap1* m_TestTreeBitmap = nullptr;

	ID3D11Texture2D* m_SharedTex11 = nullptr;
	ID3D11ShaderResourceView* m_D2DTexture = nullptr;
	ID3D11Buffer* m_D2DVertBuffer = nullptr;
	ID3D11Buffer* m_D2DIndexBuffer = nullptr;

	bool m_ComInitialized = false;

	// Scene
	DirectX::XMMATRIX m_WVP;
	DirectX::XMMATRIX m_Cube1World;
	DirectX::XMMATRIX m_Cube2World;
	DirectX::XMMATRIX m_CamView;
	DirectX::XMMATRIX m_CamProjection;

	DirectX::XMMATRIX m_Rotation;
	DirectX::XMMATRIX m_Scale;
	DirectX::XMMATRIX m_Translation;
	float m_Rot = 0.01f;

	cbPerObject m_CBPerObj = {};
};
