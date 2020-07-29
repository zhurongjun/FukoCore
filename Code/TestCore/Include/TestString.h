#pragma once
#include <String/String.h>

using Fuko::String;
void TestString()
{
	String Str(L"Oh Shit!!!");
	Str = L"afasdfasdfsdfafasdfasdfasdfasdfsdafasdfsdafsadfasfasdfasdfasdfasfasdfasdf";
	const TCHAR * ch = L"!!!!!!";
	Str.Append(ch, 3);
	Str.Add(L'G', 1000);
	Str.Remove(L'a');
	Str.Remove(L'f');
	Str.Remove(L's');
	Str.Remove(L'd');
	Str.Remove(L'G');
	std::wcout << *Str << std::endl;

	String A(L"Test Str");
	String B(L"Test Str");
	String C;
	always_check(C.Len() == 0);
	always_check(C.Num() == 0);
	always_check(C.Max() == 0);
	always_check(A.Num() == 9);
	always_check(A == B);
	always_check(A.Last() == L'r');
	always_check(A.Pop() == L'r');
	A.Add(L'r');
	always_check(A == B);
	A.Reset();
	always_check(A == C);
	always_check(A.IsEmpty());
	always_check(C.IsEmpty());

	A = L"Test Str";
	A.Append(-115641615);
	A.AppendFmt(L"  %s,%d", L"aaa");
	A.AppendFmt(L"  %s,%d", L"aaa");
	A.AppendFmt(L"  %s,%d", L"aaa");
	A.AppendFmt(L"  %s,%d", L"aaa");
	A.AppendFmt(L"  %s,%d", L"aaa");
	A.AppendFmt(L"  %s,%d", L"aaa");
	std::wcout << *A.Upper() << std::endl;
	std::wcout << *A.Lower() << std::endl;

	TMap<Fuko::TString<ANSICHAR>, Fuko::TString<ANSICHAR>> Maps;

	std::wcout << 100 << std::endl;
}