#pragma once
#include "GameObject.h"
#include <directxtk/SimpleMath.h>

// Windows.h�� min/max ��ũ�θ� �����Ͽ� RTTR���� �浹 ����
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#define RTTR_DLL
#include <rttr/registration>

using namespace DirectX::SimpleMath;

class CubeObject : public GameObject
{
	RTTR_ENABLE(GameObject)
public:
	CubeObject();
	~CubeObject();

	// ����
	Vector4 m_Color;
	std::string m_Name;
	float m_Value;
};
