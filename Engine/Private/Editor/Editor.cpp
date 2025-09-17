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

// void UEditor::RenderEditor()
// {
// 	Grid.RenderGrid();
// 	Axis.Render();
// 	Gizmo.RenderGizmo(ULevelManager::GetInstance().GetCurrentLevel()->GetSelectedActor(), Camera.GetLocation());
// }

const FVector& UEditor::GetCameraLocation()
{
	return Camera.GetLocation();
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

		/** AABB 라인들 추가 (Min/Max 입력 기반, 인스턴싱) */
		URenderer& Renderer = URenderer::GetInstance();
		if (Renderer.IsShowFlagEnabled(EEngineShowFlags::SF_Bounds))
		{
			ULevel* Level = ULevelManager::GetInstance().GetCurrentLevel();
			if (Level)
			{
				const TArray<UPrimitiveComponent*>& Primitives = Level->GetLevelPrimitiveComponents();
				for (UPrimitiveComponent* Prim : Primitives)
				{
					if (!Prim) { continue; }
					FAABB Bounds = Prim->GetWorldBounds();
					if (!Bounds.IsValid()) { continue; }
					LineBatch.AddAABB(Bounds.Min, Bounds.Max, FVector4(0, 1, 0, 1));
				}
			}
		}

		/** Gizmo 라인들 추가 (오브젝트가 선택된 경우) */
		if (AActor* SelectedActor = ULevelManager::GetInstance().GetCurrentLevel()->GetSelectedActor())
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
	if (InputManager.IsKeyPressed(EKeyInput::Space))
	{
		Gizmo.ChangeGizmoMode();
	}

	// Gizmo 표시 중 Tab: 월드→로컬 토글 (기본: 토글, 최초 누르면 로컬 보장)
	if (InputManager.IsKeyPressed(EKeyInput::Tab))
	{
		ULevel* Level = ULevelManager::GetInstance().GetCurrentLevel();
		if (Level && Level->GetSelectedActor())
		{
			if (Gizmo.IsWorld())
			{
				Gizmo.SetLocal();
			}
			else
			{
				Gizmo.SetWorld();
			}
		}
	}
}

void UEditor::ProcessMouseInput(ULevel* InLevel)
{
	const UInputManager& InputManager = UInputManager::GetInstance();
	FVector MousePositionNdc = InputManager.GetMouseNDCPosition();

	static EGizmoDirection PreviousGizmoDirection = EGizmoDirection::None;
	AActor* ActorPicked = InLevel->GetSelectedActor();
	float ActorDistance = -1;

	// Tab 처리는 키보드 입력 루틴에서 수행

	// 월드 레이 먼저 계산 (릴리즈 커밋에 사용)
	FRay WorldRay = Camera.ConvertToWorldRay(MousePositionNdc.X, MousePositionNdc.Y);

	if (InputManager.IsKeyReleased(EKeyInput::MouseLeft))
	{
		// 회전 모드에서 릴리즈 시 마지막 각도 커밋 (로컬/월드 동일)
		if (Gizmo.IsDragging() && Gizmo.GetSelectedActor() && Gizmo.GetGizmoMode() == EGizmoMode::Rotate)
		{
			FQuat FinalQuat = GetGizmoDragRotationQuat(WorldRay);
			Gizmo.SetActorRotation(FinalQuat);
		}
		Gizmo.EndDrag();
	}

	if (Gizmo.IsDragging() && Gizmo.GetSelectedActor())
	{
		switch (Gizmo.GetGizmoMode())
		{
		case EGizmoMode::Translate:
			{
				FVector GizmoDragLocation = GetGizmoDragLocation(WorldRay);
				Gizmo.SetLocation(GizmoDragLocation);
				break;
			}
		case EGizmoMode::Rotate:
			{
				FQuat GizmoDragRotation = GetGizmoDragRotationQuat(WorldRay);
				Gizmo.SetActorRotation(GizmoDragRotation);
				break;
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
		FVector CollisionPoint;
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

			UPrimitiveComponent* PrimitiveCollided = ObjectPicker.PickPrimitive(WorldRay, Candidate, &ActorDistance);

			if (PrimitiveCollided)
			{
				ActorPicked = PrimitiveCollided->GetOwner();
			}
			else
			{
				ActorPicked = nullptr;
			}
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

FVector UEditor::GetGizmoDragLocation(const FRay& WorldRay)
{
	FVector MouseWorld;
	FVector PlaneOrigin{Gizmo.GetGizmoLocation()};
	const FVector AxisLocal = Gizmo.GetGizmoAxis(); // (1,0,0) or (0,1,0) or (0,0,1)
	FVector GizmoAxis = AxisLocal;

	// 로컬 모드일 때는 드래그 시작 시점의 로컬 축을 월드로 변환 해줌
	if (!Gizmo.IsWorld())
	{
		const FQuat QuatDragStart = Gizmo.GetDragStartActorRotationQuat();
		GizmoAxis = QuatDragStart.RotateVector(AxisLocal);
	}

	if (ObjectPicker.IsRayCollideWithPlane(WorldRay, PlaneOrigin,
	                                       Camera.CalculatePlaneNormal(GizmoAxis).Cross(GizmoAxis), MouseWorld))
	{
		FVector MouseDistance = MouseWorld - Gizmo.GetDragStartMouseLocation();
		return Gizmo.GetDragStartActorLocation() + GizmoAxis * MouseDistance.Dot(GizmoAxis);
	}
	return Gizmo.GetGizmoLocation();
}

FVector UEditor::GetGizmoDragRotation(const FRay& WorldRay)
{
	FVector MouseWorld;
	FVector PlaneOrigin{Gizmo.GetGizmoLocation()};
	FVector GizmoAxis = Gizmo.GetGizmoAxis();

	if (ObjectPicker.IsRayCollideWithPlane(WorldRay, PlaneOrigin, GizmoAxis, MouseWorld))
	{
		FVector PlaneOriginToMouse = MouseWorld - PlaneOrigin;
		FVector PlaneOriginToMouseStart = Gizmo.GetDragStartMouseLocation() - PlaneOrigin;
		PlaneOriginToMouse.Normalize();
		PlaneOriginToMouseStart.Normalize();
		float Angle = acosf((PlaneOriginToMouseStart).Dot(PlaneOriginToMouse)); //플레인 중심부터 마우스까지 벡터 이용해서 회전각도 구하기
		if ((PlaneOriginToMouse.Cross(PlaneOriginToMouseStart)).Dot(GizmoAxis) < 0) // 회전축 구하기
		{
			Angle = -Angle;
		}
		return Gizmo.GetDragStartActorRotation() + GizmoAxis * FVector::GetRadianToDegree(Angle);
	}
	return Gizmo.GetActorRotation();
}

FQuat UEditor::GetGizmoDragRotationQuat(const FRay& WorldRay)
{
	FVector MouseWorld;
	FVector PlaneOrigin{Gizmo.GetGizmoLocation()};
	const FVector AxisLocal = Gizmo.GetGizmoAxis(); // (1,0,0) or (0,1,0) or (0,0,1)
	FVector AxisWorld = AxisLocal;


	// Local gizmo: 드래그 시작 시점의 로컬 축을 월드로 변환해 고정
	if (!Gizmo.IsWorld())
	{
		const FQuat QuatDragStart = Gizmo.GetDragStartActorRotationQuat();
		AxisWorld = QuatDragStart.RotateVector(AxisLocal);
	}

	if (ObjectPicker.IsRayCollideWithPlane(WorldRay, PlaneOrigin, AxisWorld, MouseWorld))
	{
		FVector V1 = Gizmo.GetDragStartMouseLocation() - PlaneOrigin;
		FVector V2 = MouseWorld - PlaneOrigin;
		V1.Normalize();
		V2.Normalize();
		float Angle = acosf(clamp(V1.Dot(V2), -1.0f, 1.0f));
		if (V2.Cross(V1).Dot(AxisWorld) < 0)
		{
			Angle = -Angle;
		}

		// 드래그 시작 시점의 오리엔테이션
		const FQuat QuatDragStart = Gizmo.GetDragStartActorRotationQuat();
		// 델타 회전 쿼터니언 구성
		// 월드: 월드축 기준 델타, 좌곱
		// 로컬: 로컬축 기준 델타, 우곱
		if (Gizmo.IsWorld())
		{
			const FQuat QuatDelta = FQuat::FromAxisAngle(AxisWorld, Angle);
			return QuatDelta * QuatDragStart;
		}
		else
		{
			const FQuat QuatDeltaLocal = FQuat::FromAxisAngle(AxisLocal, Angle);
			return QuatDragStart * QuatDeltaLocal;
		}
	}
	return Gizmo.GetActorRotationQuat();
}

FVector UEditor::GetGizmoDragScale(const FRay& WorldRay)
{
	FVector MouseWorld;
	FVector PlaneOrigin{Gizmo.GetGizmoLocation()};
	FVector GizmoAxis = Gizmo.GetGizmoAxis();

	// 스케일은 항상 로컬 기준으로 작동: 시작 시점 로컬축을 월드로 고정
	GizmoAxis = Gizmo.GetDragStartActorRotationQuat().RotateVector(GizmoAxis);

	if (ObjectPicker.IsRayCollideWithPlane(WorldRay, PlaneOrigin,
	                                       Camera.CalculatePlaneNormal(GizmoAxis).Cross(GizmoAxis), MouseWorld))
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
			// Uniform이면 모든 축 동일 비율 스케일
			if (Gizmo.GetSelectedActor()->IsUniformScale())
			{
				return {
					DragStartScale.X * ScaleFactor,
					DragStartScale.Y * ScaleFactor,
					DragStartScale.Z * ScaleFactor
				};
			}

			// 비균일: 선택 축의 성분만 스케일 변경
			FVector NewScale = DragStartScale;
			switch (Gizmo.GetGizmoDirection())
			{
			case EGizmoDirection::Right:
				NewScale.X = max(DragStartScale.X * ScaleFactor, MinScale);
				break;
			case EGizmoDirection::Up:
				NewScale.Y = max(DragStartScale.Y * ScaleFactor, MinScale);
				break;
			case EGizmoDirection::Forward:
				NewScale.Z = max(DragStartScale.Z * ScaleFactor, MinScale);
				break;
			default: break;
			}
			return NewScale;
		}
		return Gizmo.GetActorScale();
	}
	return Gizmo.GetActorScale();
}
