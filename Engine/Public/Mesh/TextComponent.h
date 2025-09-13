#pragma once
#include "Mesh/SceneComponent.h"

class UTextComponent : public UPrimitiveComponent
{
public:
	UTextComponent();
	~UTextComponent();

	void SetInstanceData(const FString& Characters);
	TArray<FTextInstance>* GetInstanceData() { return &InstanceData; }


private:
	TArray<FTextInstance> InstanceData;
};

