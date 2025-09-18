#pragma once
#include "Mesh/Actor.h"
class UTextComponent;

class ASquareActor : public AActor
{
	DECLARE_CLASS(ASquareActor, AActor)

public:
	ASquareActor();
	virtual ~ASquareActor() override {}

private:
	USquareComponent* SquareComponent = nullptr;
	UTextComponent* TextComponent = nullptr;
};
