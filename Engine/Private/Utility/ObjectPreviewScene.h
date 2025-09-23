#pragma once
#include "Global/CoreTypes.h"
#include "Core/Object.h"

class AActor;
class UPrimitiveComponent;
class UObjectPreviewScene : public UObject
{
	DECLARE_CLASS(UObjectPreviewScene, UObject)
public :
	UObjectPreviewScene();
	~UObjectPreviewScene() override;

	/*선택된 Actor 복사 및 SelectedActorInObjViewerComponent Initialize*/
	void SetActorInObjViewer(AActor* InSelectedActor);

	/*Renderer의 Render Target View에 Render 해야하는 걸 넘겨준다.*/
	TArray<UPrimitiveComponent*> GetPrimitiveInObjViewer();

private :
	AActor* SelectedActorInObjViewer = nullptr;
	TArray<UPrimitiveComponent*> ObjViewerComponents;
};
