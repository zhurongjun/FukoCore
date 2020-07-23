#pragma once
#include <String/Name.h>

using Fuko::Name;
void TestName()
{
	Name a(TSTR("Test Name"));
	Name b(TSTR("Test Name"));

	Name c;
	Name Last;
	TCHAR  chs[10] = {};
	for (int i = 0; i < 9; ++i) chs[i] = 1;
	for (int i = 0; i < 100'0000; ++i)
	{
		for (int j = 0, k = 10; j <= 7; j++)
		{
			if (j == 0)
				chs[j] = i % 10 + 1;
			else
			{
				chs[j] = ((i / k) % 10) + 1;
				k *= 10;
			}
		}
		c = Name(chs);
		always_check(c != Last);
		Last = c;
	}
	c == a;


	auto Begin = std::chrono::system_clock::now();
	for (int i = 0; i < 100'0000; ++i)
	{
		a == Name(TSTR("Test Name"));
	}
	auto End = std::chrono::system_clock::now();
	std::cout << "Construct time : " << std::chrono::duration<double, std::milli>(End - Begin).count() << " ms" << std::endl;

	Begin = std::chrono::system_clock::now();
	for (int i = 0; i < 100'0000; ++i)
	{
	}
	End = std::chrono::system_clock::now();
	std::cout << "MAKE_NAME time : " << std::chrono::duration<double, std::milli>(End - Begin).count() << " ms" << std::endl;
	
	check(a == b);
}
