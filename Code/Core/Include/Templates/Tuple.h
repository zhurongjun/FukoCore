#pragma once
#include <tuple>
#include <CoreType.h>
#include <CoreConfig.h>
#include "UtilityTemp.h"
#include "Templates/TypeHash.h"

// forward
namespace Fuko
{
	template<typename ...Types>
	struct TTuple;

	template<typename TKey,typename TValue>
	struct TPair;
}
namespace std
{
	template<size_t Index, typename TKey, typename TValue>
	FORCEINLINE decltype(auto) get(const Fuko::TPair<TKey, TValue>& Pair);
}

// support structure binding
namespace std
{
	template<size_t Index, typename Type, typename...Types>
	struct tuple_element<Index, Fuko::TTuple<Type, Types...>>
		: public std::tuple_element<Index - 1, Fuko::TTuple<Types...>>
	{
	};

	template<typename Type, typename...Types>
	struct tuple_element<0, Fuko::TTuple<Type, Types...>>
	{
		using type = Type;
		using _Ttype = Fuko::TTuple<Type, Types...>;
	};

	template<size_t Index>
	struct tuple_element<Index, Fuko::TTuple<>>
	{
		static_assert(Index == 0, "TTuple out of range");
	};

	template<typename ...Types>
	struct tuple_size<Fuko::TTuple<Types...>>
	{
		static constexpr size_t value = sizeof...(Types);
	};

	template<size_t Index, typename ...Types>
	FORCEINLINE decltype(auto) get(const Fuko::TTuple<Types...>& Tuple)
	{
		using TupleType = typename std::tuple_element<Index, Fuko::TTuple<Types...>>::_Ttype;
		return static_cast<const TupleType&>(Tuple).Head();
	}

	template<size_t Index, typename ...Types>
	FORCEINLINE decltype(auto) get(Fuko::TTuple<Types...>& Tuple)
	{
		using TupleType = typename std::tuple_element<Index, Fuko::TTuple<Types...>>::_Ttype;
		return static_cast<TupleType&>(Tuple).Head();
	}

	template<size_t Index, typename ...Types>
	FORCEINLINE decltype(auto) get(Fuko::TTuple<Types...>&& Tuple)
	{
		using TupleType = typename std::tuple_element<Index, Fuko::TTuple<Types...>>::_Ttype;
		return std::move(static_cast<TupleType&>(Tuple).Head());
	}
}

// implement make tuple
namespace Fuko
{
	template <typename... Types>
	FORCEINLINE constexpr TTuple<std::decay_t<Types>...> MakeTuple(Types&&... Args)
	{
		return TTuple<std::decay_t<Types>...>(std::forward<Types>(Args)...);
	}
}

// transform tuple helper
namespace Fuko::Tuple_Private
{
	template <typename IntegerSequence>
	struct TTransformTupleHelper;

	template <uint32... Indices>
	struct TTransformTupleHelper<std::integer_sequence<uint32, Indices...>>
	{
		template <typename TupleType, typename FuncType>
		static decltype(auto) Do(TupleType&& Tuple, FuncType Func)
		{
			return MakeTuple(Func(std::get<Indices>(std::forward<TupleType>(Tuple)))...);
		}
	};
}

// visit tuple helper
namespace Fuko::Tuple_Private
{
	template<typename Sequence>
	struct TVisitTupleHelper;

	template<uint32...Indices>
	struct TVisitTupleHelper<std::integer_sequence<uint32,Indices...>>
	{
		template<uint32 Index,typename FuncType,typename...TupleTypes>
		FORCEINLINE static void CallFunc(FuncType&& Func, TupleTypes&&...Tuples)
		{
			std::invoke(std::forward<FuncType>(Func), std::get<Index>(std::forward<TupleTypes>(Tuples))...);
		}
		template<uint32 Index, typename FuncType, typename...TupleTypes>
		FORCEINLINE static void CallFunc(FuncType&& Func, const TupleTypes&...Tuples)
		{
			std::invoke(std::forward<FuncType>(Func), std::get<Index>(Tuples)...);
		}

		template<typename FuncType,typename...TupleTypes>
		FORCEINLINE static void Do(FuncType&& Func, TupleTypes&&...Tuples)
		{
			int Temp[] = { 0,(CallFunc<Indices>(std::forward<FuncType>(Func),std::forward<TupleTypes>(Tuples)...),0)... };
			(void)Temp;
		}
		template<typename FuncType, typename...TupleTypes>
		FORCEINLINE static void Do(FuncType&& Func, const TupleTypes&...Tuples)
		{
			int Temp[] = { 0,(CallFunc<Indices>(std::forward<FuncType>(Func),Tuples...),0)... };
			(void)Temp;
		}
	};
}

// apply tuple helper
namespace Fuko::Tuple_Private
{
	template<typename Sequence>
	struct TApplyTupleHelper;

	template<uint32...Indices>
	struct TApplyTupleHelper<std::integer_sequence<uint32, Indices...>>
	{
		template<typename TFun,typename TTup>
		FORCEINLINE static decltype(auto) Apply(TFun&& Func, TTup&& Tp)
		{
			return Func(Tp.Get<Indices>()...);
		}

		template<typename TFun, typename TTup, typename...TArgs>
		FORCEINLINE static decltype(auto) ApplyBefore(TFun&& Func, TTup&& Tp, TArgs&&...Args)
		{
			return Func(Tp.Get<Indices>()..., std::forward<TArgs>(Args)...);
		}

		template<typename TFun, typename TTup, typename...TArgs>
		FORCEINLINE static decltype(auto) ApplyAfter(TFun&& Func, TTup&& Tp, TArgs&&...Args)
		{
			return Func(std::forward<TArgs>(Args)..., Tp.Get<Indices>()...);
		}
	};
}

// implement tuple
namespace Fuko
{
	template<>
	struct TTuple<>
	{
		FORCEINLINE bool operator==(const TTuple& Other) const { return true; }
		FORCEINLINE bool operator<(const TTuple& Other) const { return true; }
	};

	template<typename Type, typename...Types>
	struct TTuple<Type,Types...> : public TTuple<Types...>
	{
		using BaseType = TTuple<Types...>;
		using ThisType = TTuple<Type, Types...>;
		using ValueType = Type;

		// construct
		FORCEINLINE constexpr TTuple() = default;
		FORCEINLINE constexpr TTuple(Type&& Arg, Types&&...Args) : BaseType(std::forward<Types>(Args)...) , _Value(std::forward<Type>(Arg)) {}
		FORCEINLINE constexpr TTuple(const Type& Arg, const Types&... Args) : BaseType(Args...), _Value(Arg) {}

		// copy & move
		FORCEINLINE TTuple(TTuple&& Other) = default;
		FORCEINLINE TTuple(const TTuple& Other) = default;

		// head
		FORCEINLINE const Type &Head() const { return _Value; }
		FORCEINLINE Type& Head() { return _Value; }

		// assign
		FORCEINLINE TTuple& operator=(TTuple&& Other) = default;
		FORCEINLINE TTuple& operator=(const TTuple& Other) = default;

		// equal & not equal
		FORCEINLINE bool operator==(const ThisType& Other) const
		{
			return (_Value == Other._Value) && BaseType::operator==(static_cast<const BaseType&>(Other));
		}
		FORCEINLINE bool operator!=(const ThisType& Other) const
		{
			return !(*this == Other);
		}

		// compare
		FORCEINLINE bool operator<(const ThisType& Other) const
		{
			return (_Value < Other._Value) && BaseType::operator<(static_cast<const BaseType&>(Other));
		}
		FORCEINLINE bool operator<=(const ThisType& Other) const
		{
			return !(Other < *this);
		}
		FORCEINLINE bool operator>(const ThisType& Other) const
		{
			return Other < *this;
		}
		FORCEINLINE bool operator>=(const ThisType& Other) const
		{
			return !(*this < Other);
		}

		// get
		template<uint32 Index> 
		FORCEINLINE const std::tuple_element_t<Index,TTuple<Type,Types...>>& 
			Get() const { return std::get<Index>(*this); }
		template<uint32 Index> 
		FORCEINLINE std::tuple_element_t<Index, TTuple<Type, Types...>>&
			Get() { return std::get<Index>(*this); }

		// each
		template<typename TFun>
		FORCEINLINE void Each(TFun&& Func)
		{
			VisitTupleElements(std::forward<FuncType>(Func), *this);
		}
		template<typename TFun>
		FORCEINLINE void Each(TFun&& Func) const
		{
			VisitTupleElements(std::forward<FuncType>(Func), *this);
		}

		// apply
		template<typename TFun>
		FORCEINLINE decltype(auto) Apply(TFun&& Func)
		{
			return ApplyTuple(std::forward<TFun>(Func), *this);
		}
		template<typename TFun,typename...TArgs>
		FORCEINLINE decltype(auto) ApplyBefore(TFun&& Func, TArgs...Args)
		{
			return ApplyTupleBefore(std::forward<TFun>(Func), *this, Args...);
		}
		template<typename TFun, typename...TArgs>
		FORCEINLINE decltype(auto) ApplyAfter(TFun&& Func, TArgs...Args)
		{
			return ApplyTupleAfter(std::forward<TFun>(Func), *this, Args...);
		}
		template<typename TFun>
		FORCEINLINE decltype(auto) Apply(TFun&& Func) const
		{
			return ApplyTuple(std::forward<TFun>(Func), *this);
		}
		template<typename TFun, typename...TArgs>
		FORCEINLINE decltype(auto) ApplyBefore(TFun&& Func, TArgs...Args) const
		{
			return ApplyTupleBefore(std::forward<TFun>(Func), *this, Args...);
		}
		template<typename TFun, typename...TArgs>
		FORCEINLINE decltype(auto) ApplyAfter(TFun&& Func, TArgs...Args) const
		{
			return ApplyTupleAfter(std::forward<TFun>(Func), *this, Args...);
		}

	protected:
		Type		_Value;
	};
}

// implement visit
namespace Fuko
{
	template <typename FuncType, typename FirstTupleType, typename... TupleTypes>
	FORCEINLINE void VisitTupleElements(FuncType&& Func, FirstTupleType&& FirstTuple, TupleTypes&&...Tuples)
	{
		Tuple_Private::TVisitTupleHelper <
			std::make_integer_sequence<uint32, std::tuple_size_v<std::decay_t<FirstTupleType>>>
		>::Do(std::forward<FuncType>(Func), std::forward<FirstTupleType>(FirstTuple), std::forward<TupleTypes>(Tuples)...);
	}
	template <typename FuncType, typename FirstTupleType, typename... TupleTypes>
	FORCEINLINE void VisitTupleElements(FuncType&& Func, const FirstTupleType& FirstTuple, const TupleTypes&...Tuples)
	{
		Tuple_Private::TVisitTupleHelper <
			std::make_integer_sequence<uint32, std::tuple_size_v<std::decay_t<FirstTupleType>>>
		>::Do(std::forward<FuncType>(Func), FirstTuple, Tuples...);
	}
}

// implement apply
namespace Fuko
{
	template<typename TFun,typename TTup>
	FORCEINLINE decltype(auto) ApplyTuple(TFun&& Func, TTup&& Tup)
	{
		return Tuple_Private::TApplyTupleHelper<
			std::make_integer_sequence<uint32, std::tuple_size_v<std::decay_t<TTup>>>
		>::Apply(std::forward<TFun>(Func), std::forward<TTup>(Tup));
	}

	template<typename TFun, typename TTup,typename...TArgs>
	FORCEINLINE decltype(auto) ApplyTupleBefore(TFun&& Func, TTup&& Tup, TArgs...Args)
	{
		return Tuple_Private::TApplyTupleHelper<
			std::make_integer_sequence<uint32, std::tuple_size_v<std::decay_t<TTup>>>
		>::ApplyBefore(std::forward<TFun>(Func), std::forward<TTup>(Tup), std::forward<TArgs>(Args)...);
	}

	template<typename TFun, typename TTup, typename...TArgs>
	FORCEINLINE decltype(auto) ApplyTupleAfter(TFun&& Func, TTup&& Tup, TArgs...Args)
	{
		return Tuple_Private::TApplyTupleHelper<
			std::make_integer_sequence<uint32, std::tuple_size_v<std::decay_t<TTup>>>
		>::ApplyAfter(std::forward<TFun>(Func), std::forward<TTup>(Tup), std::forward<TArgs>(Args)...);
	}
}

// implement transform
namespace Fuko
{
	template <typename FuncType, typename... Types>
	FORCEINLINE decltype(auto) TransformTuple(TTuple<Types...>&& Tuple, FuncType&& Func)
	{
		return Tuple_Private::TTransformTupleHelper<
			std::make_integer_sequence<uint32, sizeof...(Types)>
		>
			::Do(std::move(Tuple), std::forward<FuncType>(Func));
	}
}

// implement gethash
namespace Fuko
{
	template <typename... Types>
	FORCEINLINE uint32 GetTypeHash(const TTuple<Types...>& Tuple)
	{
		uint32 RetHash = 0;
		if constexpr (sizeof...(Types) == 1)
			RetHash = GetTypeHash(Tuple.Head());
		else
			Tuple.Each([&](auto Element) { RetHash = HashCombine(RetHash, GetTypeHash(Element)); });
		return RetHash;
	}

	FORCEINLINE uint32 GetTypeHash(const TTuple<>& Tuple)
	{
		return 0;
	}
}
