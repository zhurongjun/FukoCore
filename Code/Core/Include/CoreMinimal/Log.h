#pragma once
#include <CoreConfig.h>
#include <CoreType.h>
#include <chrono>

namespace Fuko::Log
{
	enum ELogType
	{
		Debug ,	
		Info ,
		Warning ,
		Error ,
		Fatal ,
	};

	struct LogItem
	{
		std::chrono::time_point		LogTime;
		ELogType					LogLevel;
		Name						LogCategory;
		String						LogStr;
	};

	struct ILogDevice
	{
		virtual ~ILogDevice() {}

		// Add log message
		virtual void	Log(ELogType InType, const LogItem& InLog) = 0;

		// Clear log device 
		virtual bool	ClearDevice() = 0;

		// Flush
		virtual void	Flush() = 0;
		virtual float	AutoFlushRate() = 0;
		virtual void	SetAutoFlushRate(float InRate) = 0;
	};

	class LogSystem
	{
		TArray<ILogDevice*>		m_Devices;
		TArray<LogItem>			m_LogItem;
	public:

	};
}