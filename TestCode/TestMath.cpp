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

	// DirectXMath.h 에는 기본 연산자가 없다. DirectXMath.h 만 사용한다면 XMVector로 변환후 연산해야한다. 
	// simplemath.h 에서 연산자 제공
	f3_1 = DirectX::XMFLOAT3{ 1.0f, 2.0f, 3.0f } + f3_2; 
	f3_1 = DirectX::XMFLOAT3{ 1.0f, 2.0f, 3.0f } + DirectX::XMFLOAT3{ 4.0f, 5.0f, 6.0f };   
		
	DirectX::SimpleMath::Vector3 v1(1.0f, 2.0f, 3.0f);
	DirectX::SimpleMath::Vector4 v2(4.0f, 5.0f, 6.0f, 7.0f);

	// Vector3는 XMFLOAT3를 상속받으므로 서로 대입 가능
	Vector3 v3 = f3_1; 
	f3_2 = v3; 

	// Matrix는 XMFLOAT4X4를 상속받아 구현되어있다.
	// 연속적 배열이므로 각원소에 배열로 접근가능
	DirectX::SimpleMath::Matrix m1 = DirectX::SimpleMath::Matrix::Identity;
	XMFLOAT4X4 m2 = { 1.0f, 0.0f, 0.0f, 0.0f,
							0.0f, 1.0f, 0.0f, 0.0f,
							0.0f, 0.0f, 1.0f, 0.0f,
							0.0f, 0.0f, 0.0f, 1.0f };

	// XMFLOAT4는 단순히 float4 4개를 담고 있는 구조체이다.
	XMFLOAT4 f1 = { 1.0f, 2.0f, 3.0f, 4.0f };
	XMFLOAT4 f2 = { 5.0f, 6.0f, 7.0f, 8.0f };
	XMFLOAT4 result;
	result.x = f1.x + f2.x; // 순차적 연산
	result.y = f1.y + f2.y;
	result.z = f1.z + f2.z;
	result.w = f1.w + f2.w;


	// XMVector는 float4개를 동시에 담을수 있는 CPU의 128비트 레지스터 타입 __m128 을 사용한다.	
	// CPU는 이 __m128을 SIMD(single instruction, multiple data) 명령어를 사용하여 고속 연산을 지원한다. 	
	// XMFLOAT4는 XMVector는 다른 타입이므로 호환되지 않으며 별도로 구현된 함수에 의해 대입가능한다.
	XMVECTOR xmv1 = DirectX::XMLoadFloat4(&v2); // XMFLOAT4 -> XMVECTOR
	XMVECTOR xmv2 = XMVectorSet(5.0f, 6.0f, 7.0f, 8.0f);
	XMVECTOR vxresult = XMVectorAdd(xmv1, xmv2); // SIMD 연산. x,y,z,w 원소별로 동시에 연산

	DirectX::XMStoreFloat4(&result, vxresult); // XMVECTOR -> XMFLOAT4

	// XMMATRIX는 4개의 XMVector를 담고 있다.	
	DirectX::XMMATRIX xmm1 = DirectX::XMMatrixIdentity();
		
	float data = xmm1.r[0].m128_f32[0]; // XMMATRIX의 각 원소에 배열로 접근가능
	m1 = xmm1; // XMMATRIX -> Matrix 연산자 에 의해 대입가능
	// m2 = xmm1; // XMMATRIX -> XMFLOAT4X4  연산자 없음. 불가능
}
