#include "pch.h"
#include "Mesh/CubeActor.h"

ACubeActor::ACubeActor()
{
	CubeComponent = CreateDefaultSubobject<UCubeComponent>("CubeComponent");
	CubeComponent->SetOwner(this);
	SetRootComponent(CubeComponent);
}
