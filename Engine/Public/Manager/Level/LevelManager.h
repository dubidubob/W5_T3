#pragma once
#include "Core/Object.h"

class ULevel;
class UCamera;
struct FLevelMetadata;

class ULevelManager : public UObject
{
	DECLARE_CLASS(ULevelManager, UObject)
	DECLARE_SINGLETON(ULevelManager)

public:
	void Update(float DeltaTime) const;
	void Shutdown();

	// Getter
	ULevel* GetCurrentLevel() const { return CurrentLevel; }
	void SetCurrentLevel(ULevel* InLevel) { CurrentLevel = InLevel; }

	// Level Operations
	bool Init(UCamera* InCamera);
	bool CreateNewLevel();
	bool LoadLevel(const FString& InFilePath);
	bool SaveCurrentLevel(const FString& InFilePath) const;

	// Utility
	static path GetLevelDirectory();
	static path GenerateLevelFilePath(const FString& InLevelName);

private:
	// Metadata Conversion Functions
	static FLevelMetadata ConvertLevelToMetadata(ULevel* InLevel);
	static bool LoadLevelFromMetadata(ULevel* InLevel, const FLevelMetadata& InMetadata);

private:
	ULevel* CurrentLevel = nullptr;
};
