#pragma once

// current global 
// PmrAllocator
// PoolAllocator
// Log
// Name 


#ifdef EXPORT_CORE
#define CORE_API __declspec(dllexport)
#else
#define CORE_API __declspec(dllimport)
#endif

#define FORCEINLINE __forceinline
#define FORCENOINLINE __declspec(noinline)
#define RESTRICT __restrict	

inline constexpr int INDEX_NONE = -1;

inline constexpr int NumBitsPerDWORD = 32;
inline constexpr int NumBitsPerDWORDLogTwo = 5;
inline constexpr int PerDWORDMask = NumBitsPerDWORD - 1;

inline constexpr unsigned int EmptyMask = 0u;
inline constexpr unsigned int FullMask = ~EmptyMask;
