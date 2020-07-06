#pragma once

#ifdef _DEBUG
	#define FUKO_DEBUG 1
#else
	#define FUKO_DEBUG 0
#endif // DEBUG

#ifdef CORE_EXPORT
#define CORE_API __declspec(dllexport)
#else
#define CORE_API __declspec(dllimport)
#endif // CORE_EXPORT

#define FORCEINLINE __forceinline
#define FORCENOINLINE __declspec(noinline)
#define RESTRICT __restrict	

inline constexpr int INDEX_NONE = -1;

