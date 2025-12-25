#include "GameObject.h"

// Windows.h의 min/max 매크로를 해제하여 RTTR과의 충돌 방지
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include <rttr/registration>

using namespace rttr;

RTTR_REGISTRATION
{
	registration::class_<GameObject>("GameObject")
		.constructor<>()
		.property("Position", &GameObject::m_Position)
		.property("Rotation", &GameObject::m_Rotation)
		.property("Scale", &GameObject::m_Scale);

	// Vector3 타입 등록
	registration::class_<Vector3>("Vector3")
		.constructor<>()
		.constructor<float, float, float>()
		.property("x", &Vector3::x)
		.property("y", &Vector3::y)
		.property("z", &Vector3::z);

	// Vector4 타입 등록
	registration::class_<Vector4>("Vector4")
		.constructor<>()
		.constructor<float, float, float, float>()
		.property("x", &Vector4::x)
		.property("y", &Vector4::y)
		.property("z", &Vector4::z)
		.property("w", &Vector4::w);
}

GameObject::GameObject()
	: m_Position(0.0f, 0.0f, 0.0f)
	, m_Rotation(0.0f, 0.0f, 0.0f)
	, m_Scale(1.0f, 1.0f, 1.0f)
	, m_AABB(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.5f, 0.5f, 0.5f))
{
}

GameObject::~GameObject()
{
}

Matrix GameObject::GetWorldMatrix()
{
	// Scale -> Rotation -> Translation 순서로 변환
	Matrix scaleMatrix = Matrix::CreateScale(m_Scale);
	Matrix rotationMatrix = Matrix::CreateFromYawPitchRoll(m_Rotation.y, m_Rotation.x, m_Rotation.z);
	Matrix translationMatrix = Matrix::CreateTranslation(m_Position);

	m_WorldM = scaleMatrix * rotationMatrix * translationMatrix;
	return m_WorldM;
}

void GameObject::UpdateAABB()
{
	// 기본 AABB를 변환하여 새로운 AABB 생성
	BoundingBox originalAABB(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.5f, 0.5f, 0.5f));
	
	// Scale 적용
	Vector3 scaledExtents = originalAABB.Extents * m_Scale;
	
	// Position 적용 (Rotation은 AABB에 직접 적용하지 않음 - AABB는 축 정렬되어 있어야 함)
	m_AABB.Center = m_Position;
	m_AABB.Extents = scaledExtents;
}
