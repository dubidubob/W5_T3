#include "pch.h"
#include "Mesh/CubeActor.h"
#include "Mesh/TextComponent.h"
IMPLEMENT_CLASS(ACubeActor, AActor)

ACubeActor::ACubeActor()
{
	CubeComponent = CreateDefaultSubobject<UCubeComponent>("CubeComponent");
	CubeComponent->SetOwner(this);
	TextComponent = CreateDefaultSubobject<UTextRenderComponent>("TextComponent");
	TextComponent->SetOwner(this);
	TextComponent->SetParentAttachment(CubeComponent);
	SetRootComponent(CubeComponent);
}
