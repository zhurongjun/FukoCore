#pragma once
#include <CoreConfig.h>
#include <CoreType.h>
#include <Templates/Align.h>

namespace Fuko
{
	// 手动构造析构的Warpper 
	template<typename T>
	class TLazyObject
	{
		TStorage<T>			m_Storage;
		TStorage<PtrCore>	m_Core;
		SP<T>				m_SP;
	public:
		TLazyObject() = default;
		~TLazyObject() = default;

		template<typename...Ts>
		FORCEINLINE SP<T> GetSP(Ts&&...Args)
		{
			if (!m_SP.IsValid())
			{
				new(&m_Storage)T(std::forward<Ts>(Args));
				m_SP = SP<T>(new(&m_Core)PtrCore(&m_Storage, &TNoFreeDeleter<T>::Destroy, nullptr));
			}
			return m_SP;
		}
	};
}
