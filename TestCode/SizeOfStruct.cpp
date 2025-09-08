#include "pch.h"
#include "SizeOfStruct.h"

struct MyMixedData
{
	int a;	//4Byte
	float b; //4Byte
	
	char c;  //1Byte?
	bool d;  //1Byte?
};
/*
int a: 4바이트, 4바이트 정렬
float b: 4바이트, 4바이트 정렬
char d: 1바이트, 1바이트 정렬
bool e: 1바이트, 1바이트 정렬
MyMixedData의 가장 큰 정렬 요구사항은 int와 float의 4바이트. 따라서 전체 크기는 4의 배수가 되어야 합니다.
현재 크기: 4 + 4 + 1 + 1 = 10바이트

10바이트는 4의 배수가 아니므로, 컴파일러는 마지막에 2바이트의 패딩을 추가합니다.
MyMixedData의 최종 크기는 12바이트입니다.
*/


struct MyMixedData2
{
	MyMixedData a;  //?
	double b;	// 8Byte
};
/*
MyMixedData a : 크기 12바이트, 정렬 요구사항 4바이트(가장 큰 멤버의 정렬 요구사항을 따름).
double c : 크기 8바이트, 정렬 요구사항 8바이트.
MyMixedData2의 가장 큰 정렬 요구사항은 double c의 8바이트. 따라서 전체 크기는 8의 배수가 되어야 합니다.

MyMixedData a : 12바이트
double c : 8바이트.a가 12바이트를 차지했으므로 c는 오프셋 12에서 시작합니다.
하지만 double은 8의 배수 주소에 정렬되어야 하므로, 오프셋 12는 올바르지 않습니다.

컴파일러는 a와 c 사이에 4바이트의 패딩을 추가하여 c의 시작 주소를 16으로 맞춥니다.
현재 크기 : 12 + 4(패딩)+8 = 24바이트
24바이트는 8의 배수이므로 추가적인 패딩 없이 구조체 크기는 24바이트가 됩니다.
*/

struct Test1
{
	float a;	   	 // 4
	Math::Vector4 b; // 크기는 16바이트, 경계정렬 크기는 4바이트
	float c;         // 4
};// 24 (4+16+4)

struct alignas(16) Test2  // alignas(16) 사용하면 전체 크기가 16바이트 배수로 맞춰짐
{
	float a;
	float b;
	Math::Vector3 c; // 크기는 12바이트, 경계정렬 크기는 4바이트
	float d;
	// 뒤쪽에 8바이트 패딩 추가
};

// 같은 구조체 전체 16바이트 경계정렬이지만 각 멤버의 정렬 요구사항이 다르다.
struct alignas(16) Test3  // alignas가 적용된 멤버가 있으면 그 멤버의 정렬 요구사항이 구조체 전체에 적용됩니다.
{
	float a;	   	 
	float b;
	// b이후에 8바이트 패딩 추가
	alignas(16) Math::Vector3 c;	// 이멤버는 정렬 요구사항이 16바이트 
	float d;         
};  

struct alignas(16) Test4  // alignas가 적용된 멤버가 있으면 그 멤버의 정렬 요구사항이 구조체 전체에 적용됩니다.
{
	float a;
	float b;
	// b이후에 8바이트 패딩 추가
	alignas(16) Math::Vector2 c;	// 이멤버는 정렬 요구사항이 16바이트 
	alignas(16) Math::Vector2 d;	// 이멤버는 정렬 요구사항이 16바이트 
};

void SizeOfStruct()
{
	std::cout <<  "\nMyMixedData" << " " << sizeof(MyMixedData) << "\n";
	std::cout << "alignof(MyMixedData) = " << alignof(MyMixedData) << "\n";
	std::cout << "offset of a = " << offsetof(MyMixedData, a) << "\n";
	std::cout << "offset of b = " << offsetof(MyMixedData, b) << "\n";
	std::cout << "offset of c = " << offsetof(MyMixedData, c) << "\n";
	std::cout << "offset of d = " << offsetof(MyMixedData, d) << "\n";

	std::cout << "\nMyMixedData2" << " " << sizeof(MyMixedData2) << "\n";
	std::cout << "alignof(MyMixedData2) = " << alignof(MyMixedData2) << "\n";
	std::cout << "offset of a = " << offsetof(MyMixedData2, a) << "\n";
	std::cout << "offset of b = " << offsetof(MyMixedData2, b) << "\n";

	std::cout << "\nTest2" << " " << sizeof(Test2) << "\n";
	std::cout << "alignof(Test2) = " << alignof(Test2) << "\n";
	std::cout << "offset of a = " << offsetof(Test2, a) << "\n";
	std::cout << "offset of b = " << offsetof(Test2, b) << "\n";
	std::cout << "offset of c = " << offsetof(Test2, c) << "\n";
	std::cout << "offset of d = " << offsetof(Test2, d) << "\n";


	std::cout << "\nTest3" << " " << sizeof(Test3) << "\n";
	std::cout << "alignof(Test3) = " << alignof(Test3) << "\n";
	std::cout << "offset of a = " << offsetof(Test3, a) << "\n";
	std::cout << "offset of b = " << offsetof(Test3, b) << "\n";
	std::cout << "offset of c = " << offsetof(Test3, c) << "\n";
	std::cout << "offset of d = " << offsetof(Test3, d) << "\n";


	std::cout << "\nTest4" << " " << sizeof(Test4) << "\n";
	std::cout << "alignof(Test4) = " << alignof(Test4) << "\n";
	std::cout << "offset of a = " << offsetof(Test4, a) << "\n";
	std::cout << "offset of b = " << offsetof(Test4, b) << "\n";
	std::cout << "offset of c = " << offsetof(Test4, c) << "\n";
	std::cout << "offset of d = " << offsetof(Test4, d) << "\n";
}
