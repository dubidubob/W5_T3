#pragma once

#include "Mesh/Actor.h"


class ACubeActor : public AActor
{
public:
	ACubeActor();

private:
	UCubeComponent* CubeComponent = nullptr;
};
