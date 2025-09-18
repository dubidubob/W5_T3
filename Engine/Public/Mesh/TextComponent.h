#pragma once
#include "Mesh/SceneComponent.h"

class UTextComponent : public UPrimitiveComponent
{
public:
	UTextComponent();
	~UTextComponent();

	void SetInstanceData(const FWstring& Characters);
	TArray<FTextInstance>* GetInstanceData() { return &InstanceData; }

	void SetText(const FWstring& InText);
	FWstring GetText() const { return Text; }

private:
	FWstring Text;
	TArray<FTextInstance> InstanceData;
};

