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

struct FWorldContext
{
	EWorldType WorldType;
	UWorld* OnWorld;
	FName ContextHandle;

	UWorld* World() { return OnWorld; }
};

class UWorld : public UObject
{
	DECLARE_CLASS(UWorld, UObject)

public:

private:
	ULevel* Level;
	EWorldType WorldType;

	void Tick(float DeltaTime);

};

