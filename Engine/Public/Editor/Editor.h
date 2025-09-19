#pragma once
#include "Editor/Camera.h"
#include "Core/Object.h"

struct FQuat;
class URenderer;
class UObjectPicker;
class UViewportManager;
class UGizmo;
class UAxis;
class UGrid;
class ULevel;

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

	/** jft, ViewportManager Getter*/
	UViewportManager* GetViewportManager() { return ViewportManager; }

private:
	void ProcessKeyboardInput();
	void ProcessMouseInput(ULevel* InLevel);

	void HandleGizmo(ULevel* InLevel, FRay InWorldRay);
	TArray<class UPrimitiveComponent*> FindCandidatePrimitives(ULevel* InLevel);

	FVector GetGizmoDragLocation(const FRay& WorldRay);
	FVector GetGizmoDragRotation(const FRay& WorldRay);
	FQuat GetGizmoDragRotationQuat(const FRay& WorldRay);
	FVector GetGizmoDragScale(const FRay& WorldRay);

	UCamera* Camera;
	UObjectPicker* ObjectPicker;
	UViewportManager* ViewportManager;

	const float MinScale = 0.01f;
	UGizmo* Gizmo;
	UAxis* Axis;
	UGrid* Grid;
};
