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

private:
	ULevel* Level;
	EWorldType WorldType;

	void Tick(float DeltaTime);

};

struct FWorldContext
{
	EWorldType WorldType;
	UWorld* OnWorld;
	FName ContextHandle;

	UWorld* World() { return OnWorld; }
};
