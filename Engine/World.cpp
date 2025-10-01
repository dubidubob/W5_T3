#include "pch.h"
#include "World.h"
#include "Public/Mesh/Actor.h"
#include "Manager/Level/LevelManager.h"

IMPLEMENT_CLASS(UWorld, UObject)

UWorld::UWorld()
{
	Level = ULevelManager::GetInstance().GetCurrentLevel();
}

UWorld::~UWorld()
{
}

void UWorld::Tick(float DeltaTime)
{
	//ULevelManager::GetInstance().Update(DeltaTime);
	Level->Update(DeltaTime);
}


UWorld* UWorld::DuplicateWorldForPIE(UWorld* InWorld)
{
	UWorld* PIEWorld = NewObject<UWorld>();

	PIEWorld->SetWorldType(EWorldType::PIE);

	ULevel* PIELevel = NewObject<ULevel>();

	PIELevel = InWorld->GetLevel()->Duplicate();

	PIEWorld->SetLevel(PIELevel);

	return PIEWorld;
}

void UWorld::InitializeActorsForPlay()
{
}

void UWorld::CleanupWorld()
{
	Level->Cleanup();
	delete Level;
}
