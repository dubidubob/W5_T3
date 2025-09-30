#pragma once

#include "Core/Object.h"
#include "World.h"

class UEditorEngine : public UObject
{
	DECLARE_CLASS(UEditorEngine, UObject)

public:
	UEditorEngine();
	~UEditorEngine();

	virtual void Tick(float DeltaSeconds);
	FWorldContext& GetEditorWorldContext() { return EditorWorldContext; }
	static UWorld* DuplicateWorldForPIE(UWorld* InWorld);

private:
	TArray<FWorldContext> WorldContexts;
	FWorldContext EditorWorldContext;
};
