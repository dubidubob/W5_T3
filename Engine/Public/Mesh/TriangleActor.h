#pragma once

#include "Mesh/Actor.h"
class UTextComponent;
class ATriangleActor : public AActor
{
	using Super = AActor;
public:
	ATriangleActor();
	virtual ~ATriangleActor() override {}


private:
	UTriangleComponent* TriangleComponent = nullptr;
	UTextComponent* TextComponent = nullptr;
};
