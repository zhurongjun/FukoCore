#pragma once
#include <CoreType.h>
#include <CoreConfig.h>
#include <Containers/Allocator.h>

// String
namespace Fuko
{
	template<typename T, typename TAlloc = TBlockAlloc<PmrAlloc>>
	class TString
	{
		TArray<T, TAlloc>	m_Data;
		using SizeType = typename TAlloc::SizeType;

	public:
		FORCEINLINE TString(const TAlloc& Alloc = TAlloc()) : m_Data(Alloc) {}
		FORCEINLINE TString(const TString& InStr, const TAlloc& Alloc = TAlloc()) : m_Data(InStr.m_Data, Alloc) {}
		FORCEINLINE TString(TString&& InStr) : m_Data(std::move(InStr.m_Data)) {}
		FORCEINLINE TString& operator=(const TString& InStr) = default;
		FORCEINLINE TString& operator=(FString&&) = default;

		// construct from raw string 
		FORCEINLINE TString(const T* Str)
		{
			if (Str && *Str)
			{
				auto Len = TCString<T>::Strlen(Str);
				auto ArrLen = Len + 1;
				m_Data.Reserve(ArrLen);
				m_Data.AddUninitialized(ArrLen);
				Memcpy(m_Data.GetData(), Str, ArrLen);
			}
		}
		FORCEINLINE TString(const T* Begin, const T* End)
		{
			auto Len = End - Begin;
			if (Len <= 0)return;
			auto ArrLen = Len + 1;
			m_Data.Reserve(ArrLen);
			m_Data.AddUninitialized(ArrLen);
			m_Data.Last() = 0;
			Memcpy(m_Data.GetData(), Begin, Len);
		}

		// get info 
		FORCEINLINE SizeType Num() { return m_Data.Num(); }
		FORCEINLINE SizeType Max() { return m_Data.Max(); }
		FORCEINLINE SizeType Len() { return m_Data.Num(); }
		FORCEINLINE T* GetData() { return m_Data.GetData(); }
		FORCEINLINE const T* GetData() const { return m_Data.GetData(); }
		FORCEINLINE T* operator*() { return GetData(); }
		FORCEINLINE const T* operator*() const { return GetData(); }

		// 



	};
}