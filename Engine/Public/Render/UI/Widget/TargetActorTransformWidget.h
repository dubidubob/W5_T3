#pragma once
#include "Widget.h"

class UObjectPreviewScene;
class UTargetActorTransformWidget : public UWidget
{
	DECLARE_CLASS(UTargetActorTransformWidget, UWidget)
public:
	void Initialize() override;
	void Update() override;
	void RenderWidget() override;
	void PostProcess() override;

	void UpdateTransformFromActor();
	void ApplyTransformToActor() const;
#if IS_OBJ_VIEWER
	void SetObjectViewer(UObjectPreviewScene* InObjectPreview);
#endif
	// Special Member Function
	UTargetActorTransformWidget();
	~UTargetActorTransformWidget() override;

private:
	class AActor* SelectedActor;
	UObjectPreviewScene* ObjectPreview = nullptr;

	FVector EditLocation;
	FVector EditRotation;
	FVector EditScale;
	bool bScaleChanged;
	bool bRotationChanged;
	bool bPositionChanged;
	uint64 LevelMemoryByte;
	uint32 LevelObjectCount;
};
