#pragma once
#include <CoreConfig.h>
#include <CoreType.h>
#include <Containers/Allocator.h>
#include <Templates/TypeList.h>

// forward 
namespace Fuko
{
	template<typename TFun, typename TAlloc = TBlockAlloc<PmrAlloc>>
	class TDelegate;
	template<typename TFun>
	class TBaseDelegate;
	template<typename TFun, typename...TVars>
	class TBaseStaticDelegate;
	template<bool IsConst, typename TFun, typename TClass, typename TVarList>
	class TBaseMemberDelegate;
	template<typename TFunctor, typename TFun, typename...TVars>
	class TBaseFunctorDelegate;
}

// Delegate 
namespace Fuko
{
	template<typename TR,typename...TParams,typename TAlloc>
	class TDelegate<TR(TParams...), TAlloc>
	{
		using TDelegateInstance = TBaseDelegate<TR(TParams...)>;
		using SizeType = typename TAlloc::USizeType;

		template<typename...TVars>
		using TBaseStatic = TBaseStaticDelegate<TR(TParams...), TVars...>;
		template<bool IsConst,typename TClass,typename...TVars>
		using TBaseMember = TBaseMemberDelegate<IsConst, TR(TParams...), TClass, TVars...>;
		template<typename TFun,typename...TVars>
		using TBaseFunctor = TBaseFunctorDelegate<TFun, TR(TParams...), TVars...>;

		template<typename...TVars>
		using TFunPtr = typename TBaseStatic<TVars...>::TFunPtr;
		template<typename TClass, typename...TVars>
		using TMemFunPtr = typename TBaseMember<false, TClass, TVars...>::TFunPtr;
		template<typename TClass, typename...TVars>
		using TConstMemFunPtr = typename TBaseMember<true, TClass, TVars...>::TFunPtr;

		//-------------------------Begin Help Function-------------------------
		FORCEINLINE void _FreeDelegate()
		{
			if (m_pInstance)
			{
				m_pInstance->~TDelegateInstance();
				m_InstanceSize = m_Alloc.FreeRaw((void*&)m_pInstance, m_InstanceAlign);
				m_InstanceAlign = 0;
			}
		}
		FORCEINLINE void _Resize(SizeType NewSize, SizeType NewAlign)
		{
			if (NewSize <= m_InstanceSize && NewAlign <= m_InstanceAlign) return;
			m_InstanceSize = m_Alloc.ReserveRaw((void*&)m_pInstance, NewSize, NewAlign);
			m_InstanceAlign = NewAlign;
		}
		//--------------------------End Help Function--------------------------
	public:
		// construct 
		TDelegate(const TAlloc& InAlloc = TAlloc()) 
			: m_pInstance(nullptr) 
			, m_Alloc(InAlloc) 
			, m_InstanceSize(0)
			, m_InstanceAlign(0)
		{}
		
		// copy construct 
		TDelegate(const TDelegate& Other)
			: m_pInstance(nullptr)
			, m_Alloc(Other.m_Alloc)
			, m_InstanceSize(Other.m_InstanceSize)
			, m_InstanceAlign(Other.m_InstanceAlign)
		{
			if (Other.m_pInstance)
			{
				m_Alloc.Reserve(m_pInstance, m_InstanceSize, m_InstanceAlign);
				Other.m_pInstance->CreateCopy(m_pInstance);
			}
		}
		TDelegate(const TDelegate& Other, const TAlloc& InAlloc)
			: m_pInstance(nullptr)
			, m_Alloc(InAlloc)
			, m_InstanceSize(Other.m_InstanceSize)
			, m_InstanceAlign(Other.m_InstanceAlign)
		{
			if (Other.m_pInstance)
			{
				m_Alloc.Reserve(m_pInstance, m_InstanceSize, m_InstanceAlign);
				Other.m_pInstance->CreateCopy(m_pInstance);
			}
		}

		// move construct
		TDelegate(TDelegate&& Other)
			: m_pInstance(Other.m_pInstance)
			, m_Alloc(Other.m_Alloc)
			, m_InstanceSize(Other.m_InstanceSize)
			, m_InstanceAlign(Other.m_InstanceAlign)
		{ 
			Other.m_pInstance = nullptr; 
			Other.m_InstanceSize = 0;
			Other.m_InstanceAlign = 0;
		}

		// copy assign
		TDelegate& operator=(const TDelegate& Other)
		{
			_FreeDelegate();
			if (Other.m_pInstance)
			{
				m_InstanceSize = Other.m_InstanceSize;
				m_InstanceAlign = Other.m_InstanceAlign;
				m_Alloc.ReserveRaw(m_pInstance, m_InstanceSize, m_InstanceAlign);
				Other.m_pInstance->CreateCopy(m_pInstance);
			}
		}

		// move assign
		TDelegate& operator=(TDelegate&& Other)
		{
			_FreeDelegate();
			m_pInstance = Other.m_pInstance;
			m_InstanceSize = Other.m_InstanceSize;
			m_InstanceAlign = Other.m_InstanceAlign;
			m_Alloc = Other.m_Alloc;
		}

		// destruct 
		~TDelegate() { _FreeDelegate(); }

		// validate
		FORCEINLINE bool IsValid() { return m_pInstance != nullptr; }

		// invoke 
		FORCEINLINE TR Invoke(TParams...Args) { if (IsValid()) return m_pInstance->Invoke(Args...); return TR(); }
		FORCEINLINE TR operator()(TParams...Args) { if (IsValid()) return m_pInstance->Invoke(Args...); return TR(); }

		// bind
		template<typename TFun,typename...TVars>
		FORCEINLINE void Bind(TFun&& Func, TVars...Vars)
		{
			if constexpr (std::is_member_function_pointer_v<TFun>)
			{
				using FunType = TBaseMember<TIsConstMemeberFunPtr_v<TFun>, 
					std::remove_pointer_t<TNthType_t<0,TVars...>> ,
					TRightTypes_t<1, TypeList<TVars...>>>;
				if (m_pInstance) m_pInstance->~TDelegateInstance();
				SizeType NewSize = sizeof(FunType);
				SizeType NewAlign = alignof(FunType);
				_Resize(NewSize, NewAlign);
				new(m_pInstance) FunType(Func, Vars...);
			}
			else if constexpr (std::is_pointer_v<TFun>)
			{
				using FunType = TBaseStatic<TVars...>;
				if (m_pInstance) m_pInstance->~TDelegateInstance();
				SizeType NewSize = sizeof(FunType);
				SizeType NewAlign = alignof(FunType);
				_Resize(NewSize, NewAlign);
				new(m_pInstance) FunType(Func, Vars...);
			}
			else
			{
				using FunType = TBaseFunctor<std::decay_t<TFun>, TVars...>;
				if (m_pInstance) m_pInstance->~TDelegateInstance();
				SizeType NewSize = sizeof(FunType);
				SizeType NewAlign = alignof(FunType);
				_Resize(NewSize, NewAlign);
				new(m_pInstance) FunType(std::forward<TFun>(Func), Vars...);
			}
		}

	private:
		TAlloc				m_Alloc;
		TDelegateInstance*	m_pInstance;
		SizeType			m_InstanceSize;
		SizeType			m_InstanceAlign;
	};
}

// Base Delegate
namespace Fuko
{
	template<typename TR,typename...TParams>
	class TBaseDelegate<TR(TParams...)>
	{
	public:
		virtual ~TBaseDelegate() {};
		virtual TR Invoke(TParams...Args) = 0;
		virtual void CreateCopy(void* Ptr) const = 0;

		FORCEINLINE TR operator()(TParams...Args) { return this->Invoke(Args...); }
	};
}

// Static Delegate
namespace Fuko
{
	template<typename TR,typename...TParams,typename...TVars>
	class TBaseStaticDelegate<TR(TParams...), TVars...>
		: public TBaseDelegate<TR(TParams...)>
		, TTuple<TVars...>
	{
	public:
		using TFunPtr = TR(*)(TParams..., TVars...);

		TBaseStaticDelegate(TFunPtr InFun, TVars...Vars)
			: TTuple<TVars...>(std::forward<TVars>(Vars)...)
			, m_pFun(InFun)
		{}

		virtual TR Invoke(TParams...Args) override
		{
			if constexpr (sizeof...(TVars) != 0)
				return ApplyAfter(m_pFun, Args...);
			else
				return m_pFun(Args...);
		}
		virtual void CreateCopy(void* Ptr) const override
		{
			if constexpr (sizeof...(TVars) != 0)
				Apply([&](TVars...Vars) { new(Ptr)TBaseStaticDelegate(m_pFun, Vars...); });
			else
				new(Ptr) TBaseStaticDelegate(m_pFun);
		}

	private:
		TFunPtr		m_pFun;
	};
}

// Member Delegate
namespace Fuko
{
	// not const 
	template<typename TClass, typename TR, typename...TParams, typename...TVars>
	class TBaseMemberDelegate<false, TR(TParams...), TClass, TypeList<TVars...>>
		: public TBaseDelegate<TR(TParams...)>
		, TTuple<TVars...>
	{
		static_assert(!std::is_const_v<TClass>);
	public:
		using TFunPtr = TR(TClass::*)(TParams..., TVars...);

		TBaseMemberDelegate(TFunPtr InFun, TClass* Class, TVars...Vars)
			: TTuple<TVars...>(std::forward<TVars>(Vars)...)
			, m_pFun(InFun)
		{}

		virtual TR Invoke(TParams...Args) override
		{
			if constexpr (sizeof...(TVars) != 0)
				return ApplyAfter([&](TParams...LParams, TVars...LVars)->TR { return (m_pObj->*m_pFun)(LParams..., LVars...); }, Args...);
			else
				return (m_pObj->*m_pFun)(Args...);
		}
		virtual void CreateCopy(void* Ptr) const override
		{
			if constexpr (sizeof...(TVars) != 0)
				Apply([&](TVars...Vars) { new(Ptr)TBaseMemberDelegate(m_pFun, m_pObj, Vars...); });
			else
				new(Ptr) TBaseMemberDelegate(m_pFun, m_pObj);
		}

	private:
		TFunPtr		m_pFun;
		TClass*		m_pObj;
	};

	// const 
	template<typename TClass, typename TR, typename...TParams, typename...TVars>
	class TBaseMemberDelegate<true, TR(TParams...), TClass, TypeList<TVars...>>
		: public TBaseDelegate<TR(TParams...)>
		, TTuple<TVars...>
	{
	public:
		using TFunPtr = TR(TClass::*)(TParams..., TVars...) const;

		TBaseMemberDelegate(TFunPtr InFun, const TClass* Class, TVars...Vars)
			: TTuple<TVars...>(std::forward<TVars>(Vars)...)
			, m_pFun(InFun)
		{}

		virtual TR Invoke(TParams...Args) override
		{
			if constexpr (sizeof...(TVars) != 0)
				return ApplyAfter([&](TParams...LParams, TVars...LVars)->TR { return (m_pObj->*m_pFun)(LParams..., LVars...); }, Args...);
			else
				return (m_pObj->*m_pFun)(Args...);
		}
		virtual void CreateCopy(void* Ptr) const override
		{
			if constexpr (sizeof...(TVars) != 0)
				Apply([&](TVars...Vars) { new(Ptr)TBaseMemberDelegate(m_pFun, m_pObj, Vars...); });
			else
				new(Ptr) TBaseMemberDelegate(m_pFun, m_pObj);
		}

	private:
		TFunPtr			m_pFun;
		const TClass*	m_pObj;
	};
}

// Functor Delegate
namespace Fuko
{
	template<typename TFunctor, typename TR, typename...TParams, typename...TVars>
	class TBaseFunctorDelegate<TFunctor, TR(TParams...), TVars...>
		: public TBaseDelegate<TR(TParams...)>
		, public TTuple<TVars...>
	{
	public:
		TBaseFunctorDelegate(const TFunctor& InFun, TVars...Vars)
			: TTuple<TVars...>(std::forward<TVars>(Vars)...)
			, m_Functor(InFun)
		{}
		TBaseFunctorDelegate(TFunctor&& InFun,TVars...Vars)
			: TTuple<TVars...>(std::forward<TVars>(Vars)...)
			, m_Functor(std::move(InFun))
		{}

		virtual TR Invoke(TParams...Args) override
		{
			if constexpr (sizeof...(TVars) != 0)
				return ApplyAfter(m_Functor, Args...);
			else
				return m_Functor(Args...);
		}
		virtual void CreateCopy(void* Ptr) const override
		{
			if constexpr (sizeof...(TVars) != 0)
				Apply([&](TVars...Vars) { new(Ptr)TBaseFunctorDelegate(m_Functor, Vars...); });
			else
				new(Ptr) TBaseFunctorDelegate(m_Functor);
		}

	private:
		TFunctor	m_Functor;
	};
}
