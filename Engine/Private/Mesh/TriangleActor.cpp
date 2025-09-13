#include "pch.h"
#include "Mesh/TriangleActor.h"

IMPLEMENT_CLASS(ATriangleActor, AActor)

ATriangleActor::ATriangleActor()
{
	TriangleComponent = CreateDefaultSubobject<UTriangleComponent>("TriangleComponent");
	TriangleComponent->SetRelativeRotation({ 90, 0, 0 });
	TriangleComponent->SetOwner(this);
	SetRootComponent(TriangleComponent);
}

