#pragma once

class FNameTable
{
	DECLARE_SINGLETON(FNameTable);

public:
	TPair<int32, int32> FindOrAddName(const FString& Str);
	FName GetUniqueName(const FString& BaseStr);

	FString GetDisplayString(int32 Idx) const;

private:
	FString ToLower(const FString& Str) const;

	TArray<FString> ComparisonStringPool;
	TArray<FString> DisplayStringPool;

	TMap<FString, int32> ComparisonMap;
	TMap<FString, int32> DisplayMap;
	TMap<FString, int32> NextNumberMap;
};
