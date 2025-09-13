#pragma once

#include "Mesh/Actor.h"


class ASphereActor : public AActor
{
public:
	ASphereActor();
private:
	USphereComponent* SphereComponent = nullptr;
};

