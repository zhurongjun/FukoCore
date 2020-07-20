#pragma once
#include <CoreMinimal/Delegate.h>
#include <functional>

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

	int AddFourC(int a, float b, const char* c, double d) const
	{
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

struct BigFunctor
{
	double A, B, C, D, E, F, G, H;
	int operator()(int a, float b, const char* c, double d) const
	{
		return a;
	}

};

void TestDelegate()
{
	TDelegate<int(int)> A;
	TestClass C;

	std::cout << "=========================Test Static=========================" << std::endl;
	A.Bind(Print);
	check(A(100) == 100);
	A.Bind(PrintFour, 666.6f, "PrintFour", 666666.6666f);
	check(A(666) == 666);
	A.Bind(&TestClass::StaticFun);
	check(A(666) == 999);

	std::cout << "=========================Test Member=========================" << std::endl;
	A.Bind(&TestClass::PrintOne, &C);
	check(A(100) == 100);
	A.Bind(&TestClass::PrintFour, &C, 666.6f, "PrintFour", 666666.6666);
	check(A(666) == 666);
	A.Bind(&TestClass::PrintOneC, &C);
	check(A(100) == 100);
	A.Bind(&TestClass::PrintFourC, &C, 666.6f, "PrintFour", 666666.6666);
	check(A(666) == 666);

	std::cout << "=========================Test Functor========================" << std::endl;
	A.Bind(ADDShit(), 100);
	check(A(66) == 166);
	A.Bind(PrintShit());
	check(A(111) == 111);

	std::cout << "=========================Test Lambda=========================" << std::endl;
	A.Bind([](int a) { std::cout << "I'm a lambda!!!!!" << std::endl; return a; });
	check(A(111) == 111);
	A.Bind([](int a, const char* b) { std::cout << b << std::endl; return 0; }, "老子是参数!!!!!");
	char(A(999) == 0);

	std::cout << "=======================Test Performance======================" << std::endl;
	TArray<std::function<int(int)>>		StdArray(1000'0000);
	TArray<TDelegate<int(int)>>			FukoArray(1000'0000);

	auto Begin = std::chrono::system_clock::now();
	for (int i = 0; i < 1000'0000; ++i)
	{
		StdArray.Emplace_GetRef() = std::bind(BigFunctor(), std::placeholders::_1, 666.6f, "PrintFour", 666666.6666);
	}
	auto End = std::chrono::system_clock::now();
	std::cout << "Std construct time : " << std::chrono::duration<double, std::milli>(End - Begin).count() << " ms" << std::endl;

	Begin = std::chrono::system_clock::now();
	for (int i = 0; i < 1000'0000; ++i)
	{
		FukoArray.Emplace_GetRef().Bind(BigFunctor(), 666.6f, "PrintFour", 666666.6666);
	}
	End = std::chrono::system_clock::now();
	std::cout << "Fuko construct time : " << std::chrono::duration<double, std::milli>(End - Begin).count() << " ms" << std::endl;

	Begin = std::chrono::system_clock::now();
	for (int i = 0; i < 1000'0000; ++i)
	{
		StdArray[i](10);
	}
	End = std::chrono::system_clock::now();
	std::cout << "Std call time : " << std::chrono::duration<double, std::milli>(End - Begin).count() << " ms" << std::endl;

	Begin = std::chrono::system_clock::now();
	for (int i = 0; i < 1000'0000; ++i)
	{
		FukoArray[i](10);
	}
	End = std::chrono::system_clock::now();
	std::cout << "Fuko call time : " << std::chrono::duration<double, std::milli>(End - Begin).count() << " ms" << std::endl;
}
