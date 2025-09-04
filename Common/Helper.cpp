#include "pch.h"
#include "Helper.h"
#include <comdef.h>
#include <d3dcompiler.h>
#include <directXTK/DDSTextureLoader.h>
#include <directXTK/WICTextureLoader.h>
#include <dxgidebug.h>
#include <dxgi1_3.h>    // DXGIGetDebugInterface1

#pragma comment(lib, "dxguid.lib")  // �� �ʿ�!
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")




LPCWSTR GetComErrorString(HRESULT hr)
{
	_com_error err(hr);
	LPCWSTR errMsg = err.ErrorMessage();
	return errMsg;
}

std::string GetComErrorStringA(HRESULT hr)
{
	_com_error err(hr);
	LPCWSTR wMsg = err.ErrorMessage();

	// �ʿ��� ���� ũ�� ���
	int len = WideCharToMultiByte(CP_ACP, 0, wMsg, -1, nullptr, 0, nullptr, nullptr);

	std::string msg(len, '\0');
	WideCharToMultiByte(CP_ACP, 0, wMsg, -1, &msg[0], len, nullptr, nullptr);

	return msg; // std::string���� ��ȯ (���������� LPCSTR�� ȣȯ)
}



HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;

	// Disable optimizations to further improve shader debugging
	dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ID3DBlob* pErrorBlob = nullptr;
	hr = D3DCompileFromFile(szFileName, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, szEntryPoint, szShaderModel,
		dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			MessageBoxA(NULL, (char*)pErrorBlob->GetBufferPointer(), "CompileShaderFromFile", MB_OK);
			pErrorBlob->Release();
		}
		return hr;
	}
	if (pErrorBlob) pErrorBlob->Release();

	return S_OK;
}

HRESULT CreateTextureFromFile(ID3D11Device* d3dDevice, const wchar_t* szFileName, ID3D11ShaderResourceView** textureView)
{
	HRESULT hr = S_OK;

	// Load the Texture
	hr = DirectX::CreateDDSTextureFromFile(d3dDevice, szFileName, nullptr, textureView);
	if (FAILED(hr))
	{
		hr = DirectX::CreateWICTextureFromFile(d3dDevice, szFileName, nullptr, textureView);
		if (FAILED(hr))
		{
			MessageBoxW(NULL, GetComErrorString(hr), szFileName, MB_OK);
			return hr;
		}
	}
	return S_OK;
}



void CheckDXGIDebug()
{
	IDXGIDebug1* pDebug = nullptr;

	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug))))
	{
		// ���� ����ִ� DXGI/D3D ��ü ���
		pDebug->ReportLiveObjects(
			DXGI_DEBUG_ALL,                 // ��� DXGI/D3D ������Ʈ
			DXGI_DEBUG_RLO_ALL              // ��ü ����Ʈ �ɼ�
		);

		pDebug->Release();
	}
}