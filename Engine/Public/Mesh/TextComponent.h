#pragma once
#include "Mesh/SceneComponent.h"

class UTextRenderComponent : public UPrimitiveComponent
{
	DECLARE_CLASS(UTextRenderComponent, UPrimitiveComponent)
public:
	UTextRenderComponent();
	~UTextRenderComponent();

	void SetInstanceData(const FWstring& Characters);
	TArray<FTextInstance>* GetInstanceData() { return &InstanceData; }

	void SetText(const FWstring& InText);
	FWstring GetText() const { return Text; }

	virtual void DuplicateSubObjects() override;
	virtual UTextRenderComponent* Duplicate() override;

private:
	FWstring Text;
	TArray<FTextInstance> InstanceData;
};

