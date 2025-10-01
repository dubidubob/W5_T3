#pragma once

#include "Core/Object.h"
#include "Level/Level.h"
#include "Core/Class.h"

enum EWorldType
{
	Editor,
	EditorPreview,
	PIE,
	Game,
};

class UWorld : public UObject
{
	DECLARE_CLASS(UWorld, UObject)

public:
	UWorld();
	~UWorld();

	const EWorldType GetWorldType() const { return WorldType; }
	ULevel* GetLevel() const { return Level; }

	void SetWorldType(EWorldType InWorldType) { WorldType = InWorldType; }
	void SetLevel(ULevel* InLevel) { Level = InLevel; }

	static UWorld* DuplicateWorldForPIE(UWorld* InWorld);

	void InitializeActorsForPlay();

	bool IsPIEWorld() { return WorldType == EWorldType::PIE; }
	void CleanupWorld();

private:
	ULevel* Level;
	EWorldType WorldType;

	void Tick(float DeltaTime);

};

struct FWorldContext
{
	//EWorldType WorldType;
	UWorld* OnWorld;
	FName ContextHandle;

	UWorld* World() { return OnWorld; }
};
