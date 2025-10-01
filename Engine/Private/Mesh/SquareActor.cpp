#include "pch.h"
#include "Mesh/SquareActor.h"
#include "Mesh/TextComponent.h"
IMPLEMENT_CLASS(ASquareActor, AActor)

ASquareActor::ASquareActor()
{
	SquareComponent = CreateDefaultSubobject<USquareComponent>("SquareComponent");
	SquareComponent->SetRelativeRotation({ 90, 0, 0 });
	SquareComponent->SetOwner(this);

	TextComponent = CreateDefaultSubobject<UTextRenderComponent>("TextComponent");
	TextComponent->SetOwner(this);
	TextComponent->SetParentAttachment(SquareComponent);
	SetRootComponent(SquareComponent);
}
