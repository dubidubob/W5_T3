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
#include "Render/UI/Widget/TargetActorTransformWidget.h"
#include "Mesh/StaticMeshComponent.h"
#include "Manager/Viewport/ViewportManager.h"
#include "Utility/ObjectPreviewScene.h"
#include "Slate/Viewport.h"

IMPLEMENT_CLASS(UEditor, UObject)

UEditor::UEditor()
{
	Camera = NewObject<UCamera>();
	ObjectPicker = NewObject<UObjectPicker>();
	ViewportManager = NewObject<UViewportManager>();
	ObjPreview = NewObject<UObjectPreviewScene>();

	Gizmo = NewObject<UGizmo>();
	Grid = NewObject<UGrid>();
	Axis = NewObject<UAxis>();

	ObjectPicker->SetCamera(Camera);
	ViewportManager->Initialize(Camera);

	// Set Camera to Control Panel
	auto& UIManager = UUIManager::GetInstance();

	UTargetActorTransformWidget* TargetActorTransformWidget =
		Cast<UTargetActorTransformWidget>(UIManager.FindWidget("UTargetActorTransformWidget"));
	TargetActorTransformWidget->SetObjectViewer(ObjPreview);

	UCameraControlWidget* CameraControlWidget =
		Cast<UCameraControlWidget>(UIManager.FindWidget("UCameraControlWidget"));
	CameraControlWidget->SetCamera(Camera);

	UViewSettingsWidget* ViewSettingsWidget =
		Cast<UViewSettingsWidget>(UIManager.FindWidget("UViewSettingsWidget"));
	ViewSettingsWidget->SetGrid(Grid);
	ViewSettingsWidget->SetRenderer(&URenderer::GetInstance());
	ViewSettingsWidget->SetViewportManager(ViewportManager);
};

UEditor::~UEditor()
{
	delete Camera;
	delete ObjectPicker;
	delete ViewportManager;
	delete ObjPreview;
	delete Gizmo;
	delete Grid;
	delete Axis;
}

void UEditor::Update()
{
	Camera->Update(ViewportManager->GetIsWindowDivided());
	ViewportManager->Update();

	ProcessMouseInput(ULevelManager::GetInstance().GetCurrentLevel());
	ProcessKeyboardInput();

	auto& Renderer = URenderer::GetInstance();
	Renderer.UpdateViewProjConstants(Camera->GetFViewProjConstants());
}

const FVector& UEditor::GetCameraLocation()
{
	return Camera->GetLocation();
}

UCamera* UEditor::GetCamera()
{
	return Camera;
}

void UEditor::RenderEditorBatched(int Idx)
{
	ULineBatchRenderer& LineBatch = ULineBatchRenderer::GetInstance();

	/** 모든 라인 렌더링을 하나의 배치로 통합 */
	LineBatch.BeginBatch();
	{
		/** Grid 라인들 추가 */
		Grid->AddToLineBatch(LineBatch);

		/** Axis 라인들 추가 */
		Axis->AddToLineBatch(LineBatch);

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
					UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(Prim);
					if (StaticMeshComponent)
					{
						FAABB Bounds = StaticMeshComponent->GetWorldBounds();
						//FAABB Bounds = Prim->GetWorldBounds();
						if (!Bounds.IsValid()) { continue; }
						LineBatch.AddAABB(Bounds.Min, Bounds.Max, FVector4(0, 1, 0, 1));

					}
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
	if (ViewportManager->IsSelectedWindowIdx(Idx)||!ViewportManager->GetIsWindowDivided())
	{
		float CacheScale;
		Gizmo->RenderGizmo(ULevelManager::GetInstance().GetCurrentLevel()->GetSelectedActor(), Camera->GetLocation(), true, 0, CacheScale);

		if (ViewportManager->GetIsWindowDivided() && CacheScale > 0 && CacheScale < 2000) /*jft 쓰레기값 방지 부등호*/
		{
			ViewportManager->GetViewportInfo(Idx)->GizmoScale = CacheScale;
			ViewportManager->InitializeGizmoScale(CacheScale);
		}
	}
	else
	{

		float CachedScale = ViewportManager->GetViewportInfo(Idx)->GizmoScale;
		if(CachedScale > 0 && CachedScale<2000)
			Gizmo->RenderGizmo(ULevelManager::GetInstance().GetCurrentLevel()->GetSelectedActor(), Camera->GetLocation(), false, CachedScale, CachedScale);
	}
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
		Gizmo->ChangeGizmoMode();
	}

	// Gizmo 표시 중 Tab: 월드→로컬 토글 (기본: 토글, 최초 누르면 로컬 보장)
	if (InputManager.IsKeyPressed(EKeyInput::Tab))
	{
		ULevel* Level = ULevelManager::GetInstance().GetCurrentLevel();
		if (Level && Level->GetSelectedActor())
		{
			if (Gizmo->IsWorld())
			{
				Gizmo->SetLocal();
			}
			else
			{
				Gizmo->SetWorld();
			}
		}
	}
}

void UEditor::ProcessMouseInput(ULevel* InLevel)
{
	/*Make Input*/
	const UInputManager& InputManager = UInputManager::GetInstance();
	FVector2 MousePositionNdc = InputManager.GetMouseNDCPosition();
	// If Multi Viewport Mode -> Adjust MousePosition & FRay & Cam
	MousePositionNdc = ViewportManager->UpdateMouseInputNdcInViewports(MousePositionNdc);

	/*Calculate Gizmo*/
	// 월드 레이 먼저 계산 (릴리즈 커밋에 사용)
	const bool bMouseButtonStateChanged = InputManager.IsKeyPressed(EKeyInput::MouseLeft) || InputManager.IsKeyReleased(EKeyInput::MouseLeft);
	const bool bMousePositionChanged = (LastMousePosition - MousePositionNdc).Length() > 1e-6f;
	const bool  bIsMouseOverViewport = !ImGui::GetIO().WantCaptureMouse;
	if (bMouseButtonStateChanged || (bIsMouseOverViewport&& bMousePositionChanged))
	{
		LastMousePosition = MousePositionNdc;
		FRay WorldRay = Camera->ConvertToWorldRay(MousePositionNdc.X, MousePositionNdc.Y);
		HandleGizmo(InLevel, WorldRay);
	}	
}

void UEditor::HandleGizmo(ULevel* InLevel, FRay InWorldRay)
{
	static EGizmoDirection PreviousGizmoDirection = EGizmoDirection::None;
	AActor* ActorPicked = InLevel->GetSelectedActor();

	float ActorDistance = -1;

	const UInputManager& InputManager = UInputManager::GetInstance();
	if (InputManager.IsKeyReleased(EKeyInput::MouseLeft))
	{
		Gizmo->EndDrag();
	}

	if (Gizmo->IsDragging() && Gizmo->GetSelectedActor())
	{
		switch (Gizmo->GetGizmoMode())
		{
		case EGizmoMode::Translate:
		{
			FVector GizmoDragLocation = GetGizmoDragLocation(InWorldRay);
			Gizmo->SetLocation(GizmoDragLocation);
			break;
		}
		case EGizmoMode::Rotate:
		{
			FQuat GizmoDragRotation = GetGizmoDragRotationQuat(InWorldRay);
			Gizmo->SetActorRotation(GizmoDragRotation);
			break;
		}
		case EGizmoMode::Scale:
		{
			FVector GizmoDragScale = GetGizmoDragScale(InWorldRay);
			Gizmo->SetActorScale(GizmoDragScale);
		}
		}
	}
	else
	{
		FVector CollisionPoint;
		/** 기즈모가 출력되고있음. 레이캐스팅을 계속 해야함. */
		if (InLevel->GetSelectedActor())
		{
			ObjectPicker->PickGizmo(InWorldRay, Gizmo, CollisionPoint);
		}
		else
		{
			Gizmo->SetGizmoDirection(EGizmoDirection::None);
		}
		if (!ImGui::GetIO().WantCaptureMouse && InputManager.IsKeyPressed(EKeyInput::MouseLeft))
		{
			TArray<UPrimitiveComponent*> Candidate = FindCandidatePrimitives(InLevel);

			UPrimitiveComponent* PrimitiveCollided = ObjectPicker->PickPrimitive(InWorldRay, Candidate, &ActorDistance);

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
		if (Gizmo->GetGizmoDirection() == EGizmoDirection::None)
		{
			/* todo : 매번 같은 애 다시 Update 해주고 있음 */
			InLevel->SetSelectedActor(ActorPicked);
			ObjPreview->UpdatePreviewFromActor(ActorPicked);
			if (PreviousGizmoDirection != EGizmoDirection::None)
			{
				Gizmo->OnMouseRelease(PreviousGizmoDirection);
			}
		}
		/** 기즈모가 선택되었을 때. Actor가 선택되지 않으면 기즈모도 선택되지 않으므로 이미 Actor가 선택된 상황. */
		/** SelectedActor를 update하지 않고 마우스 인풋에 따라 hovering or drag */
		else
		{
			PreviousGizmoDirection = Gizmo->GetGizmoDirection();
			/** 드래그 */
			if (InputManager.IsKeyPressed(EKeyInput::MouseLeft))
			{
				Gizmo->OnMouseDragStart(CollisionPoint);
			}
			else
			{
				Gizmo->OnMouseHovering();
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
	FVector PlaneOrigin{ Gizmo->GetGizmoLocation() };
	const FVector AxisLocal = Gizmo->GetGizmoAxis(); // (1,0,0) or (0,1,0) or (0,0,1)
	FVector GizmoAxis = AxisLocal;

	// 로컬 모드일 때는 드래그 시작 시점의 로컬 축을 월드로 변환 해줌
	if (!Gizmo->IsWorld())
	{
		const FQuat QuatDragStart = Gizmo->GetDragStartActorRotationQuat();
		GizmoAxis = QuatDragStart.RotateVector(AxisLocal);
	}

	// 드래그 평면의 법선 벡터 계산
	FVector PlaneNormal;
	FVector CameraForward = Camera->GetForward();

	// 카메라 전방 벡터와 기즈모 축이 거의 평행한지 확인 (외적 결과가 0이 되는 상황 방지)
	PlaneNormal = GizmoAxis.Cross(Camera->GetUp());

	if (PlaneNormal.Length() < 0.001f)
	{
		PlaneNormal = (GizmoAxis.Cross(Camera->GetRight()));
	}
	PlaneNormal.Normalize();


	if (ObjectPicker->IsRayCollideWithPlane(WorldRay, PlaneOrigin, PlaneNormal, MouseWorld))
	{
		FVector MouseDistance = MouseWorld - Gizmo->GetDragStartMouseLocation();
		return Gizmo->GetDragStartActorLocation() + GizmoAxis * MouseDistance.Dot(GizmoAxis);
	}
	return Gizmo->GetGizmoLocation();
}

FVector UEditor::GetGizmoDragRotation(const FRay& WorldRay)
{
	FVector MouseWorld;
	FVector PlaneOrigin{Gizmo->GetGizmoLocation()};
	FVector GizmoAxis = Gizmo->GetGizmoAxis();

	if (ObjectPicker->IsRayCollideWithPlane(WorldRay, PlaneOrigin, GizmoAxis, MouseWorld))
	{
		FVector PlaneOriginToMouse = MouseWorld - PlaneOrigin;
		FVector PlaneOriginToMouseStart = Gizmo->GetDragStartMouseLocation() - PlaneOrigin;
		PlaneOriginToMouse.Normalize();
		PlaneOriginToMouseStart.Normalize();
		float Angle = acosf((PlaneOriginToMouseStart).Dot(PlaneOriginToMouse)); //플레인 중심부터 마우스까지 벡터 이용해서 회전각도 구하기
		if ((PlaneOriginToMouse.Cross(PlaneOriginToMouseStart)).Dot(GizmoAxis) < 0) // 회전축 구하기
		{
			Angle = -Angle;
		}
		return Gizmo->GetDragStartActorRotation() + GizmoAxis * FVector::GetRadianToDegree(Angle);
	}
	return Gizmo->GetActorRotation();
}

FQuat UEditor::GetGizmoDragRotationQuat(const FRay& WorldRay)
{
	FVector MouseWorld;
	FVector PlaneOrigin{Gizmo->GetGizmoLocation()};
	const FVector AxisLocal = Gizmo->GetGizmoAxis(); // (1,0,0) or (0,1,0) or (0,0,1)
	FVector AxisWorld = AxisLocal;


	// Local gizmo: 드래그 시작 시점의 로컬 축을 월드로 변환해 고정
	if (!Gizmo->IsWorld())
	{
		const FQuat QuatDragStart = Gizmo->GetDragStartActorRotationQuat();
		AxisWorld = QuatDragStart.RotateVector(AxisLocal);
	}

	if (ObjectPicker->IsRayCollideWithPlane(WorldRay, PlaneOrigin, AxisWorld, MouseWorld))
	{
		FVector V1 = Gizmo->GetDragStartMouseLocation() - PlaneOrigin;
		FVector V2 = MouseWorld - PlaneOrigin;
		V1.Normalize();
		V2.Normalize();
		float Angle = acosf(clamp(V1.Dot(V2), -1.0f, 1.0f));
		if (V2.Cross(V1).Dot(AxisWorld) < 0)
		{
			Angle = -Angle;
		}

		// 드래그 시작 시점의 오리엔테이션
		const FQuat QuatDragStart = Gizmo->GetDragStartActorRotationQuat();
		// 델타 회전 쿼터니언 구성
		// 월드: 월드축 기준 델타, 좌곱
		// 로컬: 로컬축 기준 델타, 우곱
		if (Gizmo->IsWorld())
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
	return Gizmo->GetActorRotationQuat();
}

FVector UEditor::GetGizmoDragScale(const FRay& WorldRay)
{
	FVector MouseWorld;
	FVector PlaneOrigin{Gizmo->GetGizmoLocation()};
	FVector GizmoAxis = Gizmo->GetGizmoAxis();

	// 스케일은 항상 로컬 기준으로 작동: 시작 시점 로컬축을 월드로 고정
	GizmoAxis = Gizmo->GetDragStartActorRotationQuat().RotateVector(GizmoAxis);

	if (ObjectPicker->IsRayCollideWithPlane(WorldRay, PlaneOrigin,
	                                       Camera->CalculatePlaneNormal(GizmoAxis).Cross(GizmoAxis), MouseWorld))
	{
		FVector PlaneOriginToMouse = MouseWorld - PlaneOrigin;
		FVector PlaneOriginToMouseStart = Gizmo->GetDragStartMouseLocation() - PlaneOrigin;
		float DragStartAxisDistance = PlaneOriginToMouseStart.Dot(GizmoAxis);
		float DragAxisDistance = PlaneOriginToMouse.Dot(GizmoAxis);
		float ScaleFactor = 1.0f;
		if (abs(DragStartAxisDistance) > 0.1f)
		{
			ScaleFactor = DragAxisDistance / DragStartAxisDistance;
		}

		FVector DragStartScale = Gizmo->GetDragStartActorScale();
		if (ScaleFactor > MinScale)
		{
			// Uniform이면 모든 축 동일 비율 스케일
			if (Gizmo->GetSelectedActor()->IsUniformScale())
			{
				return {
					DragStartScale.X * ScaleFactor,
					DragStartScale.Y * ScaleFactor,
					DragStartScale.Z * ScaleFactor
				};
			}

			// 비균일: 선택 축의 성분만 스케일 변경
			FVector NewScale = DragStartScale;
			switch (Gizmo->GetGizmoDirection())
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
		return Gizmo->GetActorScale();
	}
	return Gizmo->GetActorScale();
}
