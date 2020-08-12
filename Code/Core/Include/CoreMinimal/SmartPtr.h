#pragma once 
#include <CoreConfig.h>
#include <CoreType.h>
#include <Memory/MemoryPolicy.h>

// PtrCore 
namespace Fuko
{
	// contain refcounts and ptr 
	class PtrCore final
	{
		using TDestructor = void(*)(void*);

		std::atomic<TDestructor>	m_Destructor;
		std::atomic<void*>			m_Ptr;
		std::atomic<uint32>			m_StrongCount;
		std::atomic<uint32>			m_WeakCount;
	public:
		FORCEINLINE void _DestructPtr()
		{
			check(m_Destructor);
			m_Destructor.load()(m_Ptr);
			m_Destructor = nullptr;
			m_Ptr = nullptr;
		}
		FORCEINLINE PtrCore(void* Ptr,TDestructor Destructor) 
			: m_Destructor(Destructor)
			, m_Ptr(Ptr)
			, m_StrongCount(0)
			, m_WeakCount(0) 
		{}

		FORCEINLINE bool TimeToDie() const { return m_WeakCount == 0 && m_StrongCount == 0; }
		FORCEINLINE bool IsValid() const { return m_Ptr != nullptr && m_Destructor != nullptr; }

		FORCEINLINE uint32 SRelease() { auto CurCount = --m_StrongCount; if (CurCount == 0) _DestructPtr(); return CurCount; }
		FORCEINLINE uint32 WRelease() { return --m_WeakCount; }

		FORCEINLINE uint32 SRetain() { return ++m_StrongCount; }
		FORCEINLINE uint32 WRetain() { return ++m_WeakCount; }

		FORCEINLINE uint32 SCount() const { return m_StrongCount; }
		FORCEINLINE uint32 WCount() const { return m_WeakCount; }

		FORCEINLINE void* Ptr() const { return m_Ptr; }

		FORCEINLINE static PtrCore* AllocCore(void* Ptr, TDestructor Destructor)
		{
			PtrCore* Core = (PtrCore*)PoolMAlloc(sizeof(PtrCore), alignof(PtrCore));
			new (Core)PtrCore(Ptr, Destructor);
			return Core;
		}
		FORCEINLINE static void FreeCore(PtrCore* Core)
		{
			Core->~PtrCore();
			PoolFree(Core);
		}
	};
}

// DestroyPolicy
namespace Fuko
{
	template<typename T> struct TRawDeleter  { static void Destroy(void* Obj) { delete (T*)Obj; } };
	template<typename T> struct TFukoDeleter { static void Destroy(void* Obj) { ((T*)Obj)->~T(); Free(Obj); } };
	template<typename T> struct TPoolDeleter { static void Destroy(void* Obj) { ((T*)Obj)->~T(); PoolFree(Obj); } };
}

// SP
namespace Fuko
{
	template<typename T>
	class SP final
	{
		template<typename T>
		friend class WP;

		PtrCore*	m_Core;

		FORCEINLINE void _Detach();
	public:
		FORCEINLINE SP();
		FORCEINLINE SP(T* Inptr);
		template<typename Deleter>
		FORCEINLINE SP(T* InPtr, Deleter);
		FORCEINLINE SP(PtrCore* Core);

		FORCEINLINE ~SP() { _Detach(); }

		FORCEINLINE SP(const SP& Rhs);
		FORCEINLINE SP(SP&& Rhs);

		FORCEINLINE SP& operator=(const SP& Rhs);
		FORCEINLINE SP& operator=(SP&& Rhs);

		FORCEINLINE operator bool() { return m_Core; }
		FORCEINLINE bool IsValid() { return m_Core != nullptr; }
		FORCEINLINE T* Get() { return m_Core ? (T*)m_Core->Ptr() : nullptr; }
		FORCEINLINE const T* Get() const { return m_Core ? (T*)m_Core->Ptr() : nullptr; }
		FORCEINLINE T* operator->() { return m_Core ? (T*)m_Core->Ptr() : nullptr; }
		FORCEINLINE const T* operator->() const { return m_Core ? (T*)m_Core->Ptr() : nullptr; }
		FORCEINLINE T& operator*() { return *Get(); }
		FORCEINLINE const T& operator*() const { return *Get(); }

		FORCEINLINE bool operator==(const SP& Rhs) const { return Get() == Rhs.Get(); }
		FORCEINLINE bool operator!=(const SP& Rhs) const { return *this != Rhs; }
		FORCEINLINE bool operator< (const SP& Rhs) const { return Get() < Rhs.Get(); }
		FORCEINLINE bool operator<=(const SP& Rhs) const { return Get() <= Rhs.Get(); }
		FORCEINLINE bool operator> (const SP& Rhs) const { return Get() > Rhs.Get(); }
		FORCEINLINE bool operator>=(const SP& Rhs) const { return Get() >= Rhs.Get(); }

		FORCEINLINE void Reset() { _Detach(); }
		FORCEINLINE bool Unique();
	};

	template<typename T,typename...Ts>
	FORCEINLINE SP<T> MakeSP(Ts&&...Args)
	{
		T* Memory = (T*)Alloc(sizeof(T), alignof(T));
		return SP<T>(new(Memory)T(std::forward<Ts>(Args)...));
	}
}

// WP
namespace Fuko
{
	template<typename T>
	class WP final
	{
		PtrCore*	m_Core;

		FORCEINLINE void _Detach();
	public:
		FORCEINLINE WP();
		FORCEINLINE WP(const SP<T>& InSp);

		FORCEINLINE ~WP() { _Detach(); }

		FORCEINLINE WP(const WP& Rhs);
		FORCEINLINE WP(WP&& Rhs);

		FORCEINLINE WP& operator=(const WP& Rhs);
		FORCEINLINE WP& operator=(WP&& Rhs);

		FORCEINLINE operator bool() { return m_Core; }
		FORCEINLINE bool IsValid() { return m_Core != nullptr && m_Core->IsValid(); }
		FORCEINLINE SP<T> Lock();

		FORCEINLINE bool operator==(const WP& Rhs) const { return this->Lock() == Rhs.Lock(); }
		FORCEINLINE bool operator!=(const WP& Rhs) const { return this->Lock() != Rhs.Lock(); }
		FORCEINLINE bool operator< (const WP& Rhs) const { return this->Lock() <  Rhs.Lock(); }
		FORCEINLINE bool operator<=(const WP& Rhs) const { return this->Lock() <= Rhs.Lock(); }
		FORCEINLINE bool operator> (const WP& Rhs) const { return this->Lock() >  Rhs.Lock(); }
		FORCEINLINE bool operator>=(const WP& Rhs) const { return this->Lock() >= Rhs.Lock(); }

		FORCEINLINE void Reset() { _Detach(); }
	};
}

// UP
namespace Fuko
{
	template<typename T>
	class UP final
	{
		using TDestructor = void(*)(void*);

		std::atomic<TDestructor>	m_Destructor;
		std::atomic<T*>			m_Ptr;

	public:
		FORCEINLINE UP();
		FORCEINLINE UP(T* Inptr);
		template<typename Deleter>
		FORCEINLINE UP(T* InPtr, Deleter);

		FORCEINLINE ~UP() { Reset(); }

		FORCEINLINE UP(const UP& Rhs) = delete;
		FORCEINLINE UP(UP& Rhs);
		FORCEINLINE UP(UP&& Rhs);

		FORCEINLINE UP& operator=(const UP& Rhs) = delete;
		FORCEINLINE UP& operator=(UP& Rhs);
		FORCEINLINE UP& operator=(UP&& Rhs);

		FORCEINLINE operator bool() { return m_Ptr; }
		FORCEINLINE bool IsValid() { return m_Ptr != nullptr; }
		FORCEINLINE T* Get() { return m_Ptr; }
		FORCEINLINE const T* Get() const { return m_Ptr; }
		FORCEINLINE T* operator->() { return m_Ptr; }
		FORCEINLINE const T* operator->() const { return m_Ptr; }
		FORCEINLINE T& operator*() { return *Get(); }
		FORCEINLINE const T& operator*() const { return *Get(); }

		FORCEINLINE bool operator==(const UP& Rhs) const { return Get() == Rhs.Get(); }
		FORCEINLINE bool operator!=(const UP& Rhs) const { return *this != Rhs; }
		FORCEINLINE bool operator< (const UP& Rhs) const { return Get() < Rhs.Get(); }
		FORCEINLINE bool operator<=(const UP& Rhs) const { return Get() <= Rhs.Get(); }
		FORCEINLINE bool operator> (const UP& Rhs) const { return Get() > Rhs.Get(); }
		FORCEINLINE bool operator>=(const UP& Rhs) const { return Get() >= Rhs.Get(); }

		FORCEINLINE void Reset();
	};

	template<typename T,typename...Ts>
	FORCEINLINE UP<T> MakeUP(Ts&&...Args)
	{
		T* Memory = (T*)Alloc(sizeof(T), alignof(T));
		return UP<T>(new(Memory)T(std::forward<Ts>(Args)...));
	}
}

// Impl SP 
namespace Fuko
{
	template<typename T>
	FORCEINLINE void SP<T>::_Detach()
	{
		if (m_Core)
		{
			m_Core->SRelease();
			if (m_Core->TimeToDie())
			{
				PtrCore::FreeCore(m_Core);
			}
			m_Core = nullptr;
		}
	}

	template<typename T>
	FORCEINLINE SP<T>::SP() : m_Core(nullptr) {}

	template<typename T>
	FORCEINLINE SP<T>::SP(T* Inptr)
		: m_Core(PtrCore::AllocCore(Inptr, &TFukoDeleter<T>::Destroy))
	{
		check(Inptr);
		m_Core->SRetain();
	}

	template<typename T>
	template<typename Deleter>
	FORCEINLINE SP<T>::SP(T* InPtr, Deleter)
		: m_Core(PtrCore::AllocCore(Inptr, &Deleter::Destroy))
	{
		check(InPtr);
		if(m_Core) m_Core->SRetain();
	}

	template<typename T>
	FORCEINLINE SP<T>::SP(PtrCore* Core)
		: m_Core(Core)
	{
		check(m_Core);
		m_Core->SRetain();
	}

	template<typename T>
	FORCEINLINE SP<T>::SP(const SP<T>& Rhs)
		: m_Core(Rhs.m_Core)
	{
		if (m_Core) m_Core->SRetain();
	}

	template<typename T>
	FORCEINLINE SP<T>::SP(SP<T>&& Rhs)
		: m_Core(Rhs.m_Core)
	{
		Rhs.m_Core = nullptr;
	}

	template<typename T>
	FORCEINLINE SP<T>& SP<T>::operator=(const SP<T>& Rhs)
	{
		_Detach();
		m_Core = Rhs.m_Core;
		if (m_Core) m_Core->SRetain();
		return *this;
	}

	template<typename T>
	FORCEINLINE SP<T>& SP<T>::operator=(SP<T>&& Rhs)
	{
		_Detach();
		m_Core = Rhs.m_Core;
		Rhs.m_Core = nullptr;
		return *this;
	}

	template<typename T>
	FORCEINLINE bool SP<T>::Unique()
	{
		if (!m_Core) return false;
		return m_Core->SCount() == 1;
	}
}

// Impl WP 
namespace Fuko
{
	template<typename T>
	FORCEINLINE void WP<T>::_Detach()
	{
		if (m_Core)
		{
			m_Core->WRelease();
			if (m_Core->TimeToDie())
			{
				PtrCore::FreeCore(m_Core);
			}
			m_Core = nullptr;
		}
	}

	template<typename T>
	FORCEINLINE WP<T>::WP() : m_Core(nullptr) {}

	template<typename T>
	FORCEINLINE WP<T>::WP(const SP<T>& InSp)
		: m_Core(InSp->m_Core)
	{
		if (m_Core) m_Core->WRetain();
	}

	template<typename T>
	FORCEINLINE WP<T>::WP(const WP<T>& Rhs)
		: m_Core(Rhs.m_Core)
	{
		if (m_Core) m_Core->WRetain();
	}

	template<typename T>
	FORCEINLINE WP<T>::WP(WP<T>&& Rhs)
		: m_Core(Rhs.m_Core)
	{
		Rhs.m_Core = nullptr;
	}

	template<typename T>
	FORCEINLINE WP<T>& WP<T>::operator=(const WP<T>& Rhs)
	{
		 _Detach();
		 m_Core = Rhs.m_Core;
		 if (m_Core) m_Core->WRetain();
		 return *this;
	}
	
	template<typename T>
	FORCEINLINE WP<T>& WP<T>::operator=(WP<T>&& Rhs)
	{
		_Detach();
		m_Core = Rhs.m_Core;
		Rhs.m_Core = nullptr;
		return *this;
	}

	template<typename T>
	FORCEINLINE SP<T> WP<T>::Lock()
	{
		if (!IsValid()) return SP<T>();
		return SP<T>(m_Core);
	}
}

// Impl UP 
namespace Fuko
{
	template<typename T>
	FORCEINLINE UP<T>::UP()
		: m_Ptr(nullptr)
		, m_Destructor(nullptr)
	{}

	template<typename T>
	FORCEINLINE UP<T>::UP(T* Inptr)
		: m_Ptr(Inptr)
		, m_Destructor(&TFukoDeleter<T>::Destroy)
	{
		check(Inptr);
	}

	template<typename T>
	template<typename Deleter>
	FORCEINLINE UP<T>::UP(T* InPtr, Deleter)
		: m_Ptr(Inptr)
		, m_Destructor(&Deleter::Destroy)
	{
		check(InPtr);
	}

	template<typename T>
	FORCEINLINE UP<T>::UP(UP& Rhs)
		: m_Ptr(Rhs.m_Ptr)
		, m_Destructor(Rhs.m_Destructor)
	{
		Rhs.m_Ptr = nullptr;
		Rhs.m_Destructor = nullptr;
	}
	
	template<typename T>
	FORCEINLINE UP<T>::UP(UP<T>&& Rhs)
		: m_Ptr(Rhs.m_Ptr)
		, m_Destructor(Rhs.m_Destructor)
	{
		Rhs.m_Ptr = nullptr;
		Rhs.m_Destructor = nullptr;
	}

	template<typename T>
	FORCEINLINE UP<T>& UP<T>::operator=(UP& Rhs)
	{
		Reset();
		m_Ptr = Rhs.m_Ptr;
		m_Destructor = Rhs.m_Destructor;
		Rhs.m_Ptr = nullptr;
		Rhs.m_Destructor = nullptr;
	}

	template<typename T>
	FORCEINLINE UP<T>& UP<T>::operator=(UP<T>&& Rhs)
	{
		Reset();
		m_Ptr = Rhs.m_Ptr;
		m_Destructor = Rhs.m_Destructor;
		Rhs.m_Ptr = nullptr;
		Rhs.m_Destructor = nullptr;
	}

	template<typename T>
	FORCEINLINE void UP<T>::Reset()
	{
		if (m_Ptr && m_Destructor)
		{
			m_Destructor.load()(m_Ptr);
			m_Destructor = nullptr;
			m_Ptr = nullptr;
		}
	}
}
