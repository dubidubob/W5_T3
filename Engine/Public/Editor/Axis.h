#pragma once
#include "Core/Object.h"
#include "Editor/EditorPrimitive.h"
#include "Global/CoreTypes.h"

class UAxis : public UObject
{
	DECLARE_CLASS(UAxis, UObject)

public:
	UAxis();
	~UAxis() override;
	void Render();

private:
	FEditorPrimitive Primitive;
	TArray<FVertex> AxisVertices;
};
