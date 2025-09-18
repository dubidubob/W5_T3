#include "pch.h"
#include "Render/UI/Widget/ActorDetailWidget.h"

#include "Level/Level.h"
#include "Manager/Level/LevelManager.h"
#include "Mesh/Actor.h"

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
	if (!SelectedActor)
		return;

	UClass* ActorClass = SelectedActor->GetClass();
	FString ClassName = ActorClass ? ActorClass->GetName() : "Unknown";

	ImGui::Text("Class: %s", ClassName.c_str());

	void* ActorPtr = static_cast<void*>(SelectedActor);
	ImGui::Text("Address: %p", ActorPtr);

	uint64 MemoryUsage = SelectedActor->GetAllocatedBytes();
	ImGui::Text("Memory: %llu bytes", MemoryUsage);
}

void UActorDetailWidget::RenderNameField()
{
	ImGui::Text("Name");
	if (ImGui::InputText("##ActorName", ActorNameBuffer, sizeof(ActorNameBuffer)))
	{
		bNameChanged = true;
	}
}
