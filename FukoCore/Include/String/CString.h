#pragma once
#include "CoreConfig.h"
#include "CoreType.h"
#include "CoreMinimal/Assert.h"
#include "Char.h"
#include "Templates/UtilityTemp.h"
#include <string.h>

#define SWITCHCH(InCh) Switch(InCh,L##InCh)

namespace Fuko
{
	template<typename T>
	struct TCString
	{
		using CharType = T;
		static constexpr bool IsAnisChar = sizeof(CharType) == sizeof(ANSICHAR);

		static constexpr CharType Switch(const ANSICHAR ACh, const WIDECHAR WCh)
		{
			if constexpr (IsAnisChar)
				return ACh;
			else
				return WCh;
		}

		static FORCEINLINE bool IsPureAnsi(const T* Str)
		{
			if constexpr (IsAnisChar)
				return true;
			else
			{
				for (; *Str; Str++)
				{
					if (*Str > 0x7f)
					{
						return false;
					}
				}
				return true;
			}
		}
		
		static FORCEINLINE bool IsNumeric(const CharType* Str)
		{
			if (*Str == '-' || *Str == '+')
			{
				Str++;
			}

			bool bHasDot = false;
			while (*Str != '\0')
			{
				if (*Str == '.')
				{
					if (bHasDot)
					{
						return false;
					}
					bHasDot = true;
				}
				else if (!TChar<CharType>::IsDigit(*Str))
				{
					return false;
				}

				++Str;
			}

			return true;
		}

		static FORCEINLINE CharType* Strcpy(CharType* Dest, size_t DestCount, const CharType* Src)
		{
			if constexpr (IsAnisChar)
				return strcpy_s(Dest, DestCount, Src);
			else
				return wcscpy_s(Dest, DestCount, Src);
		}
		
		static FORCEINLINE CharType* Strncpy(CharType* Dest, const CharType* Src, int32 MaxLen)
		{
			if constexpr (IsAnisChar)
				return strncpy_s(Dest, Src, MaxLen);
			else
				return wcsncpy_s(Dest, Src, MaxLen);
		}
		
		static FORCEINLINE CharType* Strcat(CharType* Dest, size_t DestCount, const CharType* Src)
		{
			if constexpr (IsAnisChar)
				return strcat_s(Dest, DestCount, Src);
			else
				return wcscat_s(DestCount, DestCount, Src);
		}
		
		template<size_t DestCount>
		static FORCEINLINE CharType* Strcat(CharType(&Dest)[DestCount], const CharType* Src)
		{
			return Strcat(Dest, DestCount, Src);
		}
		
		static FORCEINLINE CharType* Strncat(CharType* Dest, const CharType* Src, int32 MaxLen)
		{
			int32 Len = Strlen(Dest);
			CharType* NewDest = Dest + Len;
			if ((MaxLen -= Len) > 0)
			{
				Strncpy(NewDest, Src, MaxLen);
			}
			return Dest;
		}
		
		static FORCEINLINE CharType* Strupr(CharType* Dest, size_t DestCount)
		{
			if constexpr (IsAnisChar)
				return _strupr_s(Dest, DestCount);
			else
				return wcsupr_s(Dest, DestCount);
		}
		
		template<size_t DestCount>
		static FORCEINLINE CharType* Strupr(CharType(&Dest)[DestCount])
		{
			return Strupr(Dest, DestCount);
		}

		static FORCEINLINE int32 Strcmp(const CharType* String1, const CharType* String2)
		{
			if constexpr (IsAnisChar)
				return strcmp(String1, String2);
			else
				return wcscmp(String1, String2);
		}

		static FORCEINLINE int32 Strncmp(const CharType* String1, const CharType* String2, size_t Count)
		{
			if constexpr (IsAnisChar)
				return strncmp(String1, String2, Count);
			else
				return wcsncmp(String1, String2, Count);
		}
		
		static FORCEINLINE int32 Stricmp(const CharType* String1, const CharType* String2)
		{
			if constexpr (IsAnisChar)
				stricmp(String1, String2);
			else
				wcsicmp(String1, String2);
		}
		
		static FORCEINLINE int32 Strnicmp(const CharType* String1, const CharType* String2, size_t Count)
		{
			if constexpr (IsAnisChar)
				strnicmp(String1, String2, Count);
			else
				wcsnicmp(String1, String2, Count);
		}

		static FORCEINLINE const CharType* Spc(int32 NumSpaces)
		{
			check(NumSpaces <= 255);
			if constexpr (IsAnisChar)
			{
				static const ANSICHAR SpcArray[256] =
					"                                                                "
					"                                                                "
					"                                                                "
					"                                                               ";
				return SpcArray + 255 - NumSpaces;
			}
			else
			{
				static const WIDECHAR SpcArray[256] =
					L"                                                                "
					L"                                                                "
					L"                                                                "
					L"                                                               ";
				return SpcArray + 255 - NumSpaces;
			}
		}
		
		static FORCEINLINE const CharType* Tab(int32 NumTabs)
		{
			check(NumTabs <= 255);
			if constexpr (IsAnisChar)
			{
				static const WIDECHAR TabArray[256] =
					"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t"
					"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t"
					"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t"
					"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";
				return TabArray + 255 - NumTabs;
			}
			else
			{
				static const WIDECHAR TabArray[256] =
					L"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t"
					L"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t"
					L"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t"
					L"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";
				return TabArray + 255 - NumTabs;
			}
		}

		static FORCEINLINE const CharType* Strfind(const CharType* Str, const CharType* Find, bool bSkipQuotedChars = false)
		{
			if (Find == nullptr || Str == nullptr) nullptr;

			bool Alnum = 0;
			CharType f = *Find;
			int32 Length = Strlen(Find++) - 1;
			CharType c = *Str++;
			if (bSkipQuotedChars)
			{
				bool bInQuotedStr = false;
				while (c)
				{
					if (!bInQuotedStr && !Alnum && c == f && !Strncmp(Str, Find, Length))
					{
						return Str - 1;
					}
					Alnum = (c >= SWITCHCH('A') && c <= SWITCHCH('Z')) ||
						(c >= SWITCHCH('a') && c <= SWITCHCH('z')) ||
						(c >= SWITCHCH('0') && c <= SWITCHCH('9'));
					if (c == SWITCHCH('"'))
					{
						bInQuotedStr = !bInQuotedStr;
					}
					c = *Str++;
				}
			}
			else
			{
				while (c)
				{
					if (!Alnum && c == f && !Strncmp(Str, Find, Length))
					{
						return Str - 1;
					}
					Alnum = (c >= SWITCHCH('A') && c <= SWITCHCH('Z')) ||
						(c >= SWITCHCH('a') && c <= SWITCHCH('z')) ||
						(c >= SWITCHCH('0') && c <= SWITCHCH('9'));
					c = *Str++;
				}
			}
			return nullptr;
		}

		static FORCEINLINE const CharType* Strifind(const CharType* Str, const CharType* Find, bool bSkipQuotedChars = false)
		{
			if (Find == nullptr || Str == nullptr) return nullptr;

			bool Alnum = 0;
			CharType f = (*Find < SWITCHCH('a') || *Find > SWITCHCH('z')) ? (*Find) : (*Find + SWITCHCH('A') - SWITCHCH('a'));
			int32 Length = Strlen(Find++) - 1;
			CharType c = *Str++;

			if (bSkipQuotedChars)
			{
				bool bInQuotedStr = false;
				while (c)
				{
					if (c >= SWITCHCH('a') && c <= SWITCHCH('z'))
					{
						c += SWITCHCH('A') - SWITCHCH('a');
					}
					if (!bInQuotedStr && !Alnum && c == f && !Strnicmp(Str, Find, Length))
					{
						return Str - 1;
					}
					Alnum = (c >= SWITCHCH('A') && c <= SWITCHCH('Z')) ||
						(c >= SWITCHCH('0') && c <= SWITCHCH('9'));
					if (c == SWITCHCH('"'))
					{
						bInQuotedStr = !bInQuotedStr;
					}
					c = *Str++;
				}
			}
			else
			{
				while (c)
				{
					if (c >= SWITCHCH('a') && c <= SWITCHCH('z'))
					{
						c += SWITCHCH('A') - SWITCHCH('a');
					}
					if (!Alnum && c == f && !Strnicmp(Str, Find, Length))
					{
						return Str - 1;
					}
					Alnum = (c >= SWITCHCH('A') && c <= SWITCHCH('Z')) ||
						(c >= SWITCHCH('0') && c <= SWITCHCH('9'));
					c = *Str++;
				}
			}
			return nullptr;
		}

		static FORCEINLINE const CharType* StrfindDelim(const CharType* Str, const CharType* Find, const CharType* Delim = LITERAL(CharType, " \t,"))
		{
			if (Find == nullptr || Str == nullptr) return nullptr;

			int32 Length = Strlen(Find);
			const T* Found = Stristr(Str, Find);
			if (Found)
			{
				// check if this occurrence is delimited correctly
				if ((Found == Str || Strchr(Delim, Found[-1]) != nullptr) &&								// either first char, or following a delim
					(Found[Length] == SWITCHCH('\0') || Strchr(Delim, Found[Length]) != nullptr))	// either last or with a delim following
				{
					return Found;
				}

				// start searching again after the first matched character
				for (;;)
				{
					Str = Found + 1;
					Found = Stristr(Str, Find);

					if (Found == nullptr) return nullptr;

					// check if the next occurrence is delimited correctly
					if ((Strchr(Delim, Found[-1]) != NULL) &&												// match is following a delim
						(Found[Length] == SWITCHCH('\0') || Strchr(Delim, Found[Length]) != nullptr))	// either last or with a delim following
					{
						return Found;
					}
				}
			}

			return nullptr;
		}

		static FORCEINLINE const CharType* Stristr(const CharType* Str, const CharType* Find)
		{
			// both strings must be valid
			if (Find == nullptr || Str == nullptr) return;

			// get upper-case first letter of the find string (to reduce the number of full strnicmps)
			CharType FindInitial = TChar<CharType>::ToUpper(*Find);
			// get length of find string, and increment past first letter
			int32   Length = Strlen(Find++) - 1;
			// get the first letter of the search string, and increment past it
			CharType StrChar = *Str++;
			// while we aren't at end of string...
			while (StrChar)
			{
				// make sure it's upper-case
				StrChar = TChar<CharType>::ToUpper(StrChar);
				// if it matches the first letter of the find string, do a case-insensitive string compare for the length of the find string
				if (StrChar == FindInitial && !Strnicmp(Str, Find, Length))
				{
					// if we found the string, then return a pointer to the beginning of it in the search string
					return Str - 1;
				}
				// go to next letter
				StrChar = *Str++;
			}

			return nullptr;
		}
		static FORCEINLINE CharType* Stristr(CharType* Str, const CharType* Find) { return (CharType*)Stristr((const CharType*)Str, Find); }

		static FORCEINLINE int32 Strlen(const CharType* String)
		{
			if constexpr (IsAnisChar)
				return strlen(String);
			else
				return wcslen(String);
		}

		static FORCEINLINE const CharType* Strstr(const CharType* String, const CharType* Find)
		{
			if constexpr (IsAnisChar)
				return strstr(String, Find);
			else
				return wcsstr(String, Find);
		}
		static FORCEINLINE CharType* Strstr(CharType* String, const CharType* Find) { return (CharType*)Strstr(String, Find); }

		static FORCEINLINE const CharType* Strchr(const CharType* String, CharType c)
		{
			if constexpr (IsAnisChar)
				return strchr(String, c);
			else
				return wcschr(String, c);
		}
		static FORCEINLINE CharType* Strchr(CharType* String, CharType c) { return (CharType*)Strchr(String, c); }

		static FORCEINLINE const CharType* Strrchr(const CharType* String, CharType c)
		{
			if constexpr (IsAnisChar)
				return strrchr(String, c);
			else
				return wcsrchr(String, c);
		}
		static FORCEINLINE CharType* Strrchr(CharType* String, CharType c) { return (CharType*)Strrchr(String, c); }

		static FORCEINLINE const CharType* Strrstr(const CharType* String, const CharType* Find) { return Strrstr((CharType*)String, Find); }
		static FORCEINLINE CharType* Strrstr(CharType* String, const CharType* Find)
		{
			if (*Find == (CharType)0)
			{
				return String + Strlen(String);
			}

			CharType* Result = nullptr;
			for (;;)
			{
				CharType* Found = Strstr(String, Find);
				if (!Found)
				{
					return Result;
				}

				Result = Found;
				String = Found + 1;
			}
		}

		static FORCEINLINE int32 Strspn(const CharType* String, const CharType* Mask)
		{
			if constexpr (IsAnisChar)
				return strspn(String, Mask);
			else
				return wcsspn(String, Mask);
		}

		static FORCEINLINE int32 Strcspn(const CharType* String, const CharType* Mask)
		{
			if constexpr (IsAnisChar)
				return strcspn(String, Mask);
			else
				return wcscspn(String, Mask);
		}

		static FORCEINLINE int32 Atoi(const CharType* String)
		{
			if constexpr (IsAnisChar)
				return atoi(String);
			else
				return _wtoi(String);
		}

		static FORCEINLINE int64 Atoi64(const CharType* String)
		{
			if constexpr (IsAnisChar)
				return _atoi64(String);
			else
				return _wtoi64(String);
		}

		static FORCEINLINE float Atof(const CharType* String)
		{
			if constexpr (IsAnisChar)
				return (float)atof(String);
			else
				return _wtof(String);
		}

		static FORCEINLINE double Atod(const CharType* String)
		{
			if constexpr (IsAnisChar)
				return atof(String);
			else
				return wcstod(String);
		}

		static FORCEINLINE bool ToBool(const CharType* String)
		{
			if (
				Stricmp(String, SWITCHCH("True")) == 0 ||
				Stricmp(String, SWITCHCH("Yes")) == 0 ||
				Stricmp(String, SWITCHCH("On")) == 0
				)
			{
				return true;
			}
			else if (
				Stricmp(String, SWITCHCH("False")) == 0 ||
				Stricmp(String, SWITCHCH("No")) == 0 ||
				Stricmp(String, SWITCHCH("Off")) == 0
				)
			{
				return false;
			}
			else
			{
				return Atoi(String) ? true : false;
			}
		}

		static FORCEINLINE int32 Strtoi(const CharType* Start, CharType** End, int32 Base)
		{
			if constexpr (IsAnisChar)
				return strtol(Start, End, Base);
			else
				return wcstol(Start, End, Base);
		}

		static FORCEINLINE int64 Strtoi64(const CharType* Start, CharType** End, int32 Base)
		{
			if constexpr (IsAnisChar)
				return _strtoi64(Start, End, Base);
			else
				return _wcstoi64(Start, End, Base);
		}

		static FORCEINLINE uint64 Strtoui64(const CharType* Start, CharType** End, int32 Base)
		{
			if constexpr (IsAnisChar)
				return _strtoui64(Start, End, Base);
			else
				return _wcstoui64(Start, End, Base);
		}

		static FORCEINLINE CharType* Strtok(CharType* TokenString, const CharType* Delim, CharType** Context)
		{
			if constexpr (IsAnisChar)
				return strtok_s(TokenString, Delim, Context);
			else
				return wcstok_s(TokenString, Delim, Context);
		}

		template <typename FmtType, typename... Types>
		static FORCEINLINE int32 Sprintf(CharType* Dest, const FmtType& Fmt, Types... Args)
		{
			static_assert(TIsArrayOrRefOfType_v<FmtType, CharType>, "Formatting string must be a literal string of the same character type as template.");
			static_assert((TIsValidVariadicFunctionArg_v<Types> && ... ), "Invalid argument(s) passed to TCString::Sprintf");

			int32 Result = -1;

			{ 
				va_list ap; 
				va_start(ap, Fmt); 

				if constexpr (IsAnisChar)
					Result = _vsnprintf_s(Dest, 1024, 1024 - 1, Fmt, ap);
				else
					Result = _vsnwprintf_s(Dest, 1024, 1024 - 1, Fmt, ap);

				va_end(ap);
				if (Result >= 1024) { Result = -1; } 
			}

			return Result;
		}

		template <typename FmtType, typename... Types>
		static FORCEINLINE int32 Snprintf(CharType* Dest, int32 DestSize, const FmtType& Fmt, Types... Args)
		{
			static_assert(TIsArrayOrRefOfType_v<FmtType, CharType>, "Formatting string must be a literal string of the same character type as template.");
			static_assert((TIsValidVariadicFunctionArg_v<Types> && ...), "Invalid argument(s) passed to TCString::Sprintf");
			
			int32 Result = -1;

			{
				va_list ap;
				va_start(ap, Fmt);

				if constexpr (IsAnisChar)
					Result = _vsnprintf_s(Dest, DestSize, DestSize - 1, Fmt, ap);
				else
					Result = _vsnwprintf_s(Dest, DestSize, DestSize - 1, Fmt, ap);

				va_end(ap);
				if (Result >= DestSize) { Result = -1; }
			}

			return Result;
		}
	};

	typedef TCString<ANSICHAR> AString;
	typedef TCString<WIDECHAR> WString;

}

#undef SWITCHCH