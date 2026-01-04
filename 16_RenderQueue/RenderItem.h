#pragma once

struct RenderItem
{
	// 월드 행렬
	Matrix World world;
	//D3D11_BUFFER* vertexBuffer = nullptr;
	UINT vertexBufferStride = 0;
	UINT vertexBufferOffset = 0;

};

