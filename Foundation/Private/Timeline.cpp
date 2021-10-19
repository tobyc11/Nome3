#include "Timeline.h"

#if TC_OS == TC_OS_WINDOWS_NT
#include <Windows.h>
#endif

#if TC_OS == TC_OS_MAC_OS_X
#include <mach/mach.h>
#include <mach/mach_time.h>

static mach_timebase_info_data_t TimebaseInfo;

#endif

namespace tc
{

#if TC_OS == TC_OS_WINDOWS_NT
void FHighResolutionClock::Init()
{
    LARGE_INTEGER li;

    bPause = false;

    QueryPerformanceFrequency(&li);
    Frequency = (double)li.QuadPart;

    QueryPerformanceCounter(&li);
    CountStart = li.QuadPart;
}

uint64_t FHighResolutionClock::Now() const
{
    if (bPause)
        return CountWhenPaused;

    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return li.QuadPart - CountStart;
}
#endif

#if TC_OS == TC_OS_MAC_OS_X
void FHighResolutionClock::Init()
{
    bPause = false;

    mach_timebase_info(&TimebaseInfo);
    Frequency = (double)TimebaseInfo.denom / (double)TimebaseInfo.numer * 1000000000.0;

    CountStart = mach_absolute_time();
}

uint64_t FHighResolutionClock::Now() const
{
    if (bPause)
        return CountWhenPaused;

    uint64_t absoluteTimeNow = mach_absolute_time();
    return absoluteTimeNow - CountStart;
}
#endif

uint32_t FHighResolutionClock::NowMilliSec() const
{
    return static_cast<uint32_t>(Now() * 1000 / Frequency);
}

double FHighResolutionClock::GetFrequency() const
{
    return Frequency;
}

uint32_t FHighResolutionClock::ConvertToMilliSec(uint64_t time) const
{
    return static_cast<uint32_t>(time * 1000 / Frequency);
}

void FHighResolutionClock::Pause()
{
    CountWhenPaused = Now();
    bPause = true;
}

void FHighResolutionClock::UnPause()
{
    CountStart += Now() - CountWhenPaused;
    bPause = false;
}

bool FHighResolutionClock::IsPaused() const
{
    return bPause;
}

}
