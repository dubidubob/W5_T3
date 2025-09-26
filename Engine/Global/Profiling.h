#pragma once
#include "Global/Types.h"

#define TIME_PROFILING_START(Key)\
FString TimeProfiling_##Key = #Key;


#define TIME_PROFILING_END(Key)

struct FTimeProfile
{
	uint32 Tick;
	uint32 CallCount;
};
class UProfiling
{
	static TMap<FString, FTimeProfile> TimeProfileMap;
};
