//--------------------------------------------------------------------------------------
// File: DebugDraw.h
//
// Helpers for drawing various debug shapes using PrimitiveBatch
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//-------------------------------------------------------------------------------------

#pragma once

//https://github.com/microsoft/DirectXTK/wiki/DebugDraw
//https://github.com/walbourn/directx-sdk-samples/blob/main/Collision/Collision.cpp

#include <DirectXColors.h>
#include <DirectXCollision.h>
#include <directxtk/CommonStates.h>
#include <directxtk/PrimitiveBatch.h>
#include <directxtk/Effects.h>
#include <directXTK/VertexTypes.h>

using namespace DirectX;
using namespace DirectX::SimpleMath;

namespace DebugDraw
{
	extern std::unique_ptr<CommonStates>                           g_States;
	extern std::unique_ptr<BasicEffect>                            g_BatchEffect;
	extern std::unique_ptr<PrimitiveBatch<VertexPositionColor>>    g_Batch;
	extern Microsoft::WRL::ComPtr<ID3D11InputLayout>               g_pBatchInputLayout;
	extern Microsoft::WRL::ComPtr<ID3D11DeviceContext>             g_pDeviceContext;


	HRESULT Initialize(Microsoft::WRL::ComPtr<ID3D11Device> device,Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext);
	void Uninitialize();

	void XM_CALLCONV Draw(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch,
		const DirectX::BoundingSphere& sphere,
		DirectX::FXMVECTOR color = DirectX::Colors::White);

	void XM_CALLCONV Draw(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch,
		const DirectX::BoundingBox& box,
		DirectX::FXMVECTOR color = DirectX::Colors::White);

	void XM_CALLCONV Draw(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch,
		const DirectX::BoundingOrientedBox& obb,
		DirectX::FXMVECTOR color = DirectX::Colors::White);

	void XM_CALLCONV Draw(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch,
		const DirectX::BoundingFrustum& frustum,
		DirectX::FXMVECTOR color = DirectX::Colors::White);

	void XM_CALLCONV DrawGrid(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch,
		DirectX::FXMVECTOR xAxis, DirectX::FXMVECTOR yAxis,
		DirectX::FXMVECTOR origin, size_t xdivs, size_t ydivs,
		DirectX::GXMVECTOR color = DirectX::Colors::White);

	void XM_CALLCONV DrawRing(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch,
		DirectX::FXMVECTOR origin, DirectX::FXMVECTOR majorAxis, DirectX::FXMVECTOR minorAxis,
		DirectX::GXMVECTOR color = DirectX::Colors::White);

	void XM_CALLCONV DrawRay(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch,
		DirectX::FXMVECTOR origin, DirectX::FXMVECTOR direction, bool normalize = true,
		DirectX::FXMVECTOR color = DirectX::Colors::White);

	void XM_CALLCONV DrawTriangle(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch,
		DirectX::FXMVECTOR pointA, DirectX::FXMVECTOR pointB, DirectX::FXMVECTOR pointC,
		DirectX::GXMVECTOR color = DirectX::Colors::White);

	void XM_CALLCONV DrawQuad(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch,
		DirectX::FXMVECTOR pointA, DirectX::FXMVECTOR pointB, DirectX::FXMVECTOR pointC, DirectX::GXMVECTOR pointD,
		DirectX::HXMVECTOR color = DirectX::Colors::White);

}

