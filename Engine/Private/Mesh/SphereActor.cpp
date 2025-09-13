#include "pch.h"
#include "Mesh/SphereActor.h"

IMPLEMENT_CLASS(ASphereActor, AActor)
ASphereActor::ASphereActor()
{
	SphereComponent = CreateDefaultSubobject<USphereComponent>("SphereComponent");
	SphereComponent->SetOwner(this);
	SetRootComponent(SphereComponent);
}
