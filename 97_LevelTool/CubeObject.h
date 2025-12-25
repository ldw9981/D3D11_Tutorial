#pragma once
#include "GameObject.h"
#include <directxtk/SimpleMath.h>

// Windows.h의 min/max 매크로를 해제하여 RTTR과의 충돌 방지
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include <rttr/registration>

using namespace DirectX::SimpleMath;

class CubeObject : public GameObject
{
	RTTR_ENABLE(GameObject)
public:
	CubeObject();
	~CubeObject();

	// 색상
	Vector4 m_Color;
	std::string m_Name;
	float m_Value;
};
