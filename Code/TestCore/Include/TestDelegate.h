#pragma once
#include <CoreMinimal/Delegate.h>

using Fuko::TDelegate;

int Print(int a)
{
	std::cout << a << std::endl;
	return a;
}

int PrintFour(int a, float b, const char* c, float d)
{
	std::cout
		<< a << ", "
		<< b << ", "
		<< c << ", "
		<< d << ", "
		<< std::endl;
	return a;
}

struct TestClass
{
	int PrintOne(int a)
	{
		std::cout << a << std::endl;
		return a;
	}
	int PrintFour(int a, float b, const char* c, double d)
	{
		std::cout << a << ", " << b << ", " << c << ", " << d << std::endl;
		return a;
	}

	int PrintOneC(int a) const
	{
		std::cout << "const: " << a << std::endl;
		return a;
	}
	int PrintFourC(int a, float b, const char* c, double d) const
	{
		std::cout << "const: " << a << ", " << b << ", " << c << ", " << d << std::endl;
		return a;
	}

	static int StaticFun(int a)
	{
		std::cout << "Static Function!!!!!" << std::endl;
		return 999;
	}
};

struct ADDShit
{
	int operator()(int ShitA, int ShitB)
	{
		std::cout << "Begin Add Shit" << std::endl;
		return ShitA + ShitB;
	}
};

struct PrintShit
{
	int operator()(int Shit)
	{
		std::cout << "SSSSSSSShit!!!!!!" << std::endl;
		return Shit;
	}
};

void TestDelegate()
{
	TDelegate<int(int)> A;
	TestClass C;

	std::cout << "=========================Test Static=========================" << std::endl;
	A.BindStatic(Print);
	check(A(100) == 100);
	A.BindStatic(PrintFour, 666.6f, "PrintFour", 666666.6666f);
	check(A(666) == 666);
	A.BindStatic(&TestClass::StaticFun);
	check(A(666) == 999);

	std::cout << "=========================Test Member=========================" << std::endl;
	A.BindMember(&C, &TestClass::PrintOne);
	check(A(100) == 100);
	A.BindMember(&C, &TestClass::PrintFour, 666.6f, "PrintFour", 666666.6666);
	check(A(666) == 666);
	A.BindMember(&C, &TestClass::PrintOneC);
	check(A(100) == 100);
	A.BindMember(&C, &TestClass::PrintFourC, 666.6f, "PrintFour", 666666.6666);
	check(A(666) == 666);

	std::cout << "=========================Test Functor========================" << std::endl;
	A.BindFunctor(ADDShit(), 100);
	check(A(66) == 166);
	A.BindFunctor(PrintShit());
	check(A(111) == 111);

	std::cout << "=========================Test Lambda=========================" << std::endl;
	A.BindLambda([](int a) { std::cout << "I'm a lambda!!!!!" << std::endl; return a; });
	check(A(111) == 111);
	A.BindLambda([](int a, const char* b) { std::cout << b << std::endl; return 0; }, "老子是参数!!!!!");
	char(A(999) == 0);
}
