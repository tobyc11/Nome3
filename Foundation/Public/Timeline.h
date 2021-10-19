#pragma once
#include "FoundationAPI.h"
#include "Platform.h"
#include <cstdint>

namespace tc
{

class FOUNDATION_API FHighResolutionClock
{
    uint64_t CountStart;
    uint64_t CountWhenPaused;
    double Frequency;

    bool bPause;

public:
    void Init();

    uint64_t Now() const;
    uint32_t NowMilliSec() const;
    double GetFrequency() const;

    uint32_t ConvertToMilliSec(uint64_t time) const;

    void Pause();
    void UnPause();
    bool IsPaused() const;
};

}
