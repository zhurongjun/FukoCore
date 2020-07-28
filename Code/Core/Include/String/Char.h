#pragma once
#include <CoreType.h>
#include <CoreConfig.h>
#include <ctype.h>
#include <wctype.h>

namespace  Fuko
{
	template <typename T>
	struct TChar
	{
		using CharType = T;
		static constexpr bool IsAnisChar = sizeof(CharType) == sizeof(ANSICHAR);

		static inline CharType ToUpper(CharType Char)
		{
			if constexpr (IsAnisChar)
				return ::toupper(Char);
			else
				return ::towupper(Char);
		}

		static inline CharType ToLower(CharType Char)
		{
			if constexpr (IsAnisChar)
				return ::tolower(Char);
			else
				return ::towlower(Char);
		}

		static inline bool IsUpper(CharType Char)
		{
			if constexpr (IsAnisChar)
				return ::isupper((unsigned char)Char) != 0;
			else
				return ::iswupper(Char) != 0;
		}

		static inline bool IsLower(CharType Char)
		{
			if constexpr (IsAnisChar)
				return ::islower((unsigned char)Char) != 0;
			else
				return ::iswlower(Char) != 0;
		}

		static inline bool IsAlpha(CharType Char)
		{
			if constexpr (IsAnisChar)
				return ::isalpha((unsigned char)Char) != 0;
			else
				return ::iswalpha(Char) != 0;

		}

		static inline bool IsGraph(CharType Char)
		{
			if constexpr (IsAnisChar)
				return ::isgraph((unsigned char)Char) != 0;
			else
				return ::iswgraph(Char) != 0;

		}

		static inline bool IsPrint(CharType Char)
		{
			if constexpr (IsAnisChar)
				return ::isprint((unsigned char)Char) != 0;
			else
				return ::iswprint(Char) != 0;
		}

		static inline bool IsPunct(CharType Char)
		{
			if constexpr (IsAnisChar)
				return ::ispunct((unsigned char)Char) != 0;
			else
				return ::iswpunct(Char) != 0;
		}

		static inline bool IsAlnum(CharType Char)
		{
			if constexpr (IsAnisChar)
				return ::isalnum((unsigned char)Char) != 0;
			else
				return ::iswalnum(Char) != 0;

		}

		static inline bool IsDigit(CharType Char)
		{
			if constexpr (IsAnisChar)
				return ::isdigit((unsigned char)Char) != 0;
			else
				return ::iswdigit(Char) != 0;
		}

		static inline bool IsOctDigit(CharType Char)
		{
			return Char >= '0' && Char <= '7';
		}

		static inline bool IsHexDigit(CharType Char)
		{
			if constexpr (IsAnisChar)
				return ::isxdigit((unsigned char)Char) != 0;
			else
				return ::iswxdigit(Char) != 0;
		}

		static inline int32 ConvertCharDigitToInt(CharType Char)
		{
			return static_cast<int32>(Char) - static_cast<int32>('0');
		}

		static inline bool IsWhitespace(CharType Char)
		{
			if constexpr (IsAnisChar)
				return ::isspace((unsigned char)Char) != 0;
			else
				return ::iswspace(Char) != 0;
		}

		static inline bool IsIdentifier(CharType Char)
		{
			return IsAlnum(Char) || IsUnderscore(Char);
		}

		static inline bool IsUnderscore(CharType Char) { return Char == '_'; }

		static inline bool IsLinebreak(CharType Char)
		{
			if constexpr (IsAnisChar)
			{
				return Char == '\x000A'
					|| Char == '\x000B'
					|| Char == '\x000C'
					|| Char == '\x000D'
					|| Char == '\x0085';
			}
			else
			{
				return Char == L'\x000A'
					|| Char == L'\x000B'
					|| Char == L'\x000C'
					|| Char == L'\x000D'
					|| Char == L'\x0085'
					|| Char == L'\x2028'
					|| Char == L'\x2029';
			}
		}
	};
}