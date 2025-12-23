//--------------------------------------------------------------------------------------
// File: DebugDraw.cpp
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//-------------------------------------------------------------------------------------

#include "pch.h"
#include "DebugDraw.h"
#include <algorithm>





namespace DebugDraw
{
	std::unique_ptr<CommonStates>                           g_States=nullptr;
	std::unique_ptr<BasicEffect>                            g_BatchEffect = nullptr;
	std::unique_ptr<PrimitiveBatch<VertexPositionColor>>    g_Batch = nullptr;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>               g_pBatchInputLayout = nullptr;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>             g_pDeviceContext = nullptr;



	HRESULT Initialize(Microsoft::WRL::ComPtr<ID3D11Device> device,Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext)
	{
		HRESULT hr = S_OK;
		g_States = std::make_unique<CommonStates>(device.Get());
		g_Batch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(deviceContext.Get());

		g_BatchEffect = std::make_unique<BasicEffect>(device.Get());
		g_BatchEffect->SetVertexColorEnabled(true);
		{
			void const* shaderByteCode;
			size_t byteCodeLength;

			g_BatchEffect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

			hr = device->CreateInputLayout(VertexPositionColor::InputElements,
				VertexPositionColor::InputElementCount,
				shaderByteCode, byteCodeLength,
				&g_pBatchInputLayout);
			if (FAILED(hr))
				return hr;
		}
		return S_OK;
	}

	void Uninitialize()
	{
		g_States = nullptr;
		g_BatchEffect = nullptr;
		g_Batch = nullptr;
		g_pBatchInputLayout = nullptr;
		g_pDeviceContext = nullptr;
	}

	inline void XM_CALLCONV DrawCube(PrimitiveBatch<VertexPositionColor>* batch,
		CXMMATRIX matWorld,
		FXMVECTOR color)
	{
		static const XMVECTORF32 s_verts[8] =
		{
			{ { { -1.f, -1.f, -1.f, 0.f } } },
			{ { {  1.f, -1.f, -1.f, 0.f } } },
			{ { {  1.f, -1.f,  1.f, 0.f } } },
			{ { { -1.f, -1.f,  1.f, 0.f } } },
			{ { { -1.f,  1.f, -1.f, 0.f } } },
			{ { {  1.f,  1.f, -1.f, 0.f } } },
			{ { {  1.f,  1.f,  1.f, 0.f } } },
			{ { { -1.f,  1.f,  1.f, 0.f } } }
		};

		static const WORD s_indices[] =
		{
			0, 1,
			1, 2,
			2, 3,
			3, 0,
			4, 5,
			5, 6,
			6, 7,
			7, 4,
			0, 4,
			1, 5,
			2, 6,
			3, 7
		};

		VertexPositionColor verts[8];
		for (size_t i = 0; i < 8; ++i)
		{
			XMVECTOR v = XMVector3Transform(s_verts[i], matWorld);
			XMStoreFloat3(&verts[i].position, v);
			XMStoreFloat4(&verts[i].color, color);
		}

		batch->DrawIndexed(D3D_PRIMITIVE_TOPOLOGY_LINELIST, s_indices, static_cast<UINT>(std::size(s_indices)), verts, 8);
	}


	void XM_CALLCONV Draw(PrimitiveBatch<VertexPositionColor>* batch,
		const BoundingSphere& sphere,
		FXMVECTOR color)
	{
		XMVECTOR origin = XMLoadFloat3(&sphere.Center);

		const float radius = sphere.Radius;

		XMVECTOR xaxis = g_XMIdentityR0 * radius;
		XMVECTOR yaxis = g_XMIdentityR1 * radius;
		XMVECTOR zaxis = g_XMIdentityR2 * radius;

		DrawRing(batch, origin, xaxis, zaxis, color);
		DrawRing(batch, origin, xaxis, yaxis, color);
		DrawRing(batch, origin, yaxis, zaxis, color);
	}


	void XM_CALLCONV Draw(PrimitiveBatch<VertexPositionColor>* batch,
		const BoundingBox& box,
		FXMVECTOR color)
	{
		XMMATRIX matWorld = XMMatrixScaling(box.Extents.x, box.Extents.y, box.Extents.z);
		XMVECTOR position = XMLoadFloat3(&box.Center);
		matWorld.r[3] = XMVectorSelect(matWorld.r[3], position, g_XMSelect1110);

		DrawCube(batch, matWorld, color);
	}


	void XM_CALLCONV Draw(PrimitiveBatch<VertexPositionColor>* batch,
		const BoundingOrientedBox& obb,
		FXMVECTOR color)
	{
		XMMATRIX matWorld = XMMatrixRotationQuaternion(XMLoadFloat4(&obb.Orientation));
		XMMATRIX matScale = XMMatrixScaling(obb.Extents.x, obb.Extents.y, obb.Extents.z);
		matWorld = XMMatrixMultiply(matScale, matWorld);
		XMVECTOR position = XMLoadFloat3(&obb.Center);
		matWorld.r[3] = XMVectorSelect(matWorld.r[3], position, g_XMSelect1110);

		DrawCube(batch, matWorld, color);
	}


	void XM_CALLCONV Draw(PrimitiveBatch<VertexPositionColor>* batch,
		const BoundingFrustum& frustum,
		FXMVECTOR color)
	{
		XMFLOAT3 corners[BoundingFrustum::CORNER_COUNT];
		frustum.GetCorners(corners);

		VertexPositionColor verts[24] = {};
		verts[0].position = corners[0];
		verts[1].position = corners[1];
		verts[2].position = corners[1];
		verts[3].position = corners[2];
		verts[4].position = corners[2];
		verts[5].position = corners[3];
		verts[6].position = corners[3];
		verts[7].position = corners[0];

		verts[8].position = corners[0];
		verts[9].position = corners[4];
		verts[10].position = corners[1];
		verts[11].position = corners[5];
		verts[12].position = corners[2];
		verts[13].position = corners[6];
		verts[14].position = corners[3];
		verts[15].position = corners[7];

		verts[16].position = corners[4];
		verts[17].position = corners[5];
		verts[18].position = corners[5];
		verts[19].position = corners[6];
		verts[20].position = corners[6];
		verts[21].position = corners[7];
		verts[22].position = corners[7];
		verts[23].position = corners[4];

		for (size_t j = 0; j < std::size(verts); ++j)
		{
			XMStoreFloat4(&verts[j].color, color);
		}

		batch->Draw(D3D_PRIMITIVE_TOPOLOGY_LINELIST, verts, static_cast<UINT>(std::size(verts)));
	}


	void XM_CALLCONV DrawGrid(PrimitiveBatch<VertexPositionColor>* batch,
		FXMVECTOR xAxis,
		FXMVECTOR yAxis,
		FXMVECTOR origin,
		size_t xdivs,
		size_t ydivs,
		GXMVECTOR color)
	{
		xdivs = std::max<size_t>(1, xdivs);
		ydivs = std::max<size_t>(1, ydivs);

		for (size_t i = 0; i <= xdivs; ++i)
		{
			float percent = float(i) / float(xdivs);
			percent = (percent * 2.f) - 1.f;
			XMVECTOR scale = XMVectorScale(xAxis, percent);
			scale = XMVectorAdd(scale, origin);

			VertexPositionColor v1(XMVectorSubtract(scale, yAxis), color);
			VertexPositionColor v2(XMVectorAdd(scale, yAxis), color);
			batch->DrawLine(v1, v2);
		}

		for (size_t i = 0; i <= ydivs; i++)
		{
			FLOAT percent = float(i) / float(ydivs);
			percent = (percent * 2.f) - 1.f;
			XMVECTOR scale = XMVectorScale(yAxis, percent);
			scale = XMVectorAdd(scale, origin);

			VertexPositionColor v1(XMVectorSubtract(scale, xAxis), color);
			VertexPositionColor v2(XMVectorAdd(scale, xAxis), color);
			batch->DrawLine(v1, v2);
		}
	}


	void XM_CALLCONV DrawRing(PrimitiveBatch<VertexPositionColor>* batch,
		FXMVECTOR origin,
		FXMVECTOR majorAxis,
		FXMVECTOR minorAxis,
		GXMVECTOR color)
	{
		static const size_t c_ringSegments = 32;

		VertexPositionColor verts[c_ringSegments + 1];

		FLOAT fAngleDelta = XM_2PI / float(c_ringSegments);
		// Instead of calling cos/sin for each segment we calculate
		// the sign of the angle delta and then incrementally calculate sin
		// and cosine from then on.
		XMVECTOR cosDelta = XMVectorReplicate(cosf(fAngleDelta));
		XMVECTOR sinDelta = XMVectorReplicate(sinf(fAngleDelta));
		XMVECTOR incrementalSin = XMVectorZero();
		static const XMVECTORF32 s_initialCos =
		{
			1.f, 1.f, 1.f, 1.f
		};
		XMVECTOR incrementalCos = s_initialCos.v;
		for (size_t i = 0; i < c_ringSegments; i++)
		{
			XMVECTOR pos = XMVectorMultiplyAdd(majorAxis, incrementalCos, origin);
			pos = XMVectorMultiplyAdd(minorAxis, incrementalSin, pos);
			XMStoreFloat3(&verts[i].position, pos);
			XMStoreFloat4(&verts[i].color, color);
			// Standard formula to rotate a vector.
			XMVECTOR newCos = incrementalCos * cosDelta - incrementalSin * sinDelta;
			XMVECTOR newSin = incrementalCos * sinDelta + incrementalSin * cosDelta;
			incrementalCos = newCos;
			incrementalSin = newSin;
		}
		verts[c_ringSegments] = verts[0];

		batch->Draw(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, verts, c_ringSegments + 1);
	}


	void XM_CALLCONV DrawRay(PrimitiveBatch<VertexPositionColor>* batch,
		FXMVECTOR origin,
		FXMVECTOR direction,
		bool normalize,
		FXMVECTOR color)
	{
		VertexPositionColor verts[3];
		XMStoreFloat3(&verts[0].position, origin);

		XMVECTOR normDirection = XMVector3Normalize(direction);
		XMVECTOR rayDirection = (normalize) ? normDirection : direction;

		XMVECTOR perpVector = XMVector3Cross(normDirection, g_XMIdentityR1);

		if (XMVector3Equal(XMVector3LengthSq(perpVector), g_XMZero))
		{
			perpVector = XMVector3Cross(normDirection, g_XMIdentityR2);
		}
		perpVector = XMVector3Normalize(perpVector);

		XMStoreFloat3(&verts[1].position, XMVectorAdd(rayDirection, origin));
		perpVector = XMVectorScale(perpVector, 0.0625f);
		normDirection = XMVectorScale(normDirection, -0.25f);
		rayDirection = XMVectorAdd(perpVector, rayDirection);
		rayDirection = XMVectorAdd(normDirection, rayDirection);
		XMStoreFloat3(&verts[2].position, XMVectorAdd(rayDirection, origin));

		XMStoreFloat4(&verts[0].color, color);
		XMStoreFloat4(&verts[1].color, color);
		XMStoreFloat4(&verts[2].color, color);

		batch->Draw(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, verts, 2);
	}


	void XM_CALLCONV DrawTriangle(PrimitiveBatch<VertexPositionColor>* batch,
		FXMVECTOR pointA,
		FXMVECTOR pointB,
		FXMVECTOR pointC,
		GXMVECTOR color)
	{
		VertexPositionColor verts[4];
		XMStoreFloat3(&verts[0].position, pointA);
		XMStoreFloat3(&verts[1].position, pointB);
		XMStoreFloat3(&verts[2].position, pointC);
		XMStoreFloat3(&verts[3].position, pointA);

		XMStoreFloat4(&verts[0].color, color);
		XMStoreFloat4(&verts[1].color, color);
		XMStoreFloat4(&verts[2].color, color);
		XMStoreFloat4(&verts[3].color, color);

		batch->Draw(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, verts, 4);
	}


	void XM_CALLCONV DrawQuad(PrimitiveBatch<VertexPositionColor>* batch,
		FXMVECTOR pointA,
		FXMVECTOR pointB,
		FXMVECTOR pointC,
		GXMVECTOR pointD,
		HXMVECTOR color)
	{
		VertexPositionColor verts[5];
		XMStoreFloat3(&verts[0].position, pointA);
		XMStoreFloat3(&verts[1].position, pointB);
		XMStoreFloat3(&verts[2].position, pointC);
		XMStoreFloat3(&verts[3].position, pointD);
		XMStoreFloat3(&verts[4].position, pointA);

		XMStoreFloat4(&verts[0].color, color);
		XMStoreFloat4(&verts[1].color, color);
		XMStoreFloat4(&verts[2].color, color);
		XMStoreFloat4(&verts[3].color, color);
		XMStoreFloat4(&verts[4].color, color);

		batch->Draw(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, verts, 5);
	}



	//--------------------------------------------------------------------------------------
	void DrawGrid(FXMVECTOR xAxis, FXMVECTOR yAxis, FXMVECTOR origin, size_t xdivs, size_t ydivs, GXMVECTOR color)
	{
		auto context = g_pDeviceContext.Get();
		g_BatchEffect->Apply(context);

		context->IASetInputLayout(g_pBatchInputLayout.Get());

		g_Batch->Begin();

		xdivs = std::max<size_t>(1, xdivs);
		ydivs = std::max<size_t>(1, ydivs);

		for (size_t i = 0; i <= xdivs; ++i)
		{
			float fPercent = float(i) / float(xdivs);
			fPercent = (fPercent * 2.0f) - 1.0f;
			XMVECTOR vScale = XMVectorScale(xAxis, fPercent);
			vScale = XMVectorAdd(vScale, origin);

			VertexPositionColor v1(XMVectorSubtract(vScale, yAxis), color);
			VertexPositionColor v2(XMVectorAdd(vScale, yAxis), color);
			g_Batch->DrawLine(v1, v2);
		}

		for (size_t i = 0; i <= ydivs; i++)
		{
			FLOAT fPercent = float(i) / float(ydivs);
			fPercent = (fPercent * 2.0f) - 1.0f;
			XMVECTOR vScale = XMVectorScale(yAxis, fPercent);
			vScale = XMVectorAdd(vScale, origin);

			VertexPositionColor v1(XMVectorSubtract(vScale, xAxis), color);
			VertexPositionColor v2(XMVectorAdd(vScale, xAxis), color);
			g_Batch->DrawLine(v1, v2);
		}

		g_Batch->End();
	}


	//--------------------------------------------------------------------------------------
	void DrawFrustum(const BoundingFrustum& frustum, FXMVECTOR color)
	{
		XMFLOAT3 corners[BoundingFrustum::CORNER_COUNT];
		frustum.GetCorners(corners);

		VertexPositionColor verts[24];
		verts[0].position = corners[0];
		verts[1].position = corners[1];
		verts[2].position = corners[1];
		verts[3].position = corners[2];
		verts[4].position = corners[2];
		verts[5].position = corners[3];
		verts[6].position = corners[3];
		verts[7].position = corners[0];

		verts[8].position = corners[0];
		verts[9].position = corners[4];
		verts[10].position = corners[1];
		verts[11].position = corners[5];
		verts[12].position = corners[2];
		verts[13].position = corners[6];
		verts[14].position = corners[3];
		verts[15].position = corners[7];

		verts[16].position = corners[4];
		verts[17].position = corners[5];
		verts[18].position = corners[5];
		verts[19].position = corners[6];
		verts[20].position = corners[6];
		verts[21].position = corners[7];
		verts[22].position = corners[7];
		verts[23].position = corners[4];

		for (size_t j = 0; j < _countof(verts); ++j)
		{
			XMStoreFloat4(&verts[j].color, color);
		}

		auto context = g_pDeviceContext.Get();
		g_BatchEffect->Apply(context);

		context->IASetInputLayout(g_pBatchInputLayout.Get());

		g_Batch->Begin();

		g_Batch->Draw(D3D11_PRIMITIVE_TOPOLOGY_LINELIST, verts, _countof(verts));

		g_Batch->End();
	}

	//--------------------------------------------------------------------------------------
	void DrawCube(CXMMATRIX mWorld, FXMVECTOR color)
	{
		static const XMVECTOR s_verts[8] =
		{
			{ -1, -1, -1, 0 },
			{ 1, -1, -1, 0 },
			{ 1, -1, 1, 0 },
			{ -1, -1, 1, 0 },
			{ -1, 1, -1, 0 },
			{ 1, 1, -1, 0 },
			{ 1, 1, 1, 0 },
			{ -1, 1, 1, 0 }
		};
		static const WORD s_indices[] =
		{
			0, 1,
			1, 2,
			2, 3,
			3, 0,
			4, 5,
			5, 6,
			6, 7,
			7, 4,
			0, 4,
			1, 5,
			2, 6,
			3, 7
		};

		VertexPositionColor verts[8];
		for (int i = 0; i < 8; ++i)
		{
			XMVECTOR v = XMVector3Transform(s_verts[i], mWorld);
			XMStoreFloat3(&verts[i].position, v);
			XMStoreFloat4(&verts[i].color, color);
		}

		auto context = g_pDeviceContext.Get();
		g_BatchEffect->Apply(context);

		context->IASetInputLayout(g_pBatchInputLayout.Get());

		g_Batch->Begin();

		g_Batch->DrawIndexed(D3D11_PRIMITIVE_TOPOLOGY_LINELIST, s_indices, _countof(s_indices), verts, 8);

		g_Batch->End();
	}


	//--------------------------------------------------------------------------------------
	void DrawAabb(const BoundingBox& box, FXMVECTOR color)
	{
		XMMATRIX matWorld = XMMatrixScaling(box.Extents.x, box.Extents.y, box.Extents.z);
		XMVECTOR position = XMLoadFloat3(&box.Center);
		matWorld.r[3] = XMVectorSelect(matWorld.r[3], position, g_XMSelect1110);

		DrawCube(matWorld, color);
	}


	//--------------------------------------------------------------------------------------
	void DrawObb(const BoundingOrientedBox& obb, FXMVECTOR color)
	{
		XMMATRIX matWorld = XMMatrixRotationQuaternion(XMLoadFloat4(&obb.Orientation));
		XMMATRIX matScale = XMMatrixScaling(obb.Extents.x, obb.Extents.y, obb.Extents.z);
		matWorld = XMMatrixMultiply(matScale, matWorld);
		XMVECTOR position = XMLoadFloat3(&obb.Center);
		matWorld.r[3] = XMVectorSelect(matWorld.r[3], position, g_XMSelect1110);

		DrawCube(matWorld, color);
	}


	//--------------------------------------------------------------------------------------
	void DrawRing(FXMVECTOR Origin, FXMVECTOR MajorAxis, FXMVECTOR MinorAxis, CXMVECTOR color)
	{
		static const DWORD dwRingSegments = 32;

		VertexPositionColor verts[dwRingSegments + 1];

		FLOAT fAngleDelta = XM_2PI / (float)dwRingSegments;
		// Instead of calling cos/sin for each segment we calculate
		// the sign of the angle delta and then incrementally calculate sin
		// and cosine from then on.
		XMVECTOR cosDelta = XMVectorReplicate(cosf(fAngleDelta));
		XMVECTOR sinDelta = XMVectorReplicate(sinf(fAngleDelta));
		XMVECTOR incrementalSin = XMVectorZero();
		static const XMVECTOR initialCos =
		{
			1.0f, 1.0f, 1.0f, 1.0f
		};
		XMVECTOR incrementalCos = initialCos;
		for (DWORD i = 0; i < dwRingSegments; i++)
		{
			XMVECTOR Pos;
			Pos = XMVectorMultiplyAdd(MajorAxis, incrementalCos, Origin);
			Pos = XMVectorMultiplyAdd(MinorAxis, incrementalSin, Pos);
			XMStoreFloat3(&verts[i].position, Pos);
			XMStoreFloat4(&verts[i].color, color);
			// Standard formula to rotate a vector.
			XMVECTOR newCos = incrementalCos * cosDelta - incrementalSin * sinDelta;
			XMVECTOR newSin = incrementalCos * sinDelta + incrementalSin * cosDelta;
			incrementalCos = newCos;
			incrementalSin = newSin;
		}
		verts[dwRingSegments] = verts[0];

		// Draw ring
		auto context = g_pDeviceContext.Get();
		g_BatchEffect->Apply(context);

		context->IASetInputLayout(g_pBatchInputLayout.Get());

		g_Batch->Begin();

		g_Batch->Draw(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP, verts, dwRingSegments + 1);

		g_Batch->End();
	}


	//--------------------------------------------------------------------------------------
	void DrawSphere(const BoundingSphere& sphere, FXMVECTOR color)
	{
		XMVECTOR origin = XMLoadFloat3(&sphere.Center);

		const float fRadius = sphere.Radius;

		XMVECTOR xaxis = g_XMIdentityR0 * fRadius;
		XMVECTOR yaxis = g_XMIdentityR1 * fRadius;
		XMVECTOR zaxis = g_XMIdentityR2 * fRadius;

		DrawRing(origin, xaxis, zaxis, color);
		DrawRing(origin, xaxis, yaxis, color);
		DrawRing(origin, yaxis, zaxis, color);
	}


	//--------------------------------------------------------------------------------------
	void DrawRay(FXMVECTOR Origin, FXMVECTOR Direction, bool bNormalize, FXMVECTOR color)
	{
		VertexPositionColor verts[3];
		XMStoreFloat3(&verts[0].position, Origin);

		XMVECTOR NormDirection = XMVector3Normalize(Direction);
		XMVECTOR RayDirection = (bNormalize) ? NormDirection : Direction;

		XMVECTOR PerpVector = XMVector3Cross(NormDirection, g_XMIdentityR1);

		if (XMVector3Equal(XMVector3LengthSq(PerpVector), g_XMZero))
		{
			PerpVector = XMVector3Cross(NormDirection, g_XMIdentityR2);
		}
		PerpVector = XMVector3Normalize(PerpVector);

		XMStoreFloat3(&verts[1].position, XMVectorAdd(RayDirection, Origin));
		PerpVector = XMVectorScale(PerpVector, 0.0625f);
		NormDirection = XMVectorScale(NormDirection, -0.25f);
		RayDirection = XMVectorAdd(PerpVector, RayDirection);
		RayDirection = XMVectorAdd(NormDirection, RayDirection);
		XMStoreFloat3(&verts[2].position, XMVectorAdd(RayDirection, Origin));

		XMStoreFloat4(&verts[0].color, color);
		XMStoreFloat4(&verts[1].color, color);
		XMStoreFloat4(&verts[2].color, color);

		auto context = g_pDeviceContext.Get();
		g_BatchEffect->Apply(context);

		context->IASetInputLayout(g_pBatchInputLayout.Get());

		g_Batch->Begin();

		g_Batch->Draw(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP, verts, 2);

		g_Batch->End();
	}


	//--------------------------------------------------------------------------------------
	void DrawTriangle(FXMVECTOR PointA, FXMVECTOR PointB, FXMVECTOR PointC, CXMVECTOR color)
	{
		VertexPositionColor verts[4];
		XMStoreFloat3(&verts[0].position, PointA);
		XMStoreFloat3(&verts[1].position, PointB);
		XMStoreFloat3(&verts[2].position, PointC);
		XMStoreFloat3(&verts[3].position, PointA);

		XMStoreFloat4(&verts[0].color, color);
		XMStoreFloat4(&verts[1].color, color);
		XMStoreFloat4(&verts[2].color, color);
		XMStoreFloat4(&verts[3].color, color);

		auto context = g_pDeviceContext.Get();
		g_BatchEffect->Apply(context);

		context->IASetInputLayout(g_pBatchInputLayout.Get());

		g_Batch->Begin();

		g_Batch->Draw(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP, verts, 4);

		g_Batch->End();
	}

}
