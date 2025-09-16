#pragma once
#include "Editor/Camera.h"
#include "Editor/Gizmo.h"
#include "Editor/Grid.h"
#include "Editor/Axis.h"
#include "Core/Object.h"
#include "Editor/ObjectPicker.h"

class ULineBatchRenderer;

class UEditor : public UObject
{
	DECLARE_CLASS(UEditor, UObject)

public:
	UEditor();
	~UEditor();

	void Update();

	const FVector& GetCameraLocation();

	/** 배칭 렌더링 버전 */
	// void RenderEditor();
	void RenderEditorBatched();

private:

	void ProcessMouseInput(ULevel* InLevel);
	void ProcessKeyboardInput();
	TArray<UPrimitiveComponent*> FindCandidatePrimitives(ULevel* InLevel);

	FVector GetGizmoDragLocation(FRay& WorldRay);
	FVector GetGizmoDragRotation(FRay& WorldRay);
	FVector GetGizmoDragScale(FRay& WorldRay);

	UCamera Camera;
	UObjectPicker ObjectPicker;

	const float MinScale = 0.01f;
	UGizmo Gizmo;
	UAxis Axis;
	UGrid Grid;
};
