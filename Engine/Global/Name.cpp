#include "pch.h"
#include "Global/Name.h"
#include "Global/NameTable.h"

FName::FName(const FString& Str)
{
	TPair<int32, int32> Indices = FNameTable::GetInstance().FindOrAddName(Str);
	ComparisonIndex = Indices.first;
	DisplayIndex = Indices.second;
}

FName::FName(const char* Str) : FName(FString(Str)) { }

bool FName::operator==(const FName& Other) const
{
	return ComparisonIndex == Other.ComparisonIndex;
}

int32 FName::Compare(const FName& Other) const
{
	if (*this == Other) { return 0; }
	return this->Compare(Other);
}

FString FName::ToString() const
{
	return FNameTable::GetInstance().GetDisplayString(DisplayIndex);
}
