#pragma once

#include "Mesh/Actor.h"

class UTextComponent;
class ASphereActor : public AActor
{
	DECLARE_CLASS(ASphereActor, AActor)
public:
	ASphereActor();
private:
	USphereComponent* SphereComponent = nullptr;
	UTextComponent* TextComponent = nullptr;
};

