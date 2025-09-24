#include "pch.h"
#include "Render/UI/Widget/TargetActorTransformWidget.h"
#include "Mesh/Actor.h"
#include "Level/Level.h"
#include "Manager/Level/LevelManager.h"
#include "Render/Renderer/Renderer.h"
#if IS_OBJ_VIEWER
#include "Utility/ObjectPreviewScene.h"
#endif

IMPLEMENT_CLASS(UTargetActorTransformWidget, UWidget)
UTargetActorTransformWidget::UTargetActorTransformWidget()
{
}

UTargetActorTransformWidget::~UTargetActorTransformWidget() = default;

void UTargetActorTransformWidget::Initialize()
{
	// Do Nothing Here
}

void UTargetActorTransformWidget::Update()
{
	// 매 프레임 Level의 선택된 Actor를 확인해서 정보 반영
	ULevelManager& LevelManager = ULevelManager::GetInstance();
	ULevel* CurrentLevel = LevelManager.GetCurrentLevel();

	LevelMemoryByte = CurrentLevel->GetAllocatedBytes();
 	LevelObjectCount = CurrentLevel->GetAllocatedCount();

	if (CurrentLevel)
	{
		AActor* CurrentSelectedActor = CurrentLevel->GetSelectedActor();

		// Update Current Selected Actor
		if (SelectedActor != CurrentSelectedActor)
		{
			SelectedActor = CurrentSelectedActor;
		}

		// Get Current Selected Actor Information
		if (SelectedActor)
		{
			UpdateTransformFromActor();
		}
		else if (CurrentSelectedActor)
		{
			SelectedActor = nullptr;
		}
	}
}

void UTargetActorTransformWidget::RenderWidget()
{
	// Level Memory Information
	ImGui::Text("Level Memory Information");
	ImGui::Text("Level Object Count: %s", to_string(LevelObjectCount).c_str());
	ImGui::Text("Level Memory Byte: %s", to_string(LevelMemoryByte).c_str());
	ImGui::Separator();

	ImGui::Text("Transform");

	if (SelectedActor && IsValid(SelectedActor))
	{
		bPositionChanged |= ImGui::DragFloat3("Location", &EditLocation.X, 0.1f);
		bRotationChanged |= ImGui::DragFloat3("Rotation", &EditRotation.X, 0.1f);

		// Uniform Scale 옵션
		bool bUniformScale = SelectedActor->IsUniformScale();
		if (bUniformScale)
		{
			float UniformScale = EditScale.X;

			if (ImGui::DragFloat("Scale", &UniformScale, 0.01f, 0.01f, 10.0f))
			{
				EditScale = FVector(UniformScale, UniformScale, UniformScale);
				bScaleChanged = true;
			}
		}
		else
		{
			bScaleChanged |= ImGui::DragFloat3("Scale", &EditScale.X, 0.1f);
		}

		ImGui::Checkbox("Uniform Scale", &bUniformScale);

		SelectedActor->SetUniformScale(bUniformScale);
	}
	else
	{
		ImGui::TextUnformatted("Select Actor For Indicating");
	}

	ImGui::Separator();

#if IS_OBJ_VIEWER
	// jft Object Viewer 출력
	if (ObjectPreview)
	{
		ImGui::Text("Object Viewer Controls");

		// Scale 조작
		FVector previewScale = ObjectPreview->GetComponentScale();
		if (ImGui::DragFloat3("Preview Scale", &previewScale.X, 0.01f, 0.01f, 10.0f))
		{
			ObjectPreview->SetComponentScale(previewScale);
		}

		// Rotation 조작
		FVector previewRotation = ObjectPreview->GetComponentRotation();
		if (ImGui::DragFloat3("Preview Rotation", &previewRotation.X, 0.5f, -180.0f, 180.0f))
		{
			ObjectPreview->SetComponentRotation(previewRotation);
		}

		// Camera 위치 조작
		FVector previewCamLocation = ObjectPreview->GetCameraLocation();
		if (ImGui::DragFloat3("Preview Camera", &previewCamLocation.X, 0.05f))
		{
			ObjectPreview->SetCameraLocation(previewCamLocation);
		}

		ImGui::Separator();
	}

	ID3D11ShaderResourceView* objViewerSRV = URenderer::GetInstance().GetDeviceResources()->GetObjectViewerSRV();
	if (objViewerSRV)
	{
		ImVec2 avail = ImGui::GetContentRegionAvail(); // 현재 패널에 남은 공간
		float size = std::min(avail.x, avail.y);       // 정사각형 유지
		ImGui::Image((ImTextureID)objViewerSRV, ImVec2(size, size));
	}
	else
	{
		ImGui::Text("No Object Viewer Render Target");
	}
#endif
}

/**
 * @brief Render에서 체크된 내용으로 인해 이후 변경되어야 할 내용이 있다면 Change 처리
 */
void UTargetActorTransformWidget::PostProcess()
{
	if (bPositionChanged || bRotationChanged || bScaleChanged)
	{
		ApplyTransformToActor();
	}
}

void UTargetActorTransformWidget::UpdateTransformFromActor()
{
	if (SelectedActor)
	{
		EditLocation = SelectedActor->GetActorLocation();
		EditRotation = SelectedActor->GetActorRotation();
		EditScale = SelectedActor->GetActorScale3D();
	}
}

void UTargetActorTransformWidget::ApplyTransformToActor() const
{
	if (SelectedActor)
	{
		SelectedActor->SetActorLocation(EditLocation);
		SelectedActor->SetActorRotation(EditRotation);
		SelectedActor->SetActorScale3D(EditScale);
	}
}

#if IS_OBJ_VIEWER
void UTargetActorTransformWidget::SetObjectViewer(UObjectPreviewScene* InObjectPreview)
{
	ObjectPreview = InObjectPreview;
}
#endif
