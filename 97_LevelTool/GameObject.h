#pragma once
#include <directxtk/SimpleMath.h>
#include <DirectXCollision.h>
#include <string>
#include "../Common/json.hpp"

// Windows.h의 min/max 매크로를 해제하여 RTTR과의 충돌 방지
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#define RTTR_DLL
#include <rttr/registration>

using namespace DirectX::SimpleMath;
using namespace DirectX;

class GameObject
{
	RTTR_ENABLE()
public:
	GameObject();
	virtual ~GameObject();

	// Transform 속성
	Vector3 m_Position;
	Vector3 m_Rotation;
	Vector3 m_Scale;

	Matrix m_WorldM;

	// AABB (Axis-Aligned Bounding Box)
	BoundingBox m_AABB;

	// 월드 행렬 계산
	Matrix GetWorldMatrix();

	// AABB 업데이트 (변환 적용)
	void UpdateAABB();

	// 직렬화 - GameObject를 JSON으로 변환
	nlohmann::json Serialize() const;

	// 역직렬화 - JSON에서 GameObject로 복원
	bool Deserialize(const nlohmann::json& jsonObj);
};
