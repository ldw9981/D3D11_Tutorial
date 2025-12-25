#pragma once
#include "GameObject.h"
#include <directxtk/SimpleMath.h>


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

	// 속성
	Vector4 m_Color;
	std::string m_Name;
	float m_Value;
};
