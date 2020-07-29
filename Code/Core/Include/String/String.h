#pragma once
#include <CoreType.h>
#include <CoreConfig.h>
#include <Containers/Allocator.h>
#include "../Algo/Find.h"

// String shared ptr
namespace Fuko
{
	template<typename T, typename TAlloc>
	struct TStringSP
	{
		using SizeType = typename TAlloc::SizeType;

		T*			Str = nullptr;
		SizeType	Num = 0;
		SizeType	Max = 0;
		SizeType	RefCount = 0;

		// Shared ptr
		FORCEINLINE void Retain() { ++RefCount; }
		FORCEINLINE bool IsUnique() { return RefCount == 1; }
		FORCEINLINE bool Release() { --RefCount; return RefCount == 0; }
		
		// Memory operator
		FORCEINLINE void Free(TAlloc& Alloc) 
		{
			Num = 0;
			Max = Alloc.Free(Str); 
		}
		FORCEINLINE void Reserve(SizeType InMax, TAlloc& Alloc) 
		{
			Max = Alloc.Reserve(Str, InMax); 
		}
		FORCEINLINE void Grow(TAlloc& Alloc)
		{
			SizeType NewMax = Alloc.GetGrow(Num, Max);
			Reserve(NewMax, Alloc);
		}
		FORCEINLINE void Shrink(TAlloc& Alloc)
		{
			SizeType NewMax = Alloc.GetShrink(Num, Max);
			Reserve(NewMax, Alloc);
		}

		// From raw 
		FORCEINLINE void CopyFromRaw(const T* InStr, SizeType Len)
		{
			Num = Len + 1;
			Memcpy(Str, InStr, Len * sizeof(T));
			Str[Len] = 0;
		}
		FORCEINLINE void AppendRaw(const T* InStr, SizeType Len)
		{
			if (Len == 0) return;
			if (Num)
			{
				Memcpy(Str + Num - 1, InStr, Len * sizeof(T));
				Num += Len;
				Str[Num - 1] = 0;		
			}
			else
			{
				CopyFromRaw(InStr, Len);
			}
		}
		FORCEINLINE void AppendCh(T Ch, SizeType Count)
		{
			if (Count == 0) return;
			if (Num)
			{
				T* Begin = Str + Num - 1;
				Num += Count;
				T* End = Str + Num - 1;
				// copy data 
				*End = 0;
				for (; Begin != End; ++Begin) *Begin = Ch;
			}
			else
			{
				Num = Count + 1;
				T* Begin = Str;
				T* End = Str + Num - 1;
				*End = 0;
				for (; Begin != End; ++Begin) *Begin = Ch;
			}
		}

		// Sub inline
		FORCEINLINE void LeftInline(SizeType Index)
		{
			Str[Index] = 0;
			Num = Index + 1;
		}
		FORCEINLINE void RightInline(SizeType Index)
		{
			Num -= Index
			Memmove(Str, Str + Index, Num * sizeof(T));
		}
		FORCEINLINE void SubStrInline (SizeType Index, SizeType Count)
		{
			Num = Count + 1;
			T* Src = Str + Index;
			Memmove(Str, Src, Count * sizeof(T));
			Str[Num - 1] = 0;
		}
	};
}

// String
namespace Fuko
{
	template<typename T, typename TAlloc = TBlockAlloc<PmrAlloc>>
	class TString
	{
		using StringSP = TStringSP<T, TAlloc>;

		TAlloc		m_Alloc;
		StringSP*	m_StrSP;

		using SizeType = typename TAlloc::SizeType;
		using CString = TCString<T>;
		//===============================Begin help function===============================
		void _Free()
		{
			if (m_StrSP && m_StrSP->Release())
			{
				m_StrSP->Free(m_Alloc);
				m_Alloc.Free(m_StrSP);
			}
		}
		void _Detach()
		{
			if (m_StrSP && !m_StrSP->IsUnique())
			{
				// detach old shared ptr
				m_StrSP->Release();

				// create new shared ptr
				StringSP* NewSP = nullptr;
				m_Alloc.Reserve(NewSP, 1);
				new (NewSP)StringSP();
				NewSP->Retain();

				// copy data 
				NewSP->Reserve(m_StrSP->Max, m_Alloc);
				NewSP->Num = m_StrSP->Num;
				Memcpy(NewSP->Str, m_StrSP->Str, m_StrSP->Num * sizeof(T));

				m_StrSP = NewSP;
			}
		}
		void _AllocIfNull()
		{
			if (!m_StrSP)
			{
				m_Alloc.Reserve(m_StrSP, 1);
				new(m_StrSP)StringSP();
				m_StrSP->Retain();
			}
		}

		void _ResizeTo(SizeType NewSize)
		{
			if (NewSize == 0)
			{
				_Free();
				return;
			}
			_Detach();
			_AllocIfNull();

			m_StrSP->Reserve(NewSize, m_Alloc);
		}
		void _ResizeGrow(SizeType NewMax)
		{
			_Detach();
			_AllocIfNull();

			SizeType LastNum = m_StrSP->Num;
			m_StrSP->Num = NewMax;
			m_StrSP->Grow(m_Alloc);
			m_StrSP->Num = LastNum;
		}
		void _ResizeShrink()
		{
			if (!m_StrSP) return;
			_Detach();

			m_StrSP->Shrink(m_Alloc);
		}
		//================================End help function================================
	public:
		// construct 
		FORCEINLINE TString(const TAlloc& Alloc = TAlloc())
			: m_Alloc(Alloc)
			, m_StrSP(nullptr)
		{}
		FORCEINLINE TString(const T* Str, const TAlloc& Alloc = TAlloc())
			: m_Alloc(Alloc)
			, m_StrSP(nullptr)
		{
			*this = Str;
		}
		FORCEINLINE TString(const T* Str, SizeType StrLen, const TAlloc& Alloc = TAlloc())
			: m_Alloc(Alloc)
			, m_StrSP(nullptr)
		{
			Append(Str, StrLen);
		}
		FORCEINLINE TString(const T* Begin, const T* End, const TAlloc& Alloc = TAlloc())
			: m_Alloc(Alloc)
			, m_StrSP(nullptr)
		{
			Append(Begin, End);
		}

		// copy construct
		FORCEINLINE TString(const TString& InStr, const TAlloc& Alloc = TAlloc())
			: m_Alloc(Alloc)
			, m_StrSP(InStr.m_StrSP)
		{
			if (m_StrSP) m_StrSP->Retain();
		}

		// move construct 
		FORCEINLINE TString(TString&& InStr)
			: m_Alloc(InStr.m_Alloc)
			, m_StrSP(InStr.m_StrSP)
		{
			InStr.m_StrSP = nullptr;
		}

		// copy assign 
		FORCEINLINE TString& operator=(const TString& InStr)
		{
			_Free();
			m_StrSP = InStr.m_StrSP;
			if (m_StrSP) m_StrSP->Retain();
			return *this;
		}

		// move assign 
		FORCEINLINE TString& operator=(TString&& InStr)
		{
			_Free();
			m_StrSP = InStr.m_StrSP;
			InStr.m_StrSP = nullptr;
			return *this;
		}

		// assign from raw string 
		TString& operator=(const T* InStr)
		{
			Reset();
			SizeType StrLen = CString::Strlen(InStr);
			SizeType ArrLen = StrLen + 1;
			if (Max() < ArrLen) _ResizeTo(ArrLen);
			m_StrSP->CopyFromRaw(InStr, StrLen);
			return *this;
		}

		// destruct
		FORCEINLINE ~TString() { _Free(); }

		// get info 
		FORCEINLINE SizeType Num() const { return m_StrSP ? m_StrSP->Num : 0; }
		FORCEINLINE SizeType Max() const { return m_StrSP ? m_StrSP->Max : 0; }
		FORCEINLINE SizeType Len() const { SizeType N = Num(); return N ? N - 1 : 0; }
		FORCEINLINE bool IsEmpty() const { return Len() == 0; }
		FORCEINLINE T* GetData() { return m_StrSP ? m_StrSP->Str : nullptr; }
		FORCEINLINE const T* GetData() const { return const_cast<TString*>(this)->GetData(); }
		FORCEINLINE T* operator*() { return GetData(); }
		FORCEINLINE const T* operator*() const { return GetData(); }

		// Reset & Empty & Shrink & Reserve
		FORCEINLINE void Shrink() { _ResizeShrink(); }
		FORCEINLINE void Reserve(SizeType Number)
		{
			_ResizeTo(Number);
		}
		FORCEINLINE void Reset(SizeType NewSize = 0)
		{
			if (NewSize < Max())
			{
				_Detach();
				if (m_StrSP)
				{
					m_StrSP->Num = 0;
					if (m_StrSP->Str) m_StrSP->Str[0] = 0;
				}
			}
			else
			{
				Empty(NewSize);
			}
		}
		FORCEINLINE void Empty(SizeType InSlack = 0)
		{
			check(InSlack >= 0);
			_ResizeTo(InSlack);
			if (m_StrSP)
			{
				m_StrSP->Num = 0;
				if (m_StrSP->Str) m_StrSP->Str[0] = 0;
			}
		}

		// Add & Push & Pop
		FORCEINLINE void Add(T Ch, SizeType Count = 1)
		{
			SizeType ArrLen = Len() + Count + 1;
			if (Max() < ArrLen) _ResizeTo(ArrLen);
			m_StrSP->AppendCh(Ch, Count);
		}
		FORCEINLINE void Push(T Ch) { Add(Ch); }
		FORCEINLINE T Pop()
		{
			T& LastCh = Last();
			T RetCh = LastCh;
			LastCh = 0;
			--m_StrSP->Num;
			return RetCh;
		}

		// removeAt
		FORCEINLINE void RemoveAt(SizeType Index, SizeType Count = 1)
		{
			auto Write = begin() + Index;
			auto Read = Write + Count;
			SizeType MoveSize = Num() - Index - Count;
			Memmove(Write, Read, MoveSize);
		}

		// remove
		FORCEINLINE SizeType Remove(T Ch)
		{
			if (IsEmpty()) return 0;
			SizeType Count = 0;
			auto Write = begin();
			auto Read = Write;
			auto End = end();
			for (; Read != End;)
			{
				if (*Read == Ch)
				{
					++Read;
					++Count;
				}
				else
				{
					*Write = *Read;
					++Read;
					++Write;
				}
			}
			*Write = 0;
			return Count;
		}

		// find 
		FORCEINLINE T* Find(T Ch)
		{
			if (IsEmpty()) return nullptr;
			return Algo::Find(m_StrSP->Str, m_StrSP->Num, Ch);
		}
		FORCEINLINE T* FindLast(T Ch)
		{
			if (IsEmpty()) return nullptr;
			return Algo::FindLast(m_StrSP->Str, m_StrSP->Num, Ch);
		}
		template<typename TPred>
		FORCEINLINE T* FindBy(TPred&& Pred)
		{
			if (IsEmpty()) return nullptr;
			return Algo::FindBy(m_StrSP->Str, m_StrSP->Num, std::forward<TPred>(Pred));
		}
		template<typename TPred>
		FORCEINLINE T* FindLastBy(TPred&& Pred)
		{
			if (IsEmpty()) return nullptr;
			return Algo::FindLastBy(m_StrSP->Str, m_StrSP->Num, std::forward<TPred>(Pred));
		}
		FORCEINLINE T* Find(const T* Str) { return CString::Strstr(m_StrSP->Str, Str); }
		FORCEINLINE T* FindLast(const T* Str) { return CString::Strrstr(m_StrSP->Str, Str); }

		// index of 
		FORCEINLINE SizeType IndexOf(T Ch) const { auto Ptr = Find(Ch); return Ptr ? Ptr - m_StrSP->Str : INDEX_NONE; }
		FORCEINLINE SizeType IndexOfLast(T Ch) const { auto Ptr = FindLast(Ch); return Ptr ? Ptr - m_StrSP->Str : INDEX_NONE; }
		template<typename TPred>
		FORCEINLINE SizeType IndexOfBy(TPred&& Pred) const { auto Ptr = FindBy(std::forward<TPred>(Pred)); return Ptr ? Ptr - m_StrSP->Str : INDEX_NONE; }
		template<typename TPred>
		FORCEINLINE SizeType IndexOfLastBy(TPred&& Pred) const { auto Ptr = FindLastBy(std::forward<TPred>(Pred)); return Ptr ? Ptr - m_StrSP->Str : INDEX_NONE; }
		FORCEINLINE SizeType IndexOf(const T* Str) { auto Ptr = Find(Str); return Ptr ? Ptr - m_StrSP->Str : INDEX_NONE; }
		FORCEINLINE SizeType IndexOfLast(const T* Str) const { auto Ptr = FindLast(Str); return Ptr ? Ptr - m_StrSP->Str : INDEX_NONE; }

		// sub string 
		FORCEINLINE TString SubStr(SizeType Index, SizeType Len) const { return TString(begin() + Index, Len); }
		FORCEINLINE TString Left(SizeType Index) const { return TString(begin(), Index); }
		FORCEINLINE TString Right(SizeType Index) const { return TString(begin() + Index, end()); }
		void SubStrInline(SizeType Index, SizeType Len)
		{
			_Detach();
			m_StrSP->SubStrInline(Index, Len);
		}
		void LeftInline(SizeType Index)
		{
			_Detach();
			m_StrSP->LeftInline(Index);
		}
		void RightInline(SizeType Index)
		{
			_Detach();
			m_StrSP->RightInline(Index);
		}

		// split 
		FORCEINLINE bool Split(T Ch, TString& OutLeft, TString& OutRight) const
		{
			if (IsEmpty()) return false;
			SizeType Index = IndexOf(Ch);
			if (Index == INDEX_NONE) return false;
			OutLeft = Left(Index);
			OutRight = Right(Index);
			return true;
		}
		template<typename TArrAlloc>
		FORCEINLINE bool Split(T Ch, TArray<TString, TArrAlloc>& OutArr) const
		{
			if (IsEmpty()) return false;
			SizeType BeginIndex = 0;
			SizeType SplitCount = 0;
			T* It = begin();
			for (SizeType i = 0; i < m_StrSP->Num; ++i, ++It)
			{
				if (*It == Ch)
				{
					OutArr.Emplace(m_StrSP->Str + BeginIndex, i - BeginIndex);
					++SplitCount;
					BeginIndex = i + 1;
				}
			}

			// last str
			if (SplitCount && BeginIndex < m_StrSP->Num)
			{
				OutArr.Emplace(m_StrSP->Str + BeginIndex, m_StrSP->Num - BeginIndex - 1);
			}
			return SplitCount;
		}

		// upper & lower
		FORCEINLINE TString Upper() const { TString Ret = TString(*this); Ret.UpperInline(); return Ret; }
		FORCEINLINE TString Lower() const { TString Ret = TString(*this); Ret.LowerInline(); return Ret; }
		void UpperInline()
		{
			if (IsEmpty()) return;
			_Detach();
			CString::Strupr(m_StrSP->Str, m_StrSP->Num);
		}
		void LowerInline()
		{
			if (IsEmpty()) return;
			_Detach();
			CString::Strlwr(m_StrSP->Str, m_StrSP->Num);
		}

		// isxxx 
		bool IsPureAnsi() { return CString::IsPureAnsi(m_StrSP->Str); }
		bool IsNumeric() { return CString::IsNumeric(m_StrSP->Str); }

		// contain 
		FORCEINLINE bool Contain(T Ch) { return Find(Ch) != nullptr; }
		template<typename TPred>
		FORCEINLINE bool ContainBy(TPred&& Pred) { return FindBy(std::forward<TPred>(Pred)) != nullptr; }

		// compare 
		FORCEINLINE bool operator==(const TString& Rhs) const
		{
			// same shared ptr 
			if (m_StrSP == Rhs.m_StrSP) return true;
			// both empty 
			if (IsEmpty() && Rhs.IsEmpty()) return true;
			// one is empty, but other not 
			if (!m_StrSP || !Rhs.m_StrSP) return false;
			// num not match 
			if (m_StrSP->Num != Rhs.m_StrSP->Num) return false;
			// compare 
			return CString::Strcmp(m_StrSP->Str, Rhs.m_StrSP->Str) == 0;
		}
		FORCEINLINE bool operator!=(const TString& Rhs) const { return !(*this == Rhs); }
		FORCEINLINE bool operator==(const T* Rhs) const 
		{
			SizeType StrLen = CString::Strlen(Rhs);
			if (StrLen != Len()) return false;
			// Both empty 
			if (!m_StrSP) return true;
			return CString::Strcmp(m_StrSP->Str, Rhs) == 0;
		}
		FORCEINLINE bool operator!=(const T* Rhs) const { return !(*this == Rhs); }

		// access 
		FORCEINLINE T& operator[](SizeType N) { return m_StrSP->Str[N]; }
		FORCEINLINE const T& operator[](SizeType N) const { return m_StrSP->Str[N]; }
		FORCEINLINE T& Last(SizeType N = 0) { return m_StrSP->Str[m_StrSP->Num - N - 2]; }
		FORCEINLINE const T& Last(SizeType N = 0) const { return m_StrSP->Str[m_StrSP->Num - N - 2]; }

		// append 
		void Append(const T* Begin, const T* End)
		{
			SizeType StrLen = End - Begin;
			SizeType ArrLen = Len() + StrLen + 1;
			_Detach();
			if (Max() < ArrLen) _ResizeGrow(ArrLen);
			m_StrSP->AppendRaw(Begin, StrLen);
		}
		void Append(const T* Str, SizeType StrLen)
		{
			SizeType ArrLen = Len() + StrLen + 1;
			_Detach();
			if (Max() < ArrLen) _ResizeGrow(ArrLen);
			m_StrSP->AppendRaw(Str, StrLen);
		}

		// append other type 
		void Append(int32 InNum)
		{
			if constexpr (CString::IsAnisChar)
			{
				constexpr ANSICHAR* DigitToChar = "9876543210123456789";
				constexpr int32 ZeroDigitIndex = 9;
				const bool bIsNegative = InNum < 0;
				ANSICHAR TempBuf[16];
				int32 TempIndex = 16;

				do
				{
					TempBuf[--TempIndex] = DigitToChar[ZeroDigitIndex + (InNum % 10)];
				} while (InNum);
				if (bIsNegative)
				{
					TempBuf[--TempIndex] = '-';
				}

				const ANSICHAR* ChPtr = TempBuf + TempIndex;
				const int32 NumCh = 16 - TempIndex;
				Append(ChPtr, NumCh);
			}
			else
			{
				constexpr WIDECHAR* DigitToChar = L"9876543210123456789";
				constexpr int32 ZeroDigitIndex = 9;
				const bool bIsNegative = InNum < 0;
				WIDECHAR TempBuf[16];
				int32 TempIndex = 16;

				do
				{
					TempBuf[--TempIndex] = DigitToChar[ZeroDigitIndex + (InNum % 10)];
					InNum /= 10;
				} while (InNum);
				if (bIsNegative)
				{
					TempBuf[--TempIndex] = L'-';
				}

				const WIDECHAR* ChPtr = TempBuf + TempIndex;
				const int32 NumCh = 16 - TempIndex;
				Append(ChPtr, NumCh);
			}
		}
		void Append(bool InBool)
		{
			if (InBool) Append(CString::Switch("True", L"True"), 4);
			else Append(CString::Switch("False", L"False"), 5);
		}

		// append fmt
		template<typename...TArgs>
		void AppendFmt(const T* Fmt, TArgs&&...Args)
		{
			if constexpr (CString::IsAnisChar)
				char Buf[512];
			else
				wchar_t Buf[512];
			SizeType Count = CString::Snprintf(Buf, 512, Fmt, std::forward<TArgs>(Args)...);
			Append(Buf, Count);
		}
		template<typename...TArgs>
		void Format(const T* Fmt, TArgs&&...Args)
		{
			Reset();
			if constexpr (CString::IsAnisChar)
				char Buf[512];
			else
				wchar_t Buf[512];
			SizeType Count = CString::Snprintf(Buf, 512, Fmt, std::forward<TArgs>(Args)...);
			Append(Buf, Count);
		}

		// foreach
		FORCEINLINE T* begin() { return m_StrSP ? m_StrSP->Str : nullptr; }
		FORCEINLINE T* end() { return m_StrSP ? m_StrSP->Str ? m_StrSP->Str + m_StrSP->Num - 1 : nullptr : nullptr; }
		FORCEINLINE const T* begin() const { return m_StrSP ? m_StrSP->Str : nullptr; }
		FORCEINLINE const T* end() const { return m_StrSP ? m_StrSP->Str ? m_StrSP->Str + m_StrSP->Num - 1 : nullptr : nullptr; }
	};

	using String = TString<TCHAR>;
}

// Support hash
namespace Fuko
{
	template<typename T,typename TAlloc>
	uint32 GetTypeHash(const TString<T, TAlloc>& Str)
	{
		auto Data = Str.GetData();
		return Data ? Crc::StrCrc32(Data) : 0;
	}
}
