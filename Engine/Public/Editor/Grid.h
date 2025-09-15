#pragma once
#include "Core/Object.h"
#include "Global/CoreTypes.h"
#include "Editor/EditorPrimitive.h"

class ULineBatchRenderer;

class UGrid : public UObject
{
	DECLARE_CLASS(UGrid, UObject)

public:
	UGrid();
	~UGrid() override;
	void SetLineVertices();
	void SetGridProperty(float InCellSize, int InNumLines);

	/** 기존 개별 렌더링 */
	void RenderGrid();

	/** 배칭 렌더링 */
	void AddToLineBatch(ULineBatchRenderer& LineBatch);

private:
	float CellSize = 1.0f;
	int NumLines = 250;
	FEditorPrimitive Primitive;
	TArray<FVertex> LineVertices;
};
