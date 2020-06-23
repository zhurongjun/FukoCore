#pragma once

#ifdef _DEBUG
	#define FUKO_DEBUG 1
#else
	#define FUKO_DEBUG 0
#endif // DEBUG

#define TSTR(InStr) L##InStr

#define FORCEINLINE __forceinline
#define CONSTEXPR constexpr

#define FORCENOINLINE __declspec(noinline)

#define ASSUME(InExpr) __assume(InExpr)

// 指定变量不存在别名，这样子编译器就可以放心的优化程序
#define RESTRICT __restrict	

inline constexpr int INDEX_NONE = -1;

enum ENoInit{NoInit};

// Temporal
#define CORE_API