#include "pch.h"
#include "TestReflection.h"
#include <iostream>

// Windows.h의 min/max 매크로를 해제하여 RTTR과의 충돌 방지
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#define RTTR_DLL
#include <rttr/registration>
#include <rttr/detail/policies/ctor_policies.h>

#ifdef _DEBUG
#pragma comment(lib, "rttr_core_d.lib")
#else
#pragma comment(lib, "rttr_core.lib")
#endif

using namespace rttr;
using namespace std;

class NotRttrClass
{
	public:
	NotRttrClass()
		: m_Value(0)	
	{
	}
	int m_Value;
};


class BaseClass
{
	RTTR_ENABLE()
public:
	BaseClass()
		: m_Position(1.0f, 2.0f, 3.0f)
	{
	}
	Vector3 m_Position;
	int		m_ID;
};

class ChildClass : public BaseClass
{
	RTTR_ENABLE(BaseClass)
public:
	ChildClass()
		: m_Name("Child")
	{
	}
	std::string m_Name;
};

// 하나의 cpp파일에 한번만 작성해야 함
// 컴파일러가 RTTR_REGISTRATION을 전역 변수 초기화 코드로 변환하고, 프로그램 시작 시 main() 이전에 자동 실행한다.
RTTR_REGISTRATION
{
	registration::class_<BaseClass>("BaseClass")
		.constructor<>()
			(rttr::policy::ctor::as_raw_ptr)
		.property("Position", &BaseClass::m_Position)
		.property("ID", &BaseClass::m_ID);

	registration::class_<Vector3>("Vector3")
		.constructor<>()
		.constructor<float, float, float>()
		.property("x", &Vector3::x)
		.property("y", &Vector3::y)
		.property("z", &Vector3::z);


	registration::class_<ChildClass>("ChildClass")
		.constructor<>()
		(rttr::policy::ctor::as_raw_ptr)
		.property("Name", &ChildClass::m_Name);
}

// 전역 설정 초기화 함수
bool InitializeConfig()
{
	cout << "[전역 초기화] 설정 로드 중...\n";
	// 여기서 파일 읽기, 설정 로드 등 수행
	cout << "[전역 초기화] 설정 로드 완료\n";
	return true;
}
// 전역 변수로 초기화 함수 자동 실행
bool g_ConfigLoaded = InitializeConfig();


void PrintRttrClass(const variant& obj)
{
	type t = obj.get_type(); 
	cout << "TypeName " << t.get_name().to_string() <<  endl;

	for (auto& prop : t.get_properties())
	{
		std::string propName = prop.get_name().to_string();
		variant value = prop.get_value(obj);

		cout << " - " << propName << ": ";
		if (value.is_type<float>())
		{
			cout <<  value.get_value<float>();
		}
		else if (value.is_type<int>())
		{
			cout << value.get_value<int>();
		}
		else if (value.is_type<std::string>())
		{
			cout << value.get_value<std::string>();
		}
		else if (value.is_type<Vector3>())
		{
			Vector3 v = value.get_value<Vector3>();
			cout << v.x << ", " << v.y << ", " << v.z;
		}
		else if (value.is_type<Vector4>())
		{
			Vector4 v = value.get_value<Vector4>();
			cout << v.x << ", " << v.y << ", " << v.z << ", " << v.w;
		}
		cout << endl;
	}
}

void TestReflection()
{	
	BaseClass objBaseClass;
	ChildClass objChildClass;
	NotRttrClass objNotRttrClass;

	PrintRttrClass(objBaseClass);
	PrintRttrClass(objChildClass);
	PrintRttrClass(objNotRttrClass);
}
