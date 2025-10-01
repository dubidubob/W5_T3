#pragma once
#include "Mesh/Actor.h"

class UStaticMeshComponent;
class UTextRenderComponent;

class AStaticMeshActor : public AActor
{
	DECLARE_CLASS(AStaticMeshActor, AActor)

public:
	AStaticMeshActor();
	UStaticMeshComponent* GetStaticMeshComponent() { return StaticMeshComponent; }
private:
	UStaticMeshComponent* StaticMeshComponent;
	UTextRenderComponent* TextComponent = nullptr;

};

