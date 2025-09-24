#include "pch.h"
#include "Utility/ObjectPreviewScene.h"
#include "Mesh/Actor.h"

IMPLEMENT_CLASS(UObjectPreviewScene, UObject)

UObjectPreviewScene::UObjectPreviewScene()
{
}

UObjectPreviewScene::~UObjectPreviewScene()
{
}

void UObjectPreviewScene::SetActorInObjViewer(AActor* InSelectedActor)
{
	/*if (!InSelectedActor) return;
	if (SelectedActorInObjViewer)
	{
		if (InSelectedActor->GetUUID() == SelectedActorInObjViewer->GetUUID())
			return;
		SafeDelete(SelectedActorInObjViewer);
	}

	SelectedActorInObjViewer = new AActor(*InSelectedActor);
	ObjViewerComponents.clear();

	for (auto& ActorComponent : SelectedActorInObjViewer->GetOwnedComponents())
	{
		UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(ActorComponent);
		if (Primitive)
		{
			ObjViewerComponents.push_back(Primitive);
		}
	}*/
}

TArray<UPrimitiveComponent*> UObjectPreviewScene::GetPrimitiveInObjViewer()
{
	/*return ObjViewerComponents;*/
}

