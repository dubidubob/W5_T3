#include "pch.h"
#include "World.h"
#include "Public/Mesh/Actor.h"

IMPLEMENT_CLASS(UWorld, UObject)

void UWorld::Tick(float DeltaTime)
{
	Level->Update(DeltaTime);
}
