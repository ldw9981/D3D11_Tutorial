#include "pch.h"
#include "SizeOfStruct.h"


struct Test1
{
	char a;
	float b;
	char c;

};

struct Test2
{
	char a;
	char* k;
	double d;
	char e;
	char* k2;
	double f;
};


void SizeOfStruct()
{
	// char 1byte, short 2byte,int 4byte, double 8byte
	int size1 = sizeof(Test1);
	int size2 = sizeof(Test2);
	std::cout << "Hello World!\n";
}
