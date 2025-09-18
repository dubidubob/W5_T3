#pragma once

#include "Mesh/Actor.h"
class UTextComponent;

class ASquareActor : public AActor
{
	using Super = AActor;
public:
	ASquareActor();
	virtual ~ASquareActor() override {}

private:
	USquareComponent* SquareComponent = nullptr;
	UTextComponent* TextComponent = nullptr;
};
