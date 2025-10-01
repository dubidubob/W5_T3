#include "pch.h"
#include "Mesh/ActorComponent.h"

IMPLEMENT_CLASS(UActorComponent, UObject)

UActorComponent::UActorComponent()
{
	ComponentType = EComponentType::Actor;
}

UActorComponent::~UActorComponent()
{
	SetOuter(nullptr);
}

void UActorComponent::BeginPlay()
{

}

void UActorComponent::TickComponent(float DeltaTime)
{

}

void UActorComponent::EndPlay()
{

}

void UActorComponent::DuplicateSubObjects()
{

}

UActorComponent* UActorComponent::Duplicate()
{
	UActorComponent* NewComp = new UActorComponent(*this);

	NewComp->DuplicateSubObjects();

	return NewComp;
}
