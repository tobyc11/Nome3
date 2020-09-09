#include "Clock.h"
#if TC_OS == TC_OS_WINDOWS_NT
#include <Windows.h>
static double SecondsPerCount;
namespace tc
{
FPerfTimer::FPerfTimer()
{
}

void FPerfTimer::Init()
{
	int64_t CounterFreq;
	QueryPerformanceFrequency((LARGE_INTEGER*)&CounterFreq);
	SecondsPerCount = 1.0 / (double)CounterFreq;

	int64_t Count;
	QueryPerformanceCounter((LARGE_INTEGER*)&Count);
	CurrCount = Count;
	PrevCount = Count;
	CountDuringGame = 0;

	bPaused = false;
	bJustUnpaused = false;

	DeltaTime = 0.0;
}

void FPerfTimer::Tick()
{
	if (bPaused)
	{
		DeltaTime = 0.0;
	}
	else if(bJustUnpaused)
	{
		bJustUnpaused = false;
		QueryPerformanceCounter((LARGE_INTEGER*)&CurrCount);
		PrevCount = CurrCount;
		DeltaTime = 0.0;
	}
	else
	{
		PrevCount = CurrCount;
		QueryPerformanceCounter((LARGE_INTEGER*)&CurrCount);
		DeltaTime = (CurrCount - PrevCount) * SecondsPerCount;
		if (DeltaTime < 0.0)
			DeltaTime = 0.0;
		CountDuringGame += CurrCount - PrevCount;
	}
}

void FPerfTimer::SwitchPause()
{
	if(bPaused)
	{
		bPaused = false;
		bJustUnpaused = true;
	}
	else
	{
		bPaused = true;
	}
}

bool FPerfTimer::IsPaused() const
{
	return bPaused;
}

float FPerfTimer::GetTotalTime() const
{
	return static_cast<float>(CountDuringGame * SecondsPerCount);
}

float FPerfTimer::GetDeltaTime() const
{
	return static_cast<float>(DeltaTime);
}
}

#elif TC_OS == TC_OS_MAC_OS_X

#include <mach/mach.h>
#include <mach/mach_time.h>

static mach_timebase_info_data_t sTimebaseInfo;

namespace tc
{

FPerfTimer::FPerfTimer()
{
}

void FPerfTimer::Init()
{
    if(sTimebaseInfo.denom == 0)
    mach_timebase_info(&sTimebaseInfo);

    uint64_t Count = mach_absolute_time();
    CurrCount = Count;
    PrevCount = Count;
    CountDuringGame = 0;

    bPaused = false;
    bJustUnpaused = false;

    DeltaTime = 0.0;
}

void FPerfTimer::Tick()
{
    if (bPaused)
    {
        DeltaTime = 0.0;
    }
    else if(bJustUnpaused)
    {
        bJustUnpaused = false;
        CurrCount = mach_absolute_time();
        PrevCount = CurrCount;
        DeltaTime = 0.0;
    }
    else
    {
        PrevCount = CurrCount;
        CurrCount = mach_absolute_time();
        DeltaTime = (double)(CurrCount - PrevCount) * sTimebaseInfo.numer / sTimebaseInfo.denom / 1000000000.0;
        if (DeltaTime < 0.0)
            DeltaTime = 0.0;
        CountDuringGame += CurrCount - PrevCount;
    }
}

void FPerfTimer::SwitchPause()
{
    if(bPaused)
    {
        bPaused = false;
        bJustUnpaused = true;
    }
    else
    {
        bPaused = true;
    }
}

bool FPerfTimer::IsPaused() const
{
    return bPaused;
}

float FPerfTimer::GetTotalTime() const
{
    return static_cast<float>((double)CountDuringGame * sTimebaseInfo.numer / sTimebaseInfo.denom / 1000000000.0);
}

float FPerfTimer::GetDeltaTime() const
{
    return static_cast<float>(DeltaTime);
}

}

#endif
