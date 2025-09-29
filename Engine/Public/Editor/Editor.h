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
class FOctree;
struct FAABB;
class UPrimitiveComponent;

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

	/** Octree Management */
	FOctree* GetOctree() const { return SceneOctree; }
	void InitializeOctree(const FAABB& WorldBounds);
	void RebuildOctree();
	void UpdateOctree();

	/** Octree Visualization */
	void SetOctreeVisualization(bool bEnabled) { bShowOctreeVisualization = bEnabled; }
	bool IsOctreeVisualizationEnabled() const { return bShowOctreeVisualization; }
	void SetUseOctreeForPicking(bool bEnabled) { bUseOctreeForPicking = bEnabled; }

	/** Frustum Culling */
	void SetUseFrustumCulling(bool bEnabled) { bUseFrustumCulling = bEnabled; }
	bool IsUsingFrustumCulling() const { return bUseFrustumCulling; }
	TArray<UPrimitiveComponent*> GetVisiblePrimitivesInFrustum() const;

private:
	void ProcessKeyboardInput();
	void ProcessMouseInput(ULevel* InLevel);

	void HandleGizmo(ULevel* InLevel, FRay InWorldRay);
	TArray<UPrimitiveComponent*> FindCandidatePrimitives(ULevel* InLevel);
	void PopulateOctreeFromLevel(ULevel* InLevel);
	void PopulateOctreeFromCurrentLevel();
	void RegisterPrimitiveToOctree(UPrimitiveComponent* Primitive);

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

	/** Octree for spatial optimization */
	FOctree* SceneOctree;
	bool bUseOctreeForPicking = true;
	bool bShowOctreeVisualization = true;

	/** Frustum Culling */
	bool bUseFrustumCulling = true;

	FVector2 LastMousePosition = FVector2(0.0f, 0.0f);

	double TotalPickTime = 0.0;
	uint32_t TotalPickCount = 0;
	double LastPickTime = 0.0;
};
