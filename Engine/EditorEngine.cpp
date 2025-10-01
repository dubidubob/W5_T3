#include "pch.h"
#include "EditorEngine.h"
#include "Mesh/Actor.h"

IMPLEMENT_CLASS(UEditorEngine, UObject)

UEditorEngine::UEditorEngine()
{
	EditorWorldContext.OnWorld = NewObject<UWorld>();
	EditorWorldContext.OnWorld->SetWorldType(EWorldType::Editor);
}

UEditorEngine::~UEditorEngine()
{
	for (auto& WContext : WorldContexts)
	{
		delete WContext.OnWorld;
	}
}

void UEditorEngine::Tick(float DeltaSeconds)
{
	// Editor 전용 액터 Tick 처리
	for (FWorldContext& WorldContext : WorldContexts)
	{
		UWorld* EditorWorld = WorldContext.World();
		if (EditorWorld && EditorWorld->GetWorldType() == EWorldType::Editor)
		{
			ULevel* Level = EditorWorld->GetLevel();
			{
				for (AActor* Actor : Level->GetLevelActors())
				{
					if (Actor && Actor->bTickInEditor)
					{
						Actor->Tick(DeltaSeconds);
					}
				}
			}
		}
		else if (EditorWorld && EditorWorld->GetWorldType() == EWorldType::PIE)
		{
			ULevel* Level = EditorWorld->GetLevel();
			{
				for (AActor* Actor : Level->GetLevelActors())
				{
					if (Actor)
					{
						Actor->Tick(DeltaSeconds);
					}
				}
			}
		}
	}
}
