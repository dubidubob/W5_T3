#include "pch.h"
#include "Render/UI/Widget/ActorListWidget.h"

#include "Level/Level.h"
#include "Manager/Level/LevelManager.h"
#include "Mesh/Actor.h"

UActorListWidget::UActorListWidget()
{
}

void UActorListWidget::Initialize()
{
	UE_LOG("ActorListWidget: Successfully Initialized");
}

void UActorListWidget::Update()
{
	ULevelManager& LevelManager = ULevelManager::GetInstance();
	CurrentLevel = LevelManager.GetCurrentLevel();

	if (CurrentLevel)
	{
		AActor* CurrentSelectedActor = CurrentLevel->GetSelectedActor();
		if (SelectedActor != CurrentSelectedActor)
		{
			SelectedActor = CurrentSelectedActor;
		}
	}
}

void UActorListWidget::RenderWidget()
{
	ImGui::Text("Scene Actors");
	ImGui::Separator();

	if (CurrentLevel)
	{
		RenderActorList();
	}
	else
	{
		ImGui::TextUnformatted("No Level Loaded");
	}
}

void UActorListWidget::RenderActorList()
{
	const TArray<AActor*>& Actors = CurrentLevel->GetLevelActors();

	if (Actors.empty())
	{
		ImGui::TextUnformatted("No Actors in Scene");
		return;
	}

	if (ImGui::BeginChild("ActorList", ImVec2(0, 0), true))
	{
		for (int32 i = 0; i < Actors.size(); ++i)
		{
			if (Actors[i])
			{
				RenderActorItem(Actors[i], i);
			}
		}
	}
	ImGui::EndChild();
}

void UActorListWidget::RenderActorItem(AActor* InActor, int32 InIndex)
{
	FString DisplayName = GetActorDisplayName(InActor);
	FString ClassName = GetActorClassName(InActor);

	bool bIsSelected = (InActor == SelectedActor);

	if (ImGui::Selectable(DisplayName.c_str(), bIsSelected))
	{
		if (CurrentLevel)
		{
			CurrentLevel->SetSelectedActor(InActor);
			SelectedActor = InActor;
			SelectedActorIndex = InIndex;
		}
	}

	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::Text("Name: %s", DisplayName.c_str());
		ImGui::Text("Class: %s", ClassName.c_str());
		ImGui::EndTooltip();
	}
}

FString UActorListWidget::GetActorDisplayName(AActor* InActor) const
{
	if (!InActor)
		return "None";

	FString ActorName = InActor->GetName();
	if (ActorName.empty())
	{
		return GetActorClassName(InActor) + "_" + to_string(reinterpret_cast<uintptr_t>(InActor));
	}

	return ActorName;
}

FString UActorListWidget::GetActorClassName(AActor* InActor) const
{
	if (!InActor)
		return "Unknown";

	UClass* ActorClass = InActor->GetClass();
	if (ActorClass)
	{
		return ActorClass->GetName();
	}

	return "Unknown";
}
