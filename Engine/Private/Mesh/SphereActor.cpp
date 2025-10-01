#include "pch.h"
#include "Mesh/SphereActor.h"
#include "Mesh/TextComponent.h"

IMPLEMENT_CLASS(ASphereActor, AActor)
ASphereActor::ASphereActor()
{
	SphereComponent = CreateDefaultSubobject<USphereComponent>("SphereComponent");
	SphereComponent->SetOwner(this);
	TextComponent = CreateDefaultSubobject<UTextRenderComponent>("TextComponent");
	TextComponent->SetOwner(this);
	TextComponent->SetParentAttachment(SphereComponent);
	SetRootComponent(SphereComponent);
}
