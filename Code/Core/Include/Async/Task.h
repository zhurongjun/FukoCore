#pragma once
#include <CoreType.h>
#include <CoreConfig.h>

namespace Fuko
{
	struct ITask
	{
		virtual ~ITask() {}

		virtual void DoWork() = 0;
		virtual void GiveUp() = 0;
	}
}