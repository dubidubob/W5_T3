#pragma once

class FName
{
public:
	FName(const FString& Str);
	FName(const char* Str);

	bool operator==(const FName& Other) const;
	int32 Compare(const FName& Other) const;

	FString ToString() const;

	int32 GetComparisonIndex() const { return ComparisonIndex; }
	int32 GetDisplayIndex() const { return DisplayIndex; }

private:
	int32 ComparisonIndex;
	int32 DisplayIndex;
};
