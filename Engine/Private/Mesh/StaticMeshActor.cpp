//Default
#include "pch.h"
#include "Mesh/StaticMeshActor.h"

//Add Component;
#include "Mesh/TextComponent.h"
#include "Mesh/StaticMeshComponent.h"

IMPLEMENT_CLASS(AStaticMeshActor, AActor)


AStaticMeshActor::AStaticMeshActor()
{
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("StaticMeshComponent");
	StaticMeshComponent->SetOwner(this);

	TextComponent = CreateDefaultSubobject<UTextRenderComponent>("TextComponent");
	TextComponent->SetOwner(this);
	TextComponent->SetParentAttachment(StaticMeshComponent);
	SetRootComponent(StaticMeshComponent);
}
