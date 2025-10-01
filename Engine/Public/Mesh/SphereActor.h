#pragma once

#include "Mesh/Actor.h"

class UTextRenderComponent;
class ASphereActor : public AActor
{
	DECLARE_CLASS(ASphereActor, AActor)
public:
	ASphereActor();
private:
	USphereComponent* SphereComponent = nullptr;
	UTextRenderComponent* TextComponent = nullptr;
};

