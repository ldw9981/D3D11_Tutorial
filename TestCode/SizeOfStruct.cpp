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
int a: 4����Ʈ, 4����Ʈ ����
float b: 4����Ʈ, 4����Ʈ ����
char d: 1����Ʈ, 1����Ʈ ����
bool e: 1����Ʈ, 1����Ʈ ����
MyMixedData�� ���� ū ���� �䱸������ int�� float�� 4����Ʈ. ���� ��ü ũ��� 4�� ����� �Ǿ�� �մϴ�.
���� ũ��: 4 + 4 + 1 + 1 = 10����Ʈ

10����Ʈ�� 4�� ����� �ƴϹǷ�, �����Ϸ��� �������� 2����Ʈ�� �е��� �߰��մϴ�.
MyMixedData�� ���� ũ��� 12����Ʈ�Դϴ�.
*/


struct MyMixedData2
{
	MyMixedData a;  //?
	double b;	// 8Byte
};
/*
MyMixedData a : ũ�� 12����Ʈ, ���� �䱸���� 4����Ʈ(���� ū ����� ���� �䱸������ ����).
double c : ũ�� 8����Ʈ, ���� �䱸���� 8����Ʈ.
MyMixedData2�� ���� ū ���� �䱸������ double c�� 8����Ʈ. ���� ��ü ũ��� 8�� ����� �Ǿ�� �մϴ�.

MyMixedData a : 12����Ʈ
double c : 8����Ʈ.a�� 12����Ʈ�� ���������Ƿ� c�� ������ 12���� �����մϴ�.
������ double�� 8�� ��� �ּҿ� ���ĵǾ�� �ϹǷ�, ������ 12�� �ùٸ��� �ʽ��ϴ�.

�����Ϸ��� a�� c ���̿� 4����Ʈ�� �е��� �߰��Ͽ� c�� ���� �ּҸ� 16���� ����ϴ�.
���� ũ�� : 12 + 4(�е�)+8 = 24����Ʈ
24����Ʈ�� 8�� ����̹Ƿ� �߰����� �е� ���� ����ü ũ��� 24����Ʈ�� �˴ϴ�.
*/

struct Test1
{
	float a;	   	 // 4
	Math::Vector4 b; // ũ��� 16����Ʈ, ������� ũ��� 4����Ʈ
	float c;         // 4
};// 24 (4+16+4)

struct alignas(16) Test2  // alignas(16) ����ϸ� ��ü ũ�Ⱑ 16����Ʈ ����� ������
{
	float a;
	float b;
	Math::Vector3 c; // ũ��� 12����Ʈ, ������� ũ��� 4����Ʈ
	float d;
	// ���ʿ� 8����Ʈ �е� �߰�
};

// ���� ����ü ��ü 16����Ʈ ������������� �� ����� ���� �䱸������ �ٸ���.
struct alignas(16) Test3  // alignas�� ����� ����� ������ �� ����� ���� �䱸������ ����ü ��ü�� ����˴ϴ�.
{
	float a;	   	 
	float b;
	// b���Ŀ� 8����Ʈ �е� �߰�
	alignas(16) Math::Vector3 c;	// �̸���� ���� �䱸������ 16����Ʈ 
	float d;         
};  

struct alignas(16) Test4  // alignas�� ����� ����� ������ �� ����� ���� �䱸������ ����ü ��ü�� ����˴ϴ�.
{
	float a;
	float b;
	// b���Ŀ� 8����Ʈ �е� �߰�
	alignas(16) Math::Vector2 c;	// �̸���� ���� �䱸������ 16����Ʈ 
	alignas(16) Math::Vector2 d;	// �̸���� ���� �䱸������ 16����Ʈ 
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
