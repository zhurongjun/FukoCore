#pragma once
#include "CoreConfig.h"
#include "CoreType.h"

namespace Fuko::Algo
{
	/**
	 * @fn template <typename T, typename A, typename OpT> FORCEINLINE T Accumulate(const A& Input, T Init, OpT Op)
	 *
	 * @brief 累加
	 * 		  
	 * @param  Input 进行累加的数组
	 * @param  Init  初始值
	 * @param  Op    累加操作
	 *
	 * @returns 累加结果
	 */
	template <typename T, typename A, typename OpT>
	FORCEINLINE T Accumulate(const A& Input, T Init, OpT Op)
	{
		T Result = MoveTemp(Init);
		for (const auto& InputElem : Input)
		{
			Result = Invoke(Op, MoveTemp(Result), InputElem);
		}
		return Result;
	}

	/**
	 * @fn template <typename T, typename A> FORCEINLINE T Accumulate(const A& Input, T Init)
	 *
	 * @brief 累加
	 *
	 * @param  Input 进行累加的数组
	 * @param  Init  初始值
	 *
	 * @returns 累加结果
	 */
	template <typename T, typename A>
	FORCEINLINE T Accumulate(const A& Input, T Init)
	{
		return Accumulate(Input, MoveTemp(Init), TPlus<>());
	}

	/**
	 * @fn template <typename T, typename A, typename MapT, typename OpT> FORCEINLINE T TransformAccumulate(const A& Input, MapT MapOp, T Init, OpT Op)
	 *
	 * @brief 累加
	 *
	 * @param  Input 进行累加的数组
	 * @param  MapOp 映射操作
	 * @param  Init  初始值
	 * @param  Op    累加操作
	 *
	 * @returns 累加的结果
	 */
	template <typename T, typename A, typename MapT, typename OpT>
	FORCEINLINE T TransformAccumulate(const A& Input, MapT MapOp, T Init, OpT Op)
	{
		T Result = MoveTemp(Init);
		for (const auto& InputElem : Input)
		{
			Result = Invoke(Op, MoveTemp(Result), Invoke(MapOp, InputElem));
		}
		return Result;
	}

	/**
	 * @fn template <typename T, typename A, typename MapT> FORCEINLINE T TransformAccumulate(const A& Input, MapT MapOp, T Init)
	 *
	 * @brief 累加
	 *
	 * @param  Input 进行累加的数组
	 * @param  MapOp 映射操作
	 * @param  Init  初始值
	 *
	 * @returns A T.
	 */
	template <typename T, typename A, typename MapT>
	FORCEINLINE T TransformAccumulate(const A& Input, MapT MapOp, T Init)
	{
		return TransformAccumulate(Input, MapOp, MoveTemp(Init), TPlus<>());
	}
}