#pragma once
#include "FoundationAPI.h"
#include "Platform.h"
#include <cstdint>

namespace tc
{

class FOUNDATION_API ITimer
{
public:
    virtual ~ITimer() = default;
    virtual void Init() = 0;
    virtual void Tick() = 0;
    virtual void SwitchPause() = 0;
    virtual bool IsPaused() const = 0;
    virtual float GetTotalTime() const = 0;
    virtual float GetDeltaTime() const = 0;
};

class FOUNDATION_API FPerfTimer : public ITimer
{
public:
	FPerfTimer();
	void Init() override;
	void Tick() override;
	void SwitchPause() override;
	bool IsPaused() const override;
	float GetTotalTime() const override;
    float GetDeltaTime() const override;

private:
	bool bPaused;
	bool bJustUnpaused;

	double DeltaTime;
	std::uint64_t CurrCount;
	std::uint64_t PrevCount;
	std::uint64_t CountDuringGame;
};

template <typename T>
class FTimer : public ITimer
{
    const T& ParentTimer;
    bool bPaused;
    bool bJustUnpaused;
    float DeltaTime;
    float CurrTime;
    float PrevTime;
    float TotalTime;
public:
    FTimer(const T& parent) : ParentTimer(parent)
    {
    }

    void Init() override
    {
        bPaused = false;
        bJustUnpaused = false;
        DeltaTime = CurrTime = PrevTime = TotalTime = 0.0f;
    }

    void Tick() override
    {
        if (bPaused)
        {
            DeltaTime = 0.0f;
        }
        else if(bJustUnpaused)
        {
            bJustUnpaused = false;
            CurrTime = ParentTimer.GetTotalTime();
            PrevTime = CurrTime;
            DeltaTime = 0.0f;
        }
        else
        {
            PrevTime = CurrTime;
            CurrTime = ParentTimer.GetTotalTime();
            DeltaTime = CurrTime - PrevTime;
            if (DeltaTime < 0.0f)
                DeltaTime = 0.0f;
            TotalTime += DeltaTime;
        }
    }

    void SwitchPause() override
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

    bool IsPaused() const override
    {
        return bPaused;
    }

    float GetTotalTime() const override
    {
        return TotalTime;
    }

    float GetDeltaTime() const override
    {
        return DeltaTime;
    }
};

}
