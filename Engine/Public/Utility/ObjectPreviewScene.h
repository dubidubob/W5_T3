#pragma once
#include "Global/CoreTypes.h"
#include "Core/Object.h"

class AActor;
class UPrimitiveComponent;
class UStaticMeshComponent;
class UCamera;

class UObjectPreviewScene : public UObject
{
	DECLARE_CLASS(UObjectPreviewScene, UObject)
public :
	UObjectPreviewScene();
	~UObjectPreviewScene() override;

	/*선택된 Actor로부터 StaticMesh, Material 등 Asset 정보를 가져와 Preview Component에 설정*/
	void UpdatePreviewFromActor(AActor* InSelectedActor);
	void ClearPreview();
	UCamera* GetCamera() { return MiniCamera; }

	/*Renderer의 Render Target View에 Render 해야하는 걸 넘겨준다.*/
	TArray<UPrimitiveComponent*> GetPrimitiveInObjViewer();

private :
	uint32 UUID = -1;
	TArray<UPrimitiveComponent*> PreviewComponents;
	UCamera* MiniCamera;
};
