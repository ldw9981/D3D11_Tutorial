#include "pch.h"
#include "TestRttr.h"
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
	int m_Value=-1;
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
	int		m_ID=0;

	void TestFunction()
	{
		cout << "BaseClass::TestFunction 호출" << endl;
	}
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
// 컴파일러가 RTTR_REGISTRATION을 전역 변수 초기화 코드로 변환하고,
// 프로그램 시작 시 main() 이전에 자동 실행한다.
RTTR_REGISTRATION
{
	// rttr::policy::ctor::as_raw_ptr 설명:
	// - 생성자를 호출하면 힙(heap)에 객체를 생성하고 raw pointer(T*)를 반환
	// - invoke() 호출 시: new BaseClass()와 동일하게 동작
	// - 기본 정책(as_object)과 다르게 포인터로 반환되므로 delete로 수동 메모리 해제 필요
	//
	// 정책 비교:
	// 1. as_raw_ptr:  variant inst = ctor.invoke() → BaseClass* p = inst.get_value<BaseClass*>(); delete p;
	// 2. as_object:   variant inst = ctor.invoke() → BaseClass obj = inst.get_value<BaseClass>(); (자동 소멸)
	// 3. as_std_shared_ptr: variant inst = ctor.invoke() → shared_ptr<BaseClass> sp = inst.get_value<shared_ptr<BaseClass>>(); (자동 해제)
	registration::class_<BaseClass>("BaseClass")
		.constructor<>()
			(rttr::policy::ctor::as_raw_ptr)
		.property("Position", &BaseClass::m_Position)
		.property("ID", &BaseClass::m_ID)
		.method("TestFunction",&BaseClass::TestFunction);

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
	cout << "ClassName: " << t.get_name().to_string() <<  endl;

	// 멤버변수(property) 목록 출력
	for (auto& prop : t.get_properties())
	{
		std::string propName = prop.get_name().to_string();
		variant value = prop.get_value(obj);

		cout << " - Property: " << propName << ": ";
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

	// 멤버함수(method) 목록 출력
	for (auto& meth : t.get_methods())
	{
		std::string methName = meth.get_name().to_string();
		cout << " - Method: " << methName << endl;
	}
}

void TestRttr()
{	
	BaseClass objBaseClass;
	ChildClass objChildClass;
	NotRttrClass objNotRttrClass;

	// 출력 결과를 통해 RTTR이 등록된 클래스만 속성 정보를 출력하는 것을 확인할 수 있다.
	cout << "\n=== RTTR propery/method ===" << endl;
	PrintRttrClass(objBaseClass);
	PrintRttrClass(objChildClass);
	PrintRttrClass(objNotRttrClass);

	// RTTR 기능을 통해 멤버변수 Write 및 Read 테스트
	cout << "\n=== RTTR Write/Read 테스트 ===" << endl;

	// 1. 속성 가져오기
	type t = type::get(objBaseClass);
	property propID = t.get_property("ID");
	property propPosition = t.get_property("Position");

	// 2. 원래 값 읽기
	cout << "\n[원래 값]" << endl;
	variant idValue = propID.get_value(objBaseClass);
	cout << "ID: " << idValue.get_value<int>() << endl;

	variant posValue = propPosition.get_value(objBaseClass);
	Vector3 pos = posValue.get_value<Vector3>();
	cout << "Position: " << pos.x << ", " << pos.y << ", " << pos.z << endl;

	// 3. 속성 값 쓰기 (Write)
	cout << "\n[값 변경]" << endl;
	bool success1 = propID.set_value(objBaseClass, 999);
	cout << "ID 변경 성공: " << (success1 ? "Yes" : "No") << endl;

	Vector3 newPos(10.0f, 20.0f, 30.0f);
	bool success2 = propPosition.set_value(objBaseClass, newPos);
	cout << "Position 변경 성공: " << (success2 ? "Yes" : "No") << endl;

	// 4. 변경된 값 읽기 (Read)
	cout << "\n[변경된 값]" << endl;
	cout << "ID: " << propID.get_value(objBaseClass).get_value<int>() << endl;

	Vector3 changedPos = propPosition.get_value(objBaseClass).get_value<Vector3>();
	cout << "Position: " << changedPos.x << ", " << changedPos.y << ", " << changedPos.z << endl;

	// 5. 문자열 이름으로 동적(실행시간) 접근 테스트
	cout << "\n[동적 속성 접근 테스트]" << endl;
	string propName = "ID";
	property dynamicProp = t.get_property(propName);
	if (dynamicProp.is_valid())
	{
		cout << "'" << propName << "' 속성 찾음: " << dynamicProp.get_value(objBaseClass).to_string() << endl;
	}

	// 6. 메서드 호출 테스트
	cout << "\n[메서드 호출 테스트]" << endl;
	method meth = t.get_method("TestFunction");
	if (meth.is_valid())
	{
		variant result = meth.invoke(objBaseClass);
	}

	// 7. 힙 인스턴스 동적 생성 (new)
	cout << "\n=== 힙 인스턴스 동적 생성 ===" << endl;

	// 타입 이름으로 클래스 찾기
	type baseType = type::get_by_name("BaseClass");
	if (baseType.is_valid())
	{
		cout << "타입 찾음: " << baseType.get_name().to_string() << endl;

		// 생성자로 힙에 인스턴스 생성
		constructor ctor = baseType.get_constructor();
		if (ctor.is_valid())
		{
			variant newInstance = ctor.invoke();  // new BaseClass() 와 동일
			cout << "인스턴스 생성 성공!" << endl;

			// 생성된 인스턴스는 포인터로 관리됨
			if (newInstance.is_valid())
			{
				// 포인터로 변환
				BaseClass* pObj = newInstance.get_value<BaseClass*>();
				cout << "생성된 객체 주소: " << pObj << endl;

				// 속성 접근
				property pos = baseType.get_property("Position");
				Vector3 currentPos = pos.get_value(newInstance).get_value<Vector3>();
				cout << "초기 Position: " << currentPos.x << ", " << currentPos.y << ", " << currentPos.z << endl;

				// 속성 변경
				Vector3 newPos2(100.0f, 200.0f, 300.0f);
				pos.set_value(newInstance, newPos2);

				Vector3 changedPos2 = pos.get_value(newInstance).get_value<Vector3>();
				cout << "변경된 Position: " << changedPos2.x << ", " << changedPos2.y << ", " << changedPos2.z << endl;

				// 메모리 해제 (중요!)
				delete pObj;
				cout << "메모리 해제 완료" << endl;
			}
		}
	}

	// 8. 타입 이름으로 ChildClass 동적 생성
	cout << "\n=== ChildClass 동적 생성 ===" << endl;
	type childType = type::get_by_name("ChildClass");
	if (childType.is_valid())
	{
		constructor childCtor = childType.get_constructor();
		variant childInstance = childCtor.invoke();

		if (childInstance.is_valid())
		{
			ChildClass* pChild = childInstance.get_value<ChildClass*>();
			cout << "ChildClass 생성 성공: " << pChild << endl;

			// 상속된 속성 + 자체 속성 모두 접근 가능
			property nameProp = childType.get_property("Name");
			string name = nameProp.get_value(childInstance).get_value<string>();
			cout << "Name: " << name << endl;

			// Name 변경
			nameProp.set_value(childInstance, string("DynamicChild"));
			cout << "변경된 Name: " << nameProp.get_value(childInstance).get_value<string>() << endl;

			delete pChild;
			cout << "ChildClass 메모리 해제 완료" << endl;
		}
	}
}
