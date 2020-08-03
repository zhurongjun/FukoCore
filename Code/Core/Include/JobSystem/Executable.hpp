#pragma once
#include "JobConfig.hpp"

namespace Fuko::Job
{
	enum class EExecutableType : int8_t
	{
		Normal ,		// Normal task
		Condition ,		// Condition task
	};

	using NormalExec = void(*)(void*);
	using ConditionExec = uint32_t(*)(void*);

	struct alignas(16) Executable
	{
		using MemoryOpFun = void(*)(void*);
		void*				Storage;
		void*				ExecuteFunc;
		MemoryOpFun			DestroyFunc;

		inline Executable()
			: Storage(nullptr)
			, ExecuteFunc(nullptr)
			, DestroyFunc(nullptr)
		{}
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
			if (Storage)
			{
				FreeExecutable(Storage);
				Storage = nullptr;
			}
		}
		
		// Invoke 
		inline void			InvokeNormal() { ((NormalExec)(ExecuteFunc))(Storage); }
		inline uint32_t		InvokeCondition() { return ((ConditionExec)(ExecuteFunc))(Storage); }

		// Bind 
		template<typename TFun>
		inline void	BindNormal(TFun&& Fun)
		{
			using RealTFun = std::remove_reference_t<TFun>;
			
			Storage = AllocExecutable(sizeof(RealTFun), alignof(RealTFun));
			new(Storage) RealTFun(std::forward<TFun>(Fun));

			NormalExec Exec;
			Exec = [](void* InPtr) { (*(RealTFun*)(InPtr))(); };
			
			ExecuteFunc = Exec;
			DestroyFunc = [](void* InPtr) { ((RealTFun*)InPtr)->~RealTFun(); };
		}
		template<typename TFun>
		inline void BindCondition(TFun&& Fun)
		{
			using RealTFun = std::remove_reference_t<TFun>;

			Storage = AllocExecutable(sizeof(RealTFun), alignof(RealTFun));
			new(Storage) RealTFun(std::forward<TFun>(Fun));

			ConditionExec Exec;
			Exec = [](void* InPtr) { return (*(RealTFun*)(InPtr))(); };
			
			ExecuteFunc = Exec;
			DestroyFunc = [](void* InPtr) { ((RealTFun*)InPtr)->~RealTFun(); };
		}

	};
}
