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
	ULevelManager::GetInstance().Update(DeltaTime);
	//Level->Update(DeltaTime);
}
