#pragma once

class FNameTable
{
	DECLARE_SINGLETON(FNameTable);

public:
	TPair<int32, int32> FindOrAddName(const FString& Str);
	const FString& GetDisplayString(int32 Idx) const;

private:
	TArray<FString> ComparisonStringPool;
	TArray<FString> DisplayStringPool;

	TMap<FString, int32> ComparisonMap;
	TMap<FString, int32> DisplayMap;

	FString ToLower(const FString& Str) const;
};
