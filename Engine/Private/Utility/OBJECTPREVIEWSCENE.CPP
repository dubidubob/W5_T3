#include "pch.h"
#include "Editor/Camera.h"
#include "Render/Renderer/Renderer.h"
#include "Public/Utility/ObjectPreviewScene.h"
#include "Public/Mesh/Actor.h"
#include "Public/Mesh/StaticMeshComponent.h"
#include "Public/Mesh/StaticMesh/StaticMesh.h"

IMPLEMENT_CLASS(UObjectPreviewScene, UObject)

UObjectPreviewScene::UObjectPreviewScene()
{
	MiniCamera = NewObject<UCamera>();
	if (URenderer::GetInstance().GetDeviceResources())
	{
		float Width = URenderer::GetInstance().GetDeviceResources()->GetViewportInfo().Width;
		float Height = URenderer::GetInstance().GetDeviceResources()->GetViewportInfo().Height;
		MiniCamera->SetAspect(Width / Height);
	}

	MiniCamera->SetLocation(FVector( - 0.7f, 0.0f, 0.0f));
	MiniCamera->UpdateMatrixByPers();
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
	if (!InSelectedActor)
	{
		if (!PreviewComponents.empty())
		{
			ClearPreview();
		}
		return;
	}

	if (InSelectedActor->GetUUID() == UUID) return; /* 이미 선택한 것, Texture 등 달라져도 변하지 않는다...*/
	UUID = InSelectedActor->GetUUID();

	ClearPreview();

	for (UActorComponent* Component : InSelectedActor->GetOwnedComponents())
	{
		if (UStaticMeshComponent* SrcComp = Cast<UStaticMeshComponent>(Component))
		{
			if (UStaticMesh* SrcMesh = SrcComp->GetStaticMesh())
			{
				UStaticMeshComponent* NewComp = NewObject<UStaticMeshComponent>();
				NewComp->SetStaticMesh(SrcComp->GetStaticMesh());

				NewComp->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
				NewComp->SetRelativeRotation(FVector(0.0f, 0.0f, 0.0f));

				PreviewComponents.push_back(NewComp);
			}
		}
	}

	/*jft*/
	MiniCamera->UpdateMatrixByPers();
}

TArray<UPrimitiveComponent*> UObjectPreviewScene::GetPrimitiveInObjViewer()
{
	return PreviewComponents;
}

void UObjectPreviewScene::SetComponentScale(FVector Scale)
{
	for (UPrimitiveComponent* Comp : PreviewComponents)
	{
		if (Comp)
		{
			Comp->SetRelativeScale3D(Scale);
		}
	}
}

void UObjectPreviewScene::SetComponentRotation(FVector Rotation)
{
	for (UPrimitiveComponent* Comp : PreviewComponents)
	{
		if (Comp)
		{
			Comp->SetRelativeRotation(Rotation);
		}
	}
}

void UObjectPreviewScene::SetCameraLocation(FVector Location)
{
	MiniCamera->SetLocation(Location);
}
