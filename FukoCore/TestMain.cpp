#include <iostream>
#include "Templates/TypeTraits.h"
#include "Include/Templates/Align.h"
#include "Include/Containers/Allocators.h"
#include "Include/Math/MathUtility.h"
#include <windows.h>
#include "Include/Containers/ContainerFwd.h"
#include "Include/Containers/Array.h"
#include "Templates/UtilityTemp.h"
#include "Algo/BinarySearch.h"
#include "String/CString.h"
#include "Algo/Sort.h"
#include "Templates/Functor.h"
#include <string>
#include <vector>
#include "Algo/ForEach.h"
#include <atomic>
#include "Templates/Atomic.h"
#include "Templates/Tuple.h"

template<class T,class = std::void_t<>>
struct HasFFFFun
{
	static constexpr bool Value = false;
};

template<class T>
struct HasFFFFun<T,std::void_t<decltype(std::declval<T>().FFFFun())>>
{
	static constexpr bool Value = true;
};

struct AA
{
private:
	int a;
	int b;
	int c;
	int d;
};
int main()
{
	Fuko::TTuple<int, int, char, float> a(1, 2, 'a', 4.f);
	Fuko::TTuple<int, int, char, float> b = a;
	
	auto&& [n1, n2, n3, n4] = b;

	n4 = 100;

	std::cout << "---------------now begin each two tuple---------------" << std::endl;
	Fuko::VisitTupleElements(
		[](auto a, auto b) {std::cout << "a: " << a << "\tb: " << b << std::endl; },
		a,
		b
	);
	std::cout << "---------------now begin each one tuple---------------" << std::endl;
	b.Each([](auto n) { std::cout << "b: " << n << std::endl; });

	std::cout << "---------------now begin transform tuple---------------" << std::endl;
	auto c = Fuko::TransformTuple(std::move(b), [](auto n) -> decltype(auto)
	{
		if constexpr (TAreTypesEqual_v<int, decltype(n)>)
		{
			return (char)('0' + n);
		}
		else
		{
			return "Unkown Type";
		}
	});
	c.Each([](auto n) { std::cout << "c: " << n << std::endl; });

	std::cout << "---------------now begin get hash---------------" << std::endl;
	uint32 hash = Fuko::GetTypeHash(c);
	std::cout << "Hash: " << hash << std::endl;

// 	std::cout << Fuko::TCString<WIDECHAR>::IsNumeric(L"100") << std::endl;
// 
// 	Fuko::TCString<ANSICHAR>::Strfind("", "");

// 	class AAAAA
// 	{
// 	public:
// 		void FFFFun() {}
// 	};
// 
// 	class BBBBB
// 	{
// 
// 	};
// 	std::cout << HasFFFFun<AAAAA>::Value << std::endl;
// 	std::cout << HasFFFFun<BBBBB>::Value << std::endl;

// 	Arr += {5, 4, 3, 2, 1};
// 	Arr.Append(Arr);
// 	for (int i : Arr)
// 		std::cout << i << ',';
// 	std::cout << std::endl;
// 
// 	TArray<int> Five = Arr.FilterByPredicate([](int n) -> bool {return n == 5; });
// 	for (int i : Five)
// 		std::cout << i << ',';
// 	std::cout << std::endl;
// 
// 	Arr.RemoveSwap(1);
// 	for (int i : Arr)
// 		std::cout << i << ',';
// 	std::cout << std::endl;
// 
// 	Arr.RemoveAllSwap([](int n)->bool { return n <= 3; });
// 
// 	for (int i : Arr)
// 		std::cout << i << ',';
// 	std::cout << std::endl;
// 	std::cout << "Num: " << Arr.Num() << std::endl;

	system("pause");
	return 0;
}