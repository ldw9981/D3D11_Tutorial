#include "pch.h"
#include "Helper.h"
#include <comdef.h>
#include <d3dcompiler.h>
#include <directXTK/DDSTextureLoader.h>
#include <directXTK/WICTextureLoader.h>
#include <dxgidebug.h>
#include <dxgi1_3.h>    // DXGIGetDebugInterface1
#include <DirectXTex.h>

#pragma comment(lib, "dxguid.lib")  // 꼭 필요!
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

	// 필요한 버퍼 크기 계산
	int len = WideCharToMultiByte(CP_ACP, 0, wMsg, -1, nullptr, 0, nullptr, nullptr);

	std::string msg(len, '\0');
	WideCharToMultiByte(CP_ACP, 0, wMsg, -1, &msg[0], len, nullptr, nullptr);

	return msg; // std::string으로 반환 (내부적으로 LPCSTR과 호환)
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

HRESULT CreateTextureFromFile(ID3D11Device* d3dDevice,const wchar_t* szFileName,
	ID3D11ShaderResourceView** textureView, bool isSRGB /*추가*/)
{
	std::filesystem::path path(szFileName);
	std::wstring strExtension = path.extension();
	std::transform(strExtension.begin(), strExtension.end(), strExtension.begin(), ::towlower);

	DirectX::TexMetadata metadata1;
	DirectX::ScratchImage scratchImage;

	HRESULT hr = S_OK;
	if (strExtension == L".dds")
	{
		HR_T(hr = DirectX::LoadFromDDSFile(szFileName, DirectX::DDS_FLAGS_NONE, &metadata1, scratchImage));
	}
	else if (strExtension == L".tga")
	{
		HR_T(hr = DirectX::LoadFromTGAFile(szFileName, &metadata1, scratchImage));
	}
	else if (strExtension == L".hdr")
	{
		HR_T(hr = DirectX::LoadFromHDRFile(szFileName, &metadata1, scratchImage));
	}
	else // 기타..
	{
		HR_T(hr = DirectX::LoadFromWICFile(szFileName, DirectX::WIC_FLAGS_NONE, &metadata1, scratchImage));
	}

	DirectX::CREATETEX_FLAGS flags = DirectX::CREATETEX_FLAGS::CREATETEX_DEFAULT;
	flags |= isSRGB ? DirectX::CREATETEX_FORCE_SRGB : DirectX::CREATETEX_IGNORE_SRGB;

	HR_T(DirectX::CreateShaderResourceViewEx(
		d3dDevice, scratchImage.GetImages(), scratchImage.GetImageCount(), metadata1,
		D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE,
		0, 0,
		flags,
		textureView
	));
	return hr;
}


void CheckDXGIDebug()
{
	IDXGIDebug1* pDebug = nullptr;

	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug))))
	{
		// 현재 살아있는 DXGI/D3D 객체 출력
		pDebug->ReportLiveObjects(
			DXGI_DEBUG_ALL,                 // 모든 DXGI/D3D 컴포넌트
			DXGI_DEBUG_RLO_ALL              // 전체 리포트 옵션
		);

		pDebug->Release();
	}
}
