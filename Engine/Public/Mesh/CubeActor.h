#pragma once

#include "Mesh/Actor.h"

class UTextComponent;
class ACubeActor : public AActor
{
public:
	ACubeActor();

private:
	UCubeComponent* CubeComponent = nullptr;
	UTextComponent* TextComponent = nullptr;
};
