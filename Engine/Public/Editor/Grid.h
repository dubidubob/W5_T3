#pragma once
#include "Core/Object.h"
#include "Global/CoreTypes.h"
#include "Editor/EditorPrimitive.h"

class UGrid : public UObject
{
	DECLARE_CLASS(UGrid, UObject)

public:
	UGrid();
	~UGrid() override;
	void SetLineVertices();
	void SetGridProperty(float InCellSize, int InNumLines);

	void RenderGrid();

private:
	float CellSize = 1.0f;
	int NumLines = 250;
	FEditorPrimitive Primitive;
	TArray<FVertex> LineVertices;
};
