#include <Misc/Log.h>

namespace Fuko
{
	CORE_API LogSystem& GlobalLogDevice()
	{
		static LogSystem System;
		return System;
	}

}