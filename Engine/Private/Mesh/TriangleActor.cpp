#include "pch.h"
#include "Mesh/TriangleActor.h"
#include "Mesh/TextComponent.h"

IMPLEMENT_CLASS(ATriangleActor, AActor)

ATriangleActor::ATriangleActor()
{
	TriangleComponent = CreateDefaultSubobject<UTriangleComponent>("TriangleComponent");
	TriangleComponent->SetRelativeRotation({ 90, 0, 0 });
	TriangleComponent->SetOwner(this);
	TextComponent = CreateDefaultSubobject<UTextComponent>("TextComponent");
	TextComponent->SetOwner(this);
	TextComponent->SetParentAttachment(TriangleComponent);
	SetRootComponent(TriangleComponent);
}

