#include "pch.h"
#include "Render/UI/Widget/ActorDetailWidget.h"

#include "Level/Level.h"
#include "Manager/Level/LevelManager.h"
#include "Mesh/Actor.h"
#include "Core/ObjectIterator.h"
#include "Mesh/StaticMesh/StaticMesh.h"
#include "Mesh/StaticMeshActor.h"
#include "Core/Object.h"
#include "Mesh/StaticMeshComponent.h"
UActorDetailWidget::UActorDetailWidget()
{
}

void UActorDetailWidget::Initialize()
{
	UE_LOG("ActorDetailWidget: Successfully Initialized");
}

void UActorDetailWidget::Update()
{
	ULevelManager& LevelManager = ULevelManager::GetInstance();
	ULevel* CurrentLevel = LevelManager.GetCurrentLevel();

	if (CurrentLevel)
	{
		AActor* CurrentSelectedActor = CurrentLevel->GetSelectedActor();
		if (SelectedActor != CurrentSelectedActor)
		{
			SelectedActor = CurrentSelectedActor;

			if (SelectedActor)
			{
				FString ActorName = SelectedActor->GetName();
				strncpy_s(ActorNameBuffer, ActorName.c_str(), sizeof(ActorNameBuffer) - 1);
				ActorNameBuffer[sizeof(ActorNameBuffer) - 1] = '\0';
			}
			else
			{
				ActorNameBuffer[0] = '\0';
			}
		}
	}
}

void UActorDetailWidget::RenderWidget()
{
	ImGui::Text("Actor Details");
	ImGui::Separator();

	if (SelectedActor)
	{
		RenderActorInfo();
		ImGui::Separator();
		RenderNameField();
		ImGui::Separator();
		RenderDropListUI();
	}
	else
	{
		ImGui::TextUnformatted("No Actor Selected");
	}

	if (bNameChanged && SelectedActor)
	{
		SelectedActor->SetName(FString(ActorNameBuffer));
		bNameChanged = false;
	}
}

void UActorDetailWidget::RenderActorInfo()
{
	if (SelectedActor && IsValid(SelectedActor))
	{
		UClass* ActorClass = SelectedActor->GetClass();
		FString ClassName = ActorClass ? ActorClass->GetName() : "Unknown";

		ImGui::Text("Class: %s", ClassName.c_str());

		void* ActorPtr = static_cast<void*>(SelectedActor);
		ImGui::Text("Address: %p", ActorPtr);

		uint64 MemoryUsage = SelectedActor->GetAllocatedBytes();
		ImGui::Text("Memory: %llu bytes", MemoryUsage);
	}
}

void UActorDetailWidget::RenderNameField()
{
	ImGui::Text("Name");
	if (ImGui::InputText("##ActorName", ActorNameBuffer, sizeof(ActorNameBuffer)))
	{
		bNameChanged = true;
	}
}

void UActorDetailWidget::RenderDropListUI()
{
	ImGui::Text("StaticMesh");

	static int CurrentMeshIndex = 0;

	// 1) UI 표시용 이름(확장자 제거) + 실제 파일명(확장자 포함) 두 개 관리
	TArray<FString> MeshNamesDisplay;
	TArray<FString> MeshNamesFull;

	for (TObjectIterator<UStaticMesh> It; It; ++It)
	{
		UStaticMesh* Mesh = *It;
		if (Mesh && Mesh->GetStaticMeshAsset())
		{
			FString FullName = Mesh->GetStaticMeshAsset()->GetFileName(); // ex) "Demon.obj"

			// 실제 파일명은 그대로 저장
			MeshNamesFull.Add(FullName);

			// 표시용 이름 (확장자 제거)
			size_t DotPos = FullName.find_last_of('.');
			if (DotPos != FString::npos)
				MeshNamesDisplay.Add(FullName.substr(0, DotPos));
			else
				MeshNamesDisplay.Add(FullName);
		}
	}

	if (MeshNamesDisplay.Num() == 0)
	{
		ImGui::Text("No StaticMesh Loaded");
		return;
	}

	// 2) 현재 Actor의 Mesh가 있으면 맨 앞으로 이동
	if (SelectedActor && IsValid(SelectedActor))
	{
		if (AStaticMeshActor* SMActor = Cast<AStaticMeshActor>(SelectedActor))
		{
			if (UStaticMeshComponent* MeshComp = SMActor->GetStaticMeshComponent())
			{
				FString CurrentMeshName = MeshComp->GetStaticMesh()
					->GetStaticMeshAsset()->GetFileName();

				for (int i = 0; i < MeshNamesFull.Num(); i++)
				{
					if (MeshNamesFull[i] == CurrentMeshName)
					{
						if (i != 0)
						{
							// Full과 Display 배열 모두 동기화해서 앞으로 이동
							FString FullTemp = MeshNamesFull[i];
							FString DisplayTemp = MeshNamesDisplay[i];

							MeshNamesFull.RemoveAt(i);
							MeshNamesDisplay.RemoveAt(i);

							MeshNamesFull.Insert(FullTemp, 0);
							MeshNamesDisplay.Insert(DisplayTemp, 0);
						}
						CurrentMeshIndex = 0;
						break;
					}
				}
			}
		}
	}

	// 3) ImGui 콤보박스: 표시용 이름만 출력
	auto ItemGetter = [](void* data, int idx, const char** out_text)
		{
			auto Names = static_cast<TArray<FString>*>(data);
			*out_text = (*Names)[idx].c_str();
			return true;
		};

	if (ImGui::Combo("##StaticMeshCombo",
		&CurrentMeshIndex,
		ItemGetter,
		(void*)&MeshNamesDisplay,
		MeshNamesDisplay.Num()))
	{
		// 4) 선택 변경 시 실제 Mesh 교체
		if (SelectedActor)
		{
			if (AStaticMeshActor* SMActor = Cast<AStaticMeshActor>(SelectedActor))
			{
				if (UStaticMeshComponent* MeshComp = SMActor->GetStaticMeshComponent())
				{
					FString SelectedFullName = MeshNamesFull[CurrentMeshIndex];
					MeshComp->SetStaticMesh(SelectedFullName);

					UE_LOG("Changed mesh to: %s", SelectedFullName.c_str());
				}
			}
		}
	}
}
