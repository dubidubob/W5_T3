#include "pch.h"
#include "Public/Utility/ObjectPreviewScene.h"
#include "Public/Mesh/Actor.h"
#include "Public/Mesh/StaticMeshComponent.h"
#include "Public/Mesh/StaticMesh/StaticMesh.h"

IMPLEMENT_CLASS(UObjectPreviewScene, UObject)

UObjectPreviewScene::UObjectPreviewScene()
{
}

UObjectPreviewScene::~UObjectPreviewScene()
{
}

void UObjectPreviewScene::ClearPreview()
{
	for (UPrimitiveComponent* Comp : PreviewComponents)
	{
		SafeDelete(Comp); 
	}
	PreviewComponents.clear();
}

void UObjectPreviewScene::UpdatePreviewFromActor(AActor* InSelectedActor)
{
	if (!InSelectedActor) return;
	if (InSelectedActor->GetUUID() == UUID) return; /* 이미 선택한 것, Texture 등 달라져도 변하지 않는다...*/
	UUID = InSelectedActor->GetUUID();

	ClearPreview();

	for (UActorComponent* Component : InSelectedActor->GetOwnedComponents())
	{
		if (UStaticMeshComponent* SrcComp = Cast<UStaticMeshComponent>(Component))
		{
			if (UStaticMesh* SrcMesh = SrcComp->GetStaticMesh())
			{
				UStaticMeshComponent* NewComp = new UStaticMeshComponent();
				NewComp->SetStaticMesh(SrcComp->GetStaticMesh());

				NewComp->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
				NewComp->SetRelativeRotation(FVector(0.0f, 0.0f, 0.0f));

				PreviewComponents.push_back(NewComp);
			}
		}
	}
}
	

TArray<UPrimitiveComponent*> UObjectPreviewScene::GetPrimitiveInObjViewer()
{
	return PreviewComponents;
}
