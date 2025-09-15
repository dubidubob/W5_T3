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

	ID3D11Buffer* GetIndexBuffer() const { return IndexBuffer; }
	uint32 GetNumIndices() const { return NumIndices; }

private:
	void GenerateWireframeVertices();
	FAABB BoundingBox;
	TArray<FVertex> WireframeVertices;
	TArray<uint32> WireframeIndices;
	ID3D11Buffer* IndexBuffer = nullptr;
	uint32 NumIndices = 0;
	bool bNeedsUpdate = true;
};
