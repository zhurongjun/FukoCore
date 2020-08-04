#pragma once
#include "JobConfig.hpp"

namespace Fuko::Job
{
	using NormalExec = void(*)(void*);
	using ConditionExec = uint32_t(*)(void*);
	using BranchExec = bool(*)(void*);

	struct alignas(16) Executable
	{
		using MemoryOpFun = void(*)(void*);
		void*				Storage;
		void*				ExecuteFunc;
		MemoryOpFun			DestroyFunc;

		inline Executable();
		inline ~Executable() { Destroy(); }

		inline bool IsValid() const { return Storage && ExecuteFunc; }
		inline bool IsDestroyed() const { return DestroyFunc == nullptr; }
		inline void Destroy();
		
		// Invoke 
		inline void			InvokeNormal() { ((NormalExec)(ExecuteFunc))(Storage); }
		inline uint32_t		InvokeCondition() { return ((ConditionExec)(ExecuteFunc))(Storage); }
		inline bool			InvokeBranch() { return ((BranchExec)(ExecuteFunc))(Storage); }

		// Bind 
		template<typename TFun> inline void	BindNormal(TFun&& Fun);
		template<typename TFun> inline void BindCondition(TFun&& Fun);
		template<typename TFun> inline void BindBranch(TFun&& Fun);
	};
}

// Impl 
namespace Fuko::Job
{
	inline Executable::Executable()
		: Storage(nullptr)
		, ExecuteFunc(nullptr)
		, DestroyFunc(nullptr)
	{}

	inline void Executable::Destroy()
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

	template<typename TFun>
	inline void	Executable::BindNormal(TFun&& Fun)
	{
		using RealTFun = std::remove_reference_t<TFun>;

		Destroy();

		Storage = AllocExecutable(sizeof(RealTFun), alignof(RealTFun));
		new(Storage) RealTFun(std::forward<TFun>(Fun));

		NormalExec Exec;
		Exec = [](void* InPtr) { (*(RealTFun*)(InPtr))(); };

		ExecuteFunc = Exec;
		DestroyFunc = [](void* InPtr) { ((RealTFun*)InPtr)->~RealTFun(); };
	}
	
	template<typename TFun>
	inline void Executable::BindCondition(TFun&& Fun)
	{
		using RealTFun = std::remove_reference_t<TFun>;

		Destroy();

		Storage = AllocExecutable(sizeof(RealTFun), alignof(RealTFun));
		new(Storage) RealTFun(std::forward<TFun>(Fun));

		ConditionExec Exec;
		Exec = [](void* InPtr) { return (*(RealTFun*)(InPtr))(); };

		ExecuteFunc = Exec;
		DestroyFunc = [](void* InPtr) { ((RealTFun*)InPtr)->~RealTFun(); };
	}
	
	template<typename TFun>
	inline void Executable::BindBranch(TFun&& Fun)
	{
		using RealTFun = std::remove_reference_t<TFun>;

		Destroy();

		Storage = AllocExecutable(sizeof(RealTFun), alignof(RealTFun));
		new(Storage) RealTFun(std::forward<TFun>(Fun));

		BranchExec Exec;
		Exec = [](void* InPtr) { return (*(RealTFun*)(InPtr))(); };

		ExecuteFunc = Exec;
		DestroyFunc = [](void* InPtr) { ((RealTFun*)InPtr)->~RealTFun(); };
	}
}
