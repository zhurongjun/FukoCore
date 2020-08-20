#pragma once
#include <CoreConfig.h>
#include <CoreType.h>
#include <chrono>
#include <String/String.h>
#include <String/Name.h>
#include <Containers/Array.h>
#include <Containers/Map.h>
#include "SmartPtr.h"

namespace Fuko
{
	enum ELogType
	{
		Debug ,	
		Message ,
		Warning ,
		Error ,
		Fatal ,
	};

	struct LogItem
	{
		std::chrono::system_clock::time_point		LogTime;
		ELogType		LogLevel;
		Name			LogCategory;
		String			LogStr;
	};

	struct ILogDevice
	{
		virtual ~ILogDevice() {}

		// Add log message
		virtual void	Log(const LogItem& InLog) = 0;

		// Clear log device 
		virtual bool	ClearDevice() = 0;

		// Flush
		virtual void	Flush() = 0;
		virtual float	AutoFlushRate() = 0;
		virtual void	SetAutoFlushRate(float InRate) = 0;
	};

	class LogSystem
	{
		TArray<SP<ILogDevice>>	m_Devices;
		TArray<LogItem>			m_LogItems;
	public:

		LogSystem()
			: m_Devices(8)
			, m_LogItems(4096)
		{}

		void Log(ELogType InType, Name InLogCategory, const String& InLogStr)
		{
			LogItem	Item;
			Item.LogCategory = InLogCategory;
			Item.LogLevel = InType;
			Item.LogTime = std::chrono::system_clock::now();
			Item.LogStr = InLogStr;

			for (uint32 i = 0; i < m_Devices.Num(); ++i)
			{
				m_Devices[i]->Log(Item);
			}
			m_LogItems.Push(Item);
		}

		void AddDevice(SP<ILogDevice> InDevice, bool LogAll = true)
		{
			m_Devices.Add(InDevice);
			if (LogAll)
			{
				for (uint32 i = 0; i < m_LogItems.Num(); ++i)
				{
					InDevice->Log(m_LogItems[i]);
				}
			}
		}

		bool RemoveDevice(SP<ILogDevice> InDevice)
		{
			int32 Num = m_Devices.RemoveSwap(InDevice);
			if (Num > 0)
			{
				InDevice->Flush();
				return true;
			}
			return false;
		}

		void FlushAll()
		{
			for (uint32 i = 0; i < m_Devices.Num(); ++i)
			{
				m_Devices[i]->Flush();
			}
		}
	};
}

namespace Fuko
{
	CORE_API LogSystem& GlobalLogDevice();
}

#define F_LOG(Level,Category,Fmt,...) ::Fuko::GlobalLogDevice().Log(Level,Category,FmtString(Fmt,...));

