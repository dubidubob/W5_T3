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
		ImGui::Separator();
		RenderUVScrollBox();
		ImGui::Separator();
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

	TArray<FString> MeshNamesDisplay;
	TArray<FString> MeshNamesFull;

	for (TObjectIterator<UStaticMesh> It; It; ++It)
	{
		UStaticMesh* Mesh = *It;
		if (Mesh && Mesh->GetStaticMeshAsset())
		{
			FString FullName = Mesh->GetStaticMeshAsset()->GetFileName();

			MeshNamesFull.Add(FullName);

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

	if (!SelectedActor || !IsValid(SelectedActor))
		return;

	AStaticMeshActor* SMActor = Cast<AStaticMeshActor>(SelectedActor);
	if (!SMActor)
		return;

	UStaticMeshComponent* MeshComp = SMActor->GetStaticMeshComponent();
	if (!MeshComp || !MeshComp->GetStaticMesh())
		return;

	FString CurrentMeshName = MeshComp->GetStaticMesh()->GetStaticMeshAsset()->GetFileName();

	for (int i = 0; i < MeshNamesFull.Num(); i++)
	{
		if (MeshNamesFull[i] != CurrentMeshName)
			continue;

		if (i != 0)
		{
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
		if (!SelectedActor)
			return;

		AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(SelectedActor);
		if (!StaticMeshActor)
			return;

		UStaticMeshComponent* MeshComp = StaticMeshActor->GetStaticMeshComponent();
		if (!MeshComp)
			return;

		const FString& SelectedFullName = MeshNamesFull[CurrentMeshIndex];
		MeshComp->SetStaticMesh(SelectedFullName);

		UE_LOG("Changed mesh to: %s", SelectedFullName.c_str());
	}
}

void UActorDetailWidget::RenderUVScrollBox()
{
	AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(SelectedActor);
	if (!StaticMeshActor) return;

	UStaticMeshComponent* MeshComp = StaticMeshActor->GetStaticMeshComponent();
	if (!MeshComp) return;

	// 현재 상태 가져오기
	bool bUseUVScroll = MeshComp->GetUseUVScroll();

	// 체크박스 렌더링
	if (ImGui::Checkbox("Use UV Scroll", &bUseUVScroll))
	{
		// 값이 바뀌면 컴포넌트 통해 반영
		MeshComp->SetUseUVScroll(bUseUVScroll);
	}
}
