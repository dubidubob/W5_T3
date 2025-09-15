#include "pch.h"
#include "Editor/Editor.h"

#include "Editor/Camera.h"
#include "Editor/Gizmo.h"
#include "Editor/Grid.h"
#include "Editor/Axis.h"
#include "Editor/ObjectPicker.h"
#include "Render/Renderer/Renderer.h"
#include "Render/Renderer/LineBatchRenderer.h"
#include "Manager/Level/LevelManager.h"
#include "Manager/UI/UIManager.h"
#include "Manager/Input/InputManager.h"
#include "Mesh/Actor.h"
#include "Level/Level.h"
#include "Render/UI/Widget/CameraControlWidget.h"
#include "Render/UI/Widget/ViewSettingsWidget.h"

IMPLEMENT_CLASS(UEditor, UObject)

UEditor::UEditor()
{
	ObjectPicker.SetCamera(&Camera);

	// Set Camera to Control Panel
	auto& UIManager = UUIManager::GetInstance();
	UCameraControlWidget* CameraControlWidget =
		Cast<UCameraControlWidget>(UIManager.FindWidget("UCameraControlWidget"));
	CameraControlWidget->SetCamera(&Camera);
	UViewSettingsWidget* ViewSettingsWidget =
		Cast<UViewSettingsWidget>(UIManager.FindWidget("UViewSettingsWidget"));
	ViewSettingsWidget->SetGrid(&Grid);

};

UEditor::~UEditor() = default;

void UEditor::Update()
{
	auto& Renderer = URenderer::GetInstance();
	Camera.Update();

	ProcessMouseInput(ULevelManager::GetInstance().GetCurrentLevel());
	ProcessKeyboardInput();

	Renderer.UpdateConstant(Camera.GetFViewProjConstants());
}
void UEditor::RenderEditor()
{
	Grid.RenderGrid();
	Axis.Render();
	Gizmo.RenderGizmo(ULevelManager::GetInstance().GetCurrentLevel()->GetSelectedActor(), Camera.GetLocation());
}

void UEditor::RenderEditorBatched()
{
	ULineBatchRenderer& LineBatch = ULineBatchRenderer::GetInstance();

	/** 모든 라인 렌더링을 하나의 배치로 통합 */
	LineBatch.BeginBatch();
	{
		/** Grid 라인들 추가 */
		Grid.AddToLineBatch(LineBatch);

		/** Axis 라인들 추가 */
		Axis.AddToLineBatch(LineBatch);

		// /** AABB 라인들 추가 */
		// AABB.AddToLineBatch(LineBatch);

		/** Gizmo 라인들 추가 (오브젝트가 선택된 경우) */
		AActor* SelectedActor = ULevelManager::GetInstance().GetCurrentLevel()->GetSelectedActor();
		if (SelectedActor)
		{
			/** Gizmo는 현재 RenderGizmo를 통해 렌더링되므로 따로 처리 */
			/** 추후 Gizmo도 배칭 지원하도록 수정 가능 */
		}
	}
	/** 1회 드로우콜로 모든 라인 렌더링 */
	LineBatch.FlushBatch();

	/** Gizmo는 별도로 렌더링 (기존 방식 유지) */
	Gizmo.RenderGizmo(ULevelManager::GetInstance().GetCurrentLevel()->GetSelectedActor(), Camera.GetLocation());
}


void UEditor::ProcessKeyboardInput()
{
	const UInputManager& InputManager = UInputManager::GetInstance();
	auto& Renderer = URenderer::GetInstance();

	// Alt+C로 바운딩 박스 토글
	if (InputManager.IsKeyDown(EKeyInput::Alt) && InputManager.IsKeyPressed(EKeyInput::C))
	{
		Renderer.ToggleShowFlag(EEngineShowFlags::SF_Bounds);
	}
}

void UEditor::ProcessMouseInput(ULevel* InLevel)
{
	FRay WorldRay;
	const UInputManager& InputManager = UInputManager::GetInstance();
	FVector MousePositionNdc = InputManager.GetMouseNDCPosition();

	static EGizmoDirection PreviousGizmoDirection = EGizmoDirection::None;
	AActor* ActorPicked = InLevel->GetSelectedActor();
	FVector CollisionPoint;
	float ActorDistance = -1;

	//로컬 기즈모, 쿼터니언 구현 후 사용
	/**if (InputManager.IsKeyPressed(EKeyInput::Tab))
	{
		if (Gizmo.IsWorld())
			Gizmo.SetLocal();
		else
			Gizmo.SetWorld();
	}*/
	if (InputManager.IsKeyPressed(EKeyInput::Space))
	{
		Gizmo.ChangeGizmoMode();
	}
	if (InputManager.IsKeyReleased(EKeyInput::MouseLeft))
	{
		Gizmo.EndDrag();
	}
	WorldRay = Camera.ConvertToWorldRay(MousePositionNdc.X, MousePositionNdc.Y);

	if(Gizmo.IsDragging() && Gizmo.GetSelectedActor())
	{
		switch (Gizmo.GetGizmoMode())
		{
		case EGizmoMode::Translate:
		{
			FVector GizmoDragLocation = GetGizmoDragLocation(WorldRay);
			Gizmo.SetLocation(GizmoDragLocation); break;
		}
		case EGizmoMode::Rotate:
		{
			FVector GizmoDragRotation = GetGizmoDragRotation(WorldRay);
			Gizmo.SetActorRotation(GizmoDragRotation); break;
		}
		case EGizmoMode::Scale:
		{
			FVector GizmoDragScale = GetGizmoDragScale(WorldRay);
			Gizmo.SetActorScale(GizmoDragScale);
		}
		}

	}
	else
	{
		/** 기즈모가 출력되고있음. 레이캐스팅을 계속 해야함. */
		if (InLevel->GetSelectedActor())
		{
			ObjectPicker.PickGizmo(WorldRay, Gizmo, CollisionPoint);
		}
		else
		{
			Gizmo.SetGizmoDirection(EGizmoDirection::None);
		}
		if (!ImGui::GetIO().WantCaptureMouse && InputManager.IsKeyPressed(EKeyInput::MouseLeft))
		{
			TArray<UPrimitiveComponent*> Candidate = FindCandidatePrimitives(InLevel);

			UPrimitiveComponent* PrimitiveCollided= ObjectPicker.PickPrimitive( WorldRay, Candidate, &ActorDistance);

			if (PrimitiveCollided)
				ActorPicked = PrimitiveCollided->GetOwner();
			else
				ActorPicked = nullptr;
		}

		/** 기즈모에 호버링되거나 클릭되지 않았을 때. Actor 업데이트해줌. */
		if (Gizmo.GetGizmoDirection() == EGizmoDirection::None)
		{
			InLevel->SetSelectedActor(ActorPicked);
			if (PreviousGizmoDirection != EGizmoDirection::None)
			{
				Gizmo.OnMouseRelease(PreviousGizmoDirection);
			}
		}
		/** 기즈모가 선택되었을 때. Actor가 선택되지 않으면 기즈모도 선택되지 않으므로 이미 Actor가 선택된 상황. */
		/** SelectedActor를 update하지 않고 마우스 인풋에 따라 hovering or drag */
		else
		{
			PreviousGizmoDirection = Gizmo.GetGizmoDirection();
			/** 드래그 */
			if (InputManager.IsKeyPressed(EKeyInput::MouseLeft))
			{
				Gizmo.OnMouseDragStart(CollisionPoint);
			}
			else
			{
				Gizmo.OnMouseHovering();
			}
		}
	}
}

TArray<UPrimitiveComponent*> UEditor::FindCandidatePrimitives(ULevel* InLevel)
{
	TArray<UPrimitiveComponent*> Candidate;

	for (AActor* Actor : InLevel->GetLevelActors())
	{
		for (auto& ActorComponent : Actor->GetOwnedComponents())
		{
			UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(ActorComponent);
			if (Primitive)
			{
				Candidate.push_back(Primitive);
			}
		}
	}

	return Candidate;
}

FVector UEditor::GetGizmoDragLocation(FRay& WorldRay)
{
	FVector MouseWorld;
	FVector PlaneOrigin{ Gizmo.GetGizmoLocation() };
	FVector GizmoAxis = Gizmo.GetGizmoAxis();

	//로컬 기즈모, 쿼터니언 구현 후 사용
	//if (!Gizmo.IsWorld())	//local
	//{
	//	FVector4 GizmoAxis4{ GizmoAxis.X,GizmoAxis.Y ,GizmoAxis.Z, 0.0f };
	//	GizmoAxis = GizmoAxis4 * FMatrix::RotationMatrix(Gizmo.GetActorRotation());
	//}

	if (ObjectPicker.IsRayCollideWithPlane(WorldRay, PlaneOrigin, Camera.CalculatePlaneNormal(GizmoAxis).Cross(GizmoAxis), MouseWorld))
	{
		FVector MouseDistance = MouseWorld - Gizmo.GetDragStartMouseLocation();
		return Gizmo.GetDragStartActorLocation() + GizmoAxis * MouseDistance.Dot(GizmoAxis);
	}
	else
		return Gizmo.GetGizmoLocation();

}

FVector UEditor::GetGizmoDragRotation(FRay& WorldRay)
{

	FVector MouseWorld;
	FVector PlaneOrigin{ Gizmo.GetGizmoLocation() };
	FVector GizmoAxis = Gizmo.GetGizmoAxis();

	//로컬 기즈모, 쿼터니언 구현 후 사용
	//if (!Gizmo.IsWorld())	//local
	//{
	//	FVector4 GizmoAxis4{ GizmoAxis.X,GizmoAxis.Y ,GizmoAxis.Z, 0.0f };
	//	GizmoAxis = GizmoAxis4 * FMatrix::RotationMatrix(Gizmo.GetActorRotation());
	//}

	if (ObjectPicker.IsRayCollideWithPlane(WorldRay, PlaneOrigin, GizmoAxis, MouseWorld))
	{
		FVector PlaneOriginToMouse = MouseWorld - PlaneOrigin;
		FVector PlaneOriginToMouseStart = Gizmo.GetDragStartMouseLocation() - PlaneOrigin;
		PlaneOriginToMouse.Normalize();
		PlaneOriginToMouseStart.Normalize();
		float Angle = acosf((PlaneOriginToMouseStart).Dot(PlaneOriginToMouse));	//플레인 중심부터 마우스까지 벡터 이용해서 회전각도 구하기
		if ((PlaneOriginToMouse.Cross(PlaneOriginToMouseStart)).Dot(GizmoAxis) < 0) // 회전축 구하기
		{
			Angle = -Angle;
		}
		return Gizmo.GetDragStartActorRotation() + GizmoAxis *FVector::GetRadianToDegree(Angle);

	}
	else
		return Gizmo.GetActorRotation();

}

FVector UEditor::GetGizmoDragScale(FRay& WorldRay)
{
	FVector MouseWorld;
	FVector PlaneOrigin{ Gizmo.GetGizmoLocation() };
	FVector GizmoAxis = Gizmo.GetGizmoAxis();

	//로컬 기즈모, 쿼터니언 구현 후 사용
	//if (!Gizmo.IsWorld())	//local
	//{
	//	FVector4 GizmoAxis4{ GizmoAxis.X,GizmoAxis.Y ,GizmoAxis.Z, 0.0f };
	//	GizmoAxis = GizmoAxis4 * FMatrix::RotationMatrix(Gizmo.GetActorRotation());
	//}

	if (ObjectPicker.IsRayCollideWithPlane(WorldRay, PlaneOrigin, Camera.CalculatePlaneNormal(GizmoAxis).Cross(GizmoAxis), MouseWorld))
	{
		FVector PlaneOriginToMouse = MouseWorld - PlaneOrigin;
		FVector PlaneOriginToMouseStart = Gizmo.GetDragStartMouseLocation() - PlaneOrigin;
		float DragStartAxisDistance = PlaneOriginToMouseStart.Dot(GizmoAxis);
		float DragAxisDistance = PlaneOriginToMouse.Dot(GizmoAxis);
		float ScaleFactor = 1.0f;
		if (abs(DragStartAxisDistance) > 0.1f)
		{
			ScaleFactor = DragAxisDistance / DragStartAxisDistance;
		}

		FVector DragStartScale = Gizmo.GetDragStartActorScale();
		if (ScaleFactor > MinScale)
		{
			if (Gizmo.GetSelectedActor()->IsUniformScale())
			{
				float UniformValue = DragStartScale.Dot(GizmoAxis);
				return FVector(UniformValue, UniformValue, UniformValue) + FVector(1, 1, 1) * (ScaleFactor - 1)* UniformValue;
			}
			else
				return DragStartScale + GizmoAxis * (ScaleFactor - 1) * DragStartScale.Dot(GizmoAxis);
		}
		else
			return Gizmo.GetActorScale();
	}
	else
		return Gizmo.GetActorScale();
}
