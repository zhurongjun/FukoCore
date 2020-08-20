#pragma once
#include <CoreConfig.h>
#include <CoreType.h>
#include "FileDevice.h"
#include <Misc/SmartPtr.h>

namespace Fuko
{
	class FileSystem
	{
		TMap<Name, TArray<SP<IFileDevice>>>			m_DeviceMap;
		SP<IFileDevice>		m_FallBackDevice;



	};
}

