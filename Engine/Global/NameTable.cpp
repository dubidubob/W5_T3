#include "pch.h"
#include "Global/NameTable.h"

IMPLEMENT_SINGLETON(FNameTable);

FNameTable::FNameTable()
{
	FindOrAddName("");
}
FNameTable::~FNameTable() = default;

/**
* @brief FString 객체를 받아 풀에 없으면 반환
* @param Str FName으로 등록되었는지 확인할 FString
* @return DisplayIndex, ComparisonIndex
*/
TPair<int32, int32> FNameTable::FindOrAddName(const FString& Str)
{
	FString LowerStr = ToLower(Str);

	int32 ComparisonIndex;
	auto ItComparison = ComparisonMap.find(LowerStr);
	if (ItComparison != ComparisonMap.end())
	{
		ComparisonIndex = ItComparison->second;
	}
	else
	{
		ComparisonIndex = ComparisonStringPool.size();
		ComparisonStringPool.push_back(LowerStr);
		ComparisonMap[LowerStr] = ComparisonIndex;
	}

	int32 DisplayIndex;
	auto ItDisplay = DisplayMap.find(Str);
	if (ItDisplay != DisplayMap.end())
	{
		DisplayIndex = ItDisplay->second;
	}
	else
	{
		DisplayIndex = DisplayStringPool.size();
		DisplayStringPool.push_back(Str);
		DisplayMap[Str] = DisplayIndex;
	}

	return { ComparisonIndex, DisplayIndex };
}

FName FNameTable::GetUniqueName(const FString& BaseStr)
{
	TPair<int32, int32> Indices = FindOrAddName(BaseStr);
	int32 DisplayIndex = Indices.second;
	int32 ComparisonIndex = Indices.first;

	int32 Number = NextNumberMap[BaseStr];
	NextNumberMap[BaseStr]++;

	return FName(DisplayIndex, ComparisonIndex, Number);
}

FString FNameTable::GetDisplayString(int32 Idx) const
{
	if (Idx >= 0 && Idx < DisplayStringPool.size())
	{
		return DisplayStringPool[Idx];
	}
	static const FString EmptyString = "";
	return EmptyString;
}

FString FNameTable::ToLower(const FString& Str) const
{
	FString LowerStr = Str;
	std::transform(LowerStr.begin(), LowerStr.end(), LowerStr.begin(),
		[](unsigned char C) { return std::tolower(C); });
	return LowerStr;
}
