#pragma once
#include <windows.h>
#include <profileapi.h>

class Timer
{
public:
	float	PerCountTime;
	int64	Count;
	
	Timer()
	{
		int64 Temp;
		QueryPerformanceFrequency((LARGE_INTEGER*)&Temp);
		PerCountTime = 1.0f / Temp;
		QueryPerformanceCounter((LARGE_INTEGER*)&Count);
	}

	float Tick()
	{
		int64 LastCount = Count;
		QueryPerformanceCounter((LARGE_INTEGER*)&Count);
		return PerCountTime * (Count - LastCount);
	}

};
