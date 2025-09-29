#include "pch.h"
#include "Render/UI/Widget/PrimitiveSpawnWidget.h"

#include "Level/Level.h"
#include "Manager/Level/LevelManager.h"
#include "Mesh/CubeActor.h"
#include "Mesh/SphereActor.h"
#include "Mesh/SquareActor.h"
#include "Mesh/TriangleActor.h"
#include "Mesh/StaticMeshActor.h"
#include "Mesh/StaticMeshComponent.h"
#include "Manager/Path/PathManager.h"

#include "Editor/Editor.h"
#include "Core/ObjectIterator.h"

IMPLEMENT_CLASS(UPrimitiveSpawnWidget, UWidget)

UPrimitiveSpawnWidget::UPrimitiveSpawnWidget()
{
}

UPrimitiveSpawnWidget::~UPrimitiveSpawnWidget() = default;

void UPrimitiveSpawnWidget::Initialize()
{
	// Do Nothing Here
}

void UPrimitiveSpawnWidget::Update()
{
	// Do Nothing Here
}

void UPrimitiveSpawnWidget::RenderWidget()
{
	ImGui::Text("Primitive Actor 생성");
	ImGui::Spacing();
	ImGui::Text("Primitive Type:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(120);
	ImGui::Combo("##PrimitiveType",
		&SelectedPrimitiveType,
		PrimitiveTypes.data(),
		PrimitiveTypes.Num());

	// Spawn 버튼과 개수 입력
	ImGui::Text("Number of Spawn:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(80);
	ImGui::InputInt("##NumberOfSpawn", &NumberOfSpawn);
	NumberOfSpawn = max(1, NumberOfSpawn);
	NumberOfSpawn = min(10000, NumberOfSpawn);

	ImGui::SameLine();
	if (ImGui::Button("Spawn Actors"))
	{
		SpawnActors();
	}

	// 스폰 범위 설정
	ImGui::Text("Spawn Range:");
	ImGui::SetNextItemWidth(80);
	ImGui::DragFloat("Min##SpawnRange", &SpawnRangeMin, 0.1f, -50.0f, SpawnRangeMax - 0.1f);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(80);
	ImGui::DragFloat("Max##SpawnRange", &SpawnRangeMax, 0.1f, SpawnRangeMin + 0.1f, 50.0f);

	ImGui::Separator();
}

/**
 * @brief Actor 생성 함수
 * 난수를 활용한 Range, Size, Rotion 값 생성으로 Actor Spawn
 */
void UPrimitiveSpawnWidget::SpawnActors() const
{
	ULevelManager& LevelManager = ULevelManager::GetInstance();
	ULevel* CurrentLevel = LevelManager.GetCurrentLevel();

	if (!CurrentLevel)
	{
		UE_LOG("ControlPanel: No Current Level To Spawn Actors");
		return;
	}

	UE_LOG("ControlPanel: %s 타입의 Actor를 %d개 생성했습니다",
		(SelectedPrimitiveType == 0 ? "Cube" : "Sphere"), NumberOfSpawn);

	// 지정된 개수만큼 액터 생성
	for (int32 i = 0; i < NumberOfSpawn; i++)
	{
		AStaticMeshActor* NewActor = nullptr;

		FString Name = PrimitiveTypes[SelectedPrimitiveType];
		path StaticMeshPath = path("Data") / (Name + ".obj");

		if (!Name.empty())
		{
			NewActor = CurrentLevel->SpawnActor<AStaticMeshActor>();
			NewActor->GetStaticMeshComponent()->SetStaticMeshByPath(StaticMeshPath);
		}

		if (NewActor)
		{
			// 범위 내 랜덤 위치
			float RandomX = SpawnRangeMin + (static_cast<float>(rand()) / RAND_MAX) * (SpawnRangeMax - SpawnRangeMin);
			float RandomY = SpawnRangeMin + (static_cast<float>(rand()) / RAND_MAX) * (SpawnRangeMax - SpawnRangeMin);
			float RandomZ = SpawnRangeMin + (static_cast<float>(rand()) / RAND_MAX) * (SpawnRangeMax - SpawnRangeMin);

			NewActor->SetActorLocation(FVector(RandomX, RandomY, RandomZ));

			// 임의의 스케일 (0.5 ~ 2.0 범위)
			float RandomScale = 0.5f + (static_cast<float>(rand()) / RAND_MAX) * 1.5f;
			NewActor->SetActorScale3D(FVector(RandomScale, RandomScale, RandomScale));

			UEditor* Editor = nullptr;
			for (TObjectIterator<UEditor> it; it; ++it)
			{
				if (it->IsA(UEditor::StaticClass()))
				{
					Editor = static_cast<UEditor*>(*it);
					break;
				}
			}

			if (Editor)
			{
				if (Editor->IsUsingOctreeForPicking())
				{
					Editor->RebuildOctree();
				}
				else if (Editor->IsUsingBVHForPicking())
				{
					Editor->RebuildBVH();
				}
			}
		}
		else
		{
			UE_LOG("ControlPanel: Actor 생성에 실패했습니다 %d", i);
		}
	}
}
