#include "pch.h"
#include "PlatformTime.h"

TMap<FString, FTimeProfile> TimeProfileMap;

void FScopeCycleCounter::AddTimeProfile(const TStatId& Key, double InMilliseconds, uint64 InCycles)
{
	if (TimeProfileMap.Contains(Key.Key) == false)
	{
		TimeProfileMap[Key.Key] = FTimeProfile{ InMilliseconds, InCycles };
	}
	else
	{
		TimeProfileMap[Key.Key].Milliseconds += InMilliseconds;
		TimeProfileMap[Key.Key].Cycles += InCycles;
	}
}
void FScopeCycleCounter::TimeProfileInit()
{
	const TArray<FString> Keys = TimeProfileMap.GetKeys();
	for (const FString& Key : Keys)
	{
		TimeProfileMap[Key].Milliseconds = 0;
		TimeProfileMap[Key].Cycles = 0;
	}
}
//const TMap<FString, FTimeProfile>& FScopeCycleCounter::GetTimeProfiles()
//{
//	return TimeProfileMap;
//}
const TArray<FString> FScopeCycleCounter::GetTimeProfileKeys()
{
	return TimeProfileMap.GetKeys();
}
const TArray<FTimeProfile> FScopeCycleCounter::GetTimeProfileValues()
{
	return TimeProfileMap.GetValues();
}
const FTimeProfile& FScopeCycleCounter::GetTimeProfile(const FString& Key)
{
	return TimeProfileMap[Key];
}
double FWindowsPlatformTime::GSecondsPerCycle = 0.0;
bool FWindowsPlatformTime::bInitialized = false;
