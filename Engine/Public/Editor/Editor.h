#pragma once
#include "Editor/Camera.h"
#include "Core/Object.h"

struct FQuat;
class URenderer;
class UObjectPicker;
class UViewportManager;
class UObjectPreviewScene;
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
	UCamera* GetCamera();

	/** 배칭 렌더링 버전 */
	// void RenderEditor();
	void RenderEditorBatched(int Idx);

	/** ViewportManager Getter*/
	UViewportManager* GetViewportManager() { return ViewportManager; }
	UObjectPreviewScene* GetObjPreview() { return ObjPreview; }

	/** Performance Metrics Getters */
	double GetTotalPickTime() const { return TotalPickTime; }
	double GetLastPickTime() const { return LastPickTime; }
	uint32_t GetTotalPickCount() const { return TotalPickCount; }

	/** Performance Metrics Management */
	void ResetPickingStatistics() { TotalPickTime = 0.0; TotalPickCount = 0; LastPickTime = 0.0; }

	/** Picking Configuration Variables */
	bool bTrianglePicking = false;
	bool bColorPicking = true;
	bool bUUIDColorPicking = false;
	bool bIndexColorPicking = true;

private:
	void ProcessKeyboardInput();
	void ProcessMouseInput(ULevel* InLevel);

	void HandleGizmo(ULevel* InLevel, FRay InWorldRay);
	TArray<class UPrimitiveComponent*> FindCandidatePrimitives(ULevel* InLevel);
	TArray<class UPrimitiveComponent*> GetAllPrimitives(ULevel* InLevel);

	FVector GetGizmoDragLocation(const FRay& WorldRay);
	FVector GetGizmoDragRotation(const FRay& WorldRay);
	FQuat GetGizmoDragRotationQuat(const FRay& WorldRay);
	FVector GetGizmoDragScale(const FRay& WorldRay);

	UCamera* Camera;
	UObjectPicker* ObjectPicker;
	UViewportManager* ViewportManager;
	//jft
	UObjectPreviewScene* ObjPreview;

	const float MinScale = 0.01f;
	UGizmo* Gizmo;
	UAxis* Axis;
	UGrid* Grid;

	FVector2 LastMousePosition = FVector2(0.0f, 0.0f);

	double TotalPickTime = 0.0;
	uint32_t TotalPickCount = 0;
	double LastPickTime = 0.0;
};
