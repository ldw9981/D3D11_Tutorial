#include "pch.h"
#include "Math.h"

#include <directXTK/SimpleMath.h> // vector, matrix  C++ Class library
#include <DirectXMath.h>	// vector, matrix  C Function library

using namespace DirectX::SimpleMath;
using namespace DirectX;

void TestMath()
{
	DirectX::XMFLOAT3 f3_1 = { 1.0f, 2.0f, 3.0f };
	DirectX::XMFLOAT3 f3_2 = { 4.0f, 5.0f, 6.0f };

	// DirectXMath.h ���� �⺻ �����ڰ� ����. DirectXMath.h �� ����Ѵٸ� XMVector�� ��ȯ�� �����ؾ��Ѵ�. 
	// simplemath.h ���� ������ ����
	f3_1 = DirectX::XMFLOAT3{ 1.0f, 2.0f, 3.0f } + f3_2; 
	f3_1 = DirectX::XMFLOAT3{ 1.0f, 2.0f, 3.0f } + DirectX::XMFLOAT3{ 4.0f, 5.0f, 6.0f };   
		
	DirectX::SimpleMath::Vector3 v1(1.0f, 2.0f, 3.0f);
	DirectX::SimpleMath::Vector4 v2(4.0f, 5.0f, 6.0f, 7.0f);

	// Vector3�� XMFLOAT3�� ��ӹ����Ƿ� ���� ���� ����
	Vector3 v3 = f3_1; 
	f3_2 = v3; 

	// Matrix�� XMFLOAT4X4�� ��ӹ޾� �����Ǿ��ִ�.
	// ������ �迭�̹Ƿ� �����ҿ� �迭�� ���ٰ���
	DirectX::SimpleMath::Matrix m1 = DirectX::SimpleMath::Matrix::Identity;
	XMFLOAT4X4 m2 = { 1.0f, 0.0f, 0.0f, 0.0f,
							0.0f, 1.0f, 0.0f, 0.0f,
							0.0f, 0.0f, 1.0f, 0.0f,
							0.0f, 0.0f, 0.0f, 1.0f };

	// XMFLOAT4�� �ܼ��� float4 4���� ��� �ִ� ����ü�̴�.
	XMFLOAT4 f1 = { 1.0f, 2.0f, 3.0f, 4.0f };
	XMFLOAT4 f2 = { 5.0f, 6.0f, 7.0f, 8.0f };
	XMFLOAT4 result;
	result.x = f1.x + f2.x; // ������ ����
	result.y = f1.y + f2.y;
	result.z = f1.z + f2.z;
	result.w = f1.w + f2.w;


	// XMVector�� float4���� ���ÿ� ������ �ִ� CPU�� 128��Ʈ �������� Ÿ�� __m128 �� ����Ѵ�.	
	// CPU�� �� __m128�� SIMD(single instruction, multiple data) ��ɾ ����Ͽ� ��� ������ �����Ѵ�. 	
	// XMFLOAT4�� XMVector�� �ٸ� Ÿ���̹Ƿ� ȣȯ���� ������ ������ ������ �Լ��� ���� ���԰����Ѵ�.
	XMVECTOR xmv1 = DirectX::XMLoadFloat4(&v2); // XMFLOAT4 -> XMVECTOR
	XMVECTOR xmv2 = XMVectorSet(5.0f, 6.0f, 7.0f, 8.0f);
	XMVECTOR vxresult = XMVectorAdd(xmv1, xmv2); // SIMD ����. x,y,z,w ���Һ��� ���ÿ� ����

	DirectX::XMStoreFloat4(&result, vxresult); // XMVECTOR -> XMFLOAT4

	// XMMATRIX�� 4���� XMVector�� ��� �ִ�.	
	DirectX::XMMATRIX xmm1 = DirectX::XMMatrixIdentity();
		
	float data = xmm1.r[0].m128_f32[0]; // XMMATRIX�� �� ���ҿ� �迭�� ���ٰ���
	m1 = xmm1; // XMMATRIX -> Matrix ������ �� ���� ���԰���
	// m2 = xmm1; // XMMATRIX -> XMFLOAT4X4  ������ ����. �Ұ���
}
