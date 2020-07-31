#pragma once
#include "JobConfig.h"

namespace Fuko::Job
{
	enum class EExecutableType : int8_t
	{
		Normal ,
		Condition ,
	};
	using NormalExec = void(*)(void*);
	using ConditionExec = int(*)(void*);

	struct alignas(16) Executable
	{
		using MemoryOpFun = void(*)(void*);
		void*				Storage;
		void*				ExecuteFunc;
		MemoryOpFun			DestroyFunc;
		EExecutableType		Type;

		inline Executable() {}
		inline ~Executable() { Destroy(); }

		inline bool IsValid() { return Storage && ExecuteFunc; }
		inline bool IsDestroyed() { return DestroyFunc == nullptr; }
		inline void Destroy() 
		{
			if (DestroyFunc && Storage)
			{
				DestroyFunc(Storage);
				DestroyFunc = nullptr;
			}
		}
		
		inline void InvokeNormal() { ((NormalExec)(ExecuteFunc))(Storage); }
		inline int  InvokeCondition() { return ((ConditionExec)(ExecuteFunc))(Storage); }
	};
}
