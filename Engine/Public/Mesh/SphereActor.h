#pragma once

#include "Mesh/Actor.h"

class UTextComponent;
class ASphereActor : public AActor
{
public:
	ASphereActor();
private:
	USphereComponent* SphereComponent = nullptr;
	UTextComponent* TextComponent = nullptr;
};

