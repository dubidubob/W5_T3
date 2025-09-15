#pragma once

#include "Mesh/SceneComponent.h"

class UAABBWireframeComponent : public UPrimitiveComponent
{
	DECLARE_CLASS(UAABBWireframeComponent, UPrimitiveComponent)

public:
	UAABBWireframeComponent();
	~UAABBWireframeComponent();

	void SetAABB(const FAABB& InAABB);
	virtual FAABB GetLocalBounds() const override;

private:
	void GenerateWireframeVertices();
	FAABB BoundingBox;
	TArray<FVertex> WireframeVertices;
	bool bNeedsUpdate = true;
};
