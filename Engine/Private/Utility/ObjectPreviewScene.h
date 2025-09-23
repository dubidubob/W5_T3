#pragma once
#include "Core/Object.h"

class AActor;
class UPrimitiveComponent;
class UObjectPreviewScene : public UObject
{
	DECLARE_CLASS(UObjectPreviewScene, UObject)
public :
	UObjectPreviewScene();
	~UObjectPreviewScene() override;

	void SetActorInObj();


private :
	AActor* SelectedActorInObjViewer = nullptr;
	TArray<UPrimitiveComponent*> SelectedActorInObjViewerComponent;
};

