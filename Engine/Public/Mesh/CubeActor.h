#pragma once

#include "Mesh/Actor.h"


class ACubeActor : public AActor
{
	DECLARE_CLASS(ACubeActor, AActor)
public:
	ACubeActor();

private:
	UCubeComponent* CubeComponent = nullptr;
};
