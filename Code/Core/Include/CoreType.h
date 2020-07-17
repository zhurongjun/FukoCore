#pragma once
#include <stdint.h>

typedef uint8_t		uint8;
typedef uint16_t	uint16;
typedef uint32_t	uint32;
typedef uint64_t	uint64;

typedef int8_t		int8;
typedef int16_t		int16;
typedef int32_t		int32;
typedef int64_t		int64;

typedef char		ANSICHAR;
typedef wchar_t		WIDECHAR;
typedef uint8		CHAR8;
typedef uint16		CHAR16;
typedef uint32		CHAR32;

typedef WIDECHAR	TCHAR;
#define TSTR(Str) L##Str
#define _TCHAR_DEFINED

constexpr size_t operator""_Kb(size_t InSize) { return InSize * 1024; }
constexpr size_t operator""_Mb(size_t InSize) { return InSize * 1024 * 1024; }
constexpr size_t operator""_Gb(size_t InSize) { return InSize * 1024 * 1024 * 1024; }