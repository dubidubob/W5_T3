#pragma once
#include "Global/Types.h"


#ifdef _DEVELOP
#define TIME_PROFILE(Key)\
FScopeCycleCounter Key##Counter(#Key);
#else
#define TIME_PROFILE(Key)
#endif

#ifdef _DEVELOP
#define TIME_PROFILE_START(Key)\
{\
	FScopeCycleCounter Key##Counter(#Key);
#else
#define TIME_PROFILE_START(Key)
#endif


#ifdef _DEVELOP
#define TIME_PROFILE_END(Key)\
}
#else
#define TIME_PROFILE_END(Key)
#endif



class FWindowsPlatformTime
{
public:
	static double GSecondsPerCycle; // 0
	static bool bInitialized; // false

	static void InitTiming()
	{
		if (!bInitialized)
		{
			bInitialized = true;

			double Frequency = (double)GetFrequency();
			if (Frequency <= 0.0)
			{
				Frequency = 1.0;
			}

			GSecondsPerCycle = 1.0 / Frequency;
		}
	}
	static double GetSecondsPerCycle()
	{
		if (!bInitialized)
		{
			InitTiming();
		}
		return (double)GSecondsPerCycle;
	}
	static uint64 GetFrequency()
	{
		LARGE_INTEGER Frequency;
		QueryPerformanceFrequency(&Frequency);
		return Frequency.QuadPart;
	}
	static double ToMilliseconds(uint64 CycleDiff)
	{
		double Ms = static_cast<double>(CycleDiff)
			* GetSecondsPerCycle()
			* 1000.0;

		return Ms;
	}

	static uint64 Cycles64()
	{
		LARGE_INTEGER CycleCount;
		QueryPerformanceCounter(&CycleCount);
		return (uint64)CycleCount.QuadPart;
	}
};

struct TStatId
{
	FString Key;
	TStatId() = default;
	TStatId(const FString& InKey) : Key(InKey) {}
};
struct FTimeProfile
{
	double Milliseconds;
	uint64 Cycles;

	const char* GetConstChar() const
	{
		static char buffer[64]; // static으로 해야 반환 가능
		snprintf(buffer, sizeof(buffer), " : %.3fms, Cycle : %llu", Milliseconds, Cycles);
		return buffer;
	}
};

typedef FWindowsPlatformTime FPlatformTime;

class FScopeCycleCounter
{
public:
	FScopeCycleCounter(TStatId StatId)
		: StartCycles(FPlatformTime::Cycles64())
		, UsedStatId(StatId)
	{
	}
	FScopeCycleCounter() : StartCycles(FPlatformTime::Cycles64()), UsedStatId()
	{
	}

	FScopeCycleCounter(const FString& Key) : StartCycles(FPlatformTime::Cycles64()), UsedStatId(TStatId(Key))
	{
	}

	~FScopeCycleCounter()
	{
		Finish();
	}

	double Finish()
	{
		const uint64 EndCycles = FPlatformTime::Cycles64();
		const uint64 CycleDiff = EndCycles - StartCycles;

		double Milliseconds = FWindowsPlatformTime::ToMilliseconds(CycleDiff);
		if (UsedStatId.Key.empty() == false)
		{
			AddTimeProfile(UsedStatId, Milliseconds, CycleDiff);
		}
		return Milliseconds;
	}

	static void AddTimeProfile(const TStatId& Key, double InMilliseconds, uint64 InCycles);
	static void TimeProfileInit();

	//이거 왜 안됨?
	//static const TMap<FString, FTimeProfile>& GetTimeProfiles();

	static const TArray<FString> GetTimeProfileKeys();
	static const TArray<FTimeProfile> GetTimeProfileValues();
	static const FTimeProfile& GetTimeProfile(const FString& Key);
private:
	uint64 StartCycles;
	TStatId UsedStatId;

};
