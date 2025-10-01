#pragma once
#include "Mesh/Actor.h"
class UTextRenderComponent;

class ASquareActor : public AActor
{
	DECLARE_CLASS(ASquareActor, AActor)

public:
	ASquareActor();
	virtual ~ASquareActor() override {}

private:
	USquareComponent* SquareComponent = nullptr;
	UTextRenderComponent* TextComponent = nullptr;
};
