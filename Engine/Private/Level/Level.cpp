#include "pch.h"
#include "Level/Level.h"

#include "Mesh/Actor.h"
#include "Mesh/StaticMeshComponent.h"
#include "Mesh/TextComponent.h"
#include "Render/Renderer/Renderer.h"
#include "Editor/Gizmo.h"

IMPLEMENT_CLASS(ULevel, UObject)

ULevel::ULevel() = default;

ULevel::~ULevel()
{
	for (auto Actor : LevelActors)
	{
		SafeDelete(Actor);
	}

	// Deprecated : EditorPrimitive는 에디터에서 처리
	// for (auto Actor : EditorActors)
	// {
	// 	SafeDelete(Actor);
	// }

	// 카메라의 실제 주인은 Editor라 삭제하면 안됨.
	//SafeDelete(CameraPtr);
}

void ULevel::Init()
{
	// TEST CODE
}

void ULevel::Update(float DeltaTime)
{
	// Process Delayed Task
	ProcessPendingDeletions();

	uint64 AllocatedByte = GetAllocatedBytes();
	uint32 AllocatedCount = GetAllocatedCount();

	//LevelPrimitiveComponents.clear();
	//LevelStaticMeshComponents.clear();
	//LastVisiblePrimitives.clear();
	//VisiblePrimitives.clear();
	//TextComponents.clear();
	//Deprecated : EditorPrimitive는 에디터에서 처리
	//EditorPrimitiveComponents.clear();

	for (auto& Actor : LevelActors)
	{
		if (Actor)
		{
			Actor->Tick(DeltaTime);
			//AddLevelPrimitiveComponent(Actor);
		}
	}

	//Deprecated : EditorPrimitive는 에디터에서 처리
	/*for (auto& Actor : EditorActors)
	{
		if (Actor)
		{
			Actor->Tick();
			AddEditorPrimitiveComponent(Actor);
		}
	}*/

}

void ULevel::Render()
{
}

void ULevel::Cleanup()
{
	// 현재 레벨의 액터 배열을 순회하며 메모리 해제
	for (auto Actor : LevelActors)
	{
		SafeDelete(Actor);
	}
	LevelActors.Empty();
	LevelPrimitiveComponents.Empty();
	LevelStaticMeshComponents.Empty();
	TextComponents.Empty();
}

void ULevel::AddLevelActor(AActor* Actor)
{
	URenderer::GetInstance().SetSortingBatchMapDirty();
	LevelActors.Add(Actor);
	for (auto& Component : Actor->GetOwnedComponents())
	{
		if (Component->GetComponentType() == EComponentType::Primitive)
		{
			UPrimitiveComponent* PrimitiveComponent = static_cast<UPrimitiveComponent*>(Component);
			UStaticMeshComponent* S = Cast<UStaticMeshComponent>(PrimitiveComponent);

			if (PrimitiveComponent->IsVisible())
			{
				LevelPrimitiveComponents.push_back(PrimitiveComponent);
				LevelStaticMeshComponents.Push(S);
			}
		}
		else if (Component->GetComponentType() == EComponentType::Text)
		{
			UTextRenderComponent* TextComponent = static_cast<UTextRenderComponent*>(Component);
			if (TextComponent->IsVisible())
				TextComponents.push_back(TextComponent);
		}
	}
}

void ULevel::AddLevelPrimitiveComponent(AActor* Actor)
{
	if (!Actor) return;

	for (auto& Component : Actor->GetOwnedComponents())
	{
		if (Component->GetComponentType() == EComponentType::Primitive)
		{
			UPrimitiveComponent* PrimitiveComponent = static_cast<UPrimitiveComponent*>(Component);
			UStaticMeshComponent* S = Cast<UStaticMeshComponent>(PrimitiveComponent);

			if (PrimitiveComponent->IsVisible())
			{
				LevelPrimitiveComponents.push_back(PrimitiveComponent);
				LevelStaticMeshComponents.Push(S);
			}
		}
		else if (Component->GetComponentType() == EComponentType::Text)
		{
			UTextRenderComponent* TextComponent = static_cast<UTextRenderComponent*>(Component);
			if(TextComponent->IsVisible())
				TextComponents.push_back(TextComponent);
		}
	}
}

void ULevel::SetSelectedActor(AActor* InActor)
{
	// Set Selected Actor
	if (SelectedActor)
	{
		for (auto& Component : SelectedActor->GetOwnedComponents())
		{
			if (Component->GetComponentType() >= EComponentType::Primitive)
			{
				UPrimitiveComponent* PrimitiveComponent = static_cast<UPrimitiveComponent*>(Component);
				if (PrimitiveComponent->IsVisible())
				{
					PrimitiveComponent->SetColor({0.f, 0.f, 0.f, 0.f});
				}
			}
		}
	}

	SelectedActor = InActor;
	if (SelectedActor)
	{
		for (auto& Component : SelectedActor->GetOwnedComponents())
		{
			if (Component->GetComponentType() >= EComponentType::Primitive)
			{
				UPrimitiveComponent* PrimitiveComponent = static_cast<UPrimitiveComponent*>(Component);
				if (PrimitiveComponent->IsVisible())
				{
					PrimitiveComponent->SetColor({1.f, 0.8f, 0.2f, 0.4f});
				}
			}
		}
	}
	//Gizmo->SetTargetActor(SelectedActor);
}

/**
 * @brief Level에서 Actor 제거하는 함수
 */
bool ULevel::DestroyActor(AActor* InActor)
{
	if (!InActor)
	{
		return false;
	}
	URenderer::GetInstance().SetSortingBatchMapDirty();

    // LevelActors 리스트에서 제거
    for (int i = 0; i < static_cast<int>(LevelActors.size()); ++i)
    {
        if (LevelActors[i] == InActor)
        {
            LevelActors.erase(LevelActors.begin() + i);
            break;
        }
    }

    // 이 Actor가 소유한 Primitive/StaticMesh/Text 컴포넌트 전부 제거 (인덱스 정합성에 의존하지 않음)
    {
        // Primitive
        for (int i = static_cast<int>(LevelPrimitiveComponents.size()) - 1; i >= 0; --i)
        {
            if (LevelPrimitiveComponents[i] && LevelPrimitiveComponents[i]->GetOwner() == InActor)
            {
                LevelPrimitiveComponents.erase(LevelPrimitiveComponents.begin() + i);
            }
        }
        // StaticMesh
        for (int i = static_cast<int>(LevelStaticMeshComponents.size()) - 1; i >= 0; --i)
        {
            if (LevelStaticMeshComponents[i] && LevelStaticMeshComponents[i]->GetOwner() == InActor)
            {
                LevelStaticMeshComponents.erase(LevelStaticMeshComponents.begin() + i);
            }
        }
        // Text
        for (int i = static_cast<int>(TextComponents.size()) - 1; i >= 0; --i)
        {
            if (TextComponents[i] && TextComponents[i]->GetOwner() == InActor)
            {
                TextComponents.erase(TextComponents.begin() + i);
            }
        }
    }

	//Deprecated : EditorPrimitive는 에디터에서 처리
	// 필요하다면 EditorActors 리스트에서도 제거
	/*for (auto Iterator = EditorActors.begin(); Iterator != EditorActors.end(); ++Iterator)
	{
		if (*Iterator == InActor)
		{
			EditorActors.erase(Iterator);
			break;
		}
	}*/

	// Remove Actor Selection
	if (SelectedActor == InActor)
	{
		SelectedActor = nullptr;

		//Deprecated : Gizmo는 에디터에서 처리
		// Gizmo Target Release
		/*if (Gizmo)
		{
			Gizmo->SetTargetActor(nullptr);
		}*/
	}

	// Remove
	delete InActor;

	//UE_LOG("Level: Actor Destroyed Successfully");
	return true;
}

/**
 * @brief Delete In Next Tick
 */
void ULevel::MarkActorForDeletion(AActor* InActor)
{
	if (!InActor)
	{
		//UE_LOG("Level: MarkActorForDeletion: InActor Is Null");
		return;
	}

	// 이미 삭제 대기 중인지 확인
	for (AActor* PendingActor : ActorsToDelete)
	{
		if (PendingActor == InActor)
		{
			//UE_LOG("Level: Actor Already Marked For Deletion");
			return;
		}
	}

	// 삭제 대기 리스트에 추가
	ActorsToDelete.push_back(InActor);
	//UE_LOG("Level: Actor Marked For Deletion In Next Tick: %p", InActor);

	// 선택 해제는 바로 처리
	if (SelectedActor == InActor)
	{
		SelectedActor = nullptr;

		//Deprecated : Gizmo는 에디터에서 처리
		// Gizmo Target도 즉시 해제
		/*if (Gizmo)
		{
			Gizmo->SetTargetActor(nullptr);
		}*/
	}
}

/**
 * @brief Level에서 Actor를 실질적으로 제거하는 함수
 * 이전 Tick에서 마킹된 Actor를 제거한다
 */
void ULevel::ProcessPendingDeletions()
{
	if (ActorsToDelete.empty())
	{
		return;
	}

	//UE_LOG("[Level] Processing %zu Pending Deletions", ActorsToDelete.size());

	// 대기 중인 액터들을 삭제
	for (AActor* ActorToDelete : ActorsToDelete)
	{
		if (!ActorToDelete)
			continue;

		// 혹시 남아있을 수 있는 참조 정리
		if (SelectedActor == ActorToDelete)
		{
			SelectedActor = nullptr;
			/*if (Gizmo)
			{
				Gizmo->SetTargetActor(nullptr);
			}*/
		}

        // LevelActors 리스트에서 제거
        for (int i = 0; i < static_cast<int>(LevelActors.size()); ++i)
        {
            if (LevelActors[i] == ActorToDelete)
            {
                LevelActors.erase(LevelActors.begin() + i);
                break;
            }
        }

        // 이 Actor가 소유한 Primitive/StaticMesh/Text 컴포넌트 전부 제거 (인덱스 정합성에 의존하지 않음)
        {
            // Primitive
            for (int i = static_cast<int>(LevelPrimitiveComponents.size()) - 1; i >= 0; --i)
            {
                if (LevelPrimitiveComponents[i] && LevelPrimitiveComponents[i]->GetOwner() == ActorToDelete)
                {
                    LevelPrimitiveComponents.erase(LevelPrimitiveComponents.begin() + i);
                }
            }
            // StaticMesh
            for (int i = static_cast<int>(LevelStaticMeshComponents.size()) - 1; i >= 0; --i)
            {
                if (LevelStaticMeshComponents[i] && LevelStaticMeshComponents[i]->GetOwner() == ActorToDelete)
                {
                    LevelStaticMeshComponents.erase(LevelStaticMeshComponents.begin() + i);
                }
            }
            // Text
            for (int i = static_cast<int>(TextComponents.size()) - 1; i >= 0; --i)
            {
                if (TextComponents[i] && TextComponents[i]->GetOwner() == ActorToDelete)
                {
                    TextComponents.erase(TextComponents.begin() + i);
                }
            }
        }
		URenderer::GetInstance().SetSortingBatchMapDirty();


		//Deprecated : EditorActor는 에디터에서 처리
		// EditorActors 리스트에서도 제거
		/*for (auto Iterator = EditorActors.begin(); Iterator != EditorActors.end(); ++Iterator)
		{
			if (*Iterator == ActorToDelete)
			{
				EditorActors.erase(Iterator);
				break;
			}
		}*/

		// Release Memory
		delete ActorToDelete;
		//UE_LOG("[Level] Actor Deleted: %p", ActorToDelete);
	}

	// Clear TArray
	ActorsToDelete.Empty();
	//UE_LOG("[Level] All Pending Deletions Processed");
}

void ULevel::DuplicateSubObjects()
{
	TArray<AActor*> NewLevelActors;

	LevelPrimitiveComponents.clear();
	LevelStaticMeshComponents.clear();
	TextComponents.clear();

	for (auto& Actor : LevelActors)
	{
		if (Actor)
		{
			Actor = Actor->Duplicate();
			Actor->SetOuter(this);
			NewLevelActors.push_back(Actor);
		}
	}

	LevelActors.clear();

	for (auto& Actor : NewLevelActors)
	{
		AddLevelActor(Actor);
	}
}

ULevel* ULevel::Duplicate()
{
	ULevel* NewLevel = new ULevel(*this);

	//ULevel* NewLevel = new ULevel(*this);
	//
	//NewLevel static_cast<ULevel*>=UObject::Duplicate();

	NewLevel->SelectedActor = nullptr;
	//NewLevel->Axis = nullptr;
	//NewLevel->Grid = nullptr;
	//NewLevel->CameraPtr = nullptr;

	NewLevel->DuplicateSubObjects();

	return NewLevel;
}
