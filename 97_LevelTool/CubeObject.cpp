#include "CubeObject.h"
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#define RTTR_DLL
#include <rttr/registration>

using namespace rttr;


RTTR_REGISTRATION
{
	registration::class_<CubeObject>("CubeObject")
		.constructor<>()		
			(policy::ctor::as_raw_ptr)
		.property("Color", &CubeObject::m_Color)
			(metadata("desc", "This is Color of Cube"))
		.property("Name", &CubeObject::m_Name)
		.property("Value", &CubeObject::m_Value);
}

CubeObject::CubeObject()
	: GameObject()
	, m_Color(1.0f, 1.0f, 1.0f, 1.0f), m_Name("Cube"), m_Value(0.0f)
{
}

CubeObject::~CubeObject()
{
}
