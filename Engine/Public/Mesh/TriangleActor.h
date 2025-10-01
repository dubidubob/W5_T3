#pragma once

#include "Mesh/Actor.h"
class UTextRenderComponent;
class ATriangleActor : public AActor
{
	DECLARE_CLASS(ATriangleActor, AActor)

public:
	ATriangleActor();
	virtual ~ATriangleActor() override {}


private:
	UTriangleComponent* TriangleComponent = nullptr;
	UTextRenderComponent* TextComponent = nullptr;
};
