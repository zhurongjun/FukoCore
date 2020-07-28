#pragma once
#include <CoreConfig.h>
#include <CoreType.h>

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

	struct ILogDevice
	{
		virtual ~ILogDevice() {}

		// Add log message
		virtual void	Log(ELogType InType, const TCHAR* InFmt, ...) = 0;
		virtual void	LogSubMsg(const TCHAR* InFmt, ...) = 0;
		
		// Clear log device 
		virtual bool	ClearDevice() = 0;

		// Flush
		virtual void	Flush() = 0;
		virtual float	AutoFlushRate() = 0;
		virtual void	SetAutoFlushRate(float InRate) = 0;
	};






}