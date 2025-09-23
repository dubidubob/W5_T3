#include "pch.h"
#include "Editor/Camera.h"
#include "Manager/Viewport/ViewportManager.h"
#include "Manager/Input/InputManager.h"

#include "Slate/VerticalBox.h"
#include "Slate/HorizontalBox.h"
#include "Slate/SplitterV.h"
#include "Slate/SplitterH.h"
#include "Slate/Viewport.h"
IMPLEMENT_CLASS(UViewportManager, UObject)

UViewportManager::UViewportManager()
{
}
UViewportManager::~UViewportManager()
{
	int32 CameraCnt = sizeof(Viewports) / sizeof(Viewports[0]);
	for (int32 Idx = 0; Idx < CameraCnt; Idx++)
	{
		if (Viewports[Idx])
		{
			delete Viewports[Idx]->GetViewportInfo()->Camera;
		}
	}
	delete RootWindow;
}

void UViewportManager::Initialize(UCamera* InCamera)
{
	InitializeSplitter();
	InitializeSubCamera(InCamera);
}
void UViewportManager::InitializeSplitter()
{
	/* Splitter 초기화*/
	// Splitter Thickness > NDC
	const float SplitterNdcWidth = 0.015f;
	const float SplitterNdcHeight = 0.015f;

	FRect RootRect = { -1.0f, -1.0f, 2.0f, 2.0f };
	SVerticalBox* RootBox = new SVerticalBox(); Windows.Add(RootBox);
	RootBox->SetRect(RootRect);

		// 상하 분할 (SplitterH)
		SSplitterH* SplitterH = new SSplitterH(); Windows.Add(SplitterH);
		SplitterH->SetRect(FRect(-1.0f, -0.5f * SplitterNdcHeight, 2.0f, SplitterNdcHeight));

		// 상단 박스 (TopBox)
		SHorizontalBox* TopBox = new SHorizontalBox(); Windows.Add(TopBox);
		TopBox->SetRect(FRect(-1.0f, SplitterH->GetRect().GetTop(), 2.0f, 1.0f - SplitterH->GetRect().GetTop()));
		SplitterH->SideLT = TopBox;

		// 하단 박스 (BottomBox)
		SHorizontalBox* BottomBox = new SHorizontalBox(); Windows.Add(BottomBox);
		BottomBox->SetRect(FRect(-1.0f, -1.0f, 2.0f, SplitterH->GetRect().Y - (-1.0f)));
		SplitterH->SideRB = BottomBox;

	RootBox->AddChild(BottomBox);
	RootBox->AddChild(SplitterH);
	RootBox->AddChild(TopBox);

		// 상단 박스 내 좌우 분할 (TopSplitterV)
		SSplitterV* TopSplitterV = new SSplitterV(); Windows.Add(TopSplitterV);
		TopSplitterV->SetRect(FRect(-0.5f * SplitterNdcWidth, TopBox->GetRect().Y, SplitterNdcWidth, TopBox->GetRect().Height));

		SViewport* TopLeft = new SViewport(); Windows.Add(TopLeft);
		TopLeft->SetRect(FRect(-1.0f, TopBox->GetRect().Y, TopSplitterV->GetRect().X - (-1.0f), TopBox->GetRect().Height));
		TopSplitterV->SideLT = TopLeft;

		SViewport* TopRight = new SViewport(); Windows.Add(TopRight);
		TopRight->SetRect(FRect(TopSplitterV->GetRect().GetRight(), TopBox->GetRect().Y, TopBox->GetRect().GetRight() - TopSplitterV->GetRect().GetRight(), TopBox->GetRect().Height));
		TopSplitterV->SideRB = TopRight;

	TopBox->AddChild(TopLeft);
	TopBox->AddChild(TopSplitterV);
	TopBox->AddChild(TopRight);

		// 하단 박스 내 좌우 분할 (BottomSplitterV)
		SSplitterV* BottomSplitterV = new SSplitterV(); Windows.Add(BottomSplitterV);
		BottomSplitterV->SetRect(FRect(-0.5f * SplitterNdcWidth, BottomBox->GetRect().Y, SplitterNdcWidth, BottomBox->GetRect().Height));

		SViewport* BottomLeft = new SViewport(); Windows.Add(BottomLeft);
		BottomLeft->SetRect(FRect(-1.0f, BottomBox->GetRect().Y, BottomSplitterV->GetRect().X - (-1.0f), BottomBox->GetRect().Height));
		BottomSplitterV->SideLT = BottomLeft;

		SViewport* BottomRight = new SViewport(); Windows.Add(BottomRight);
		BottomRight->SetRect(FRect(BottomSplitterV->GetRect().GetRight(), BottomBox->GetRect().Y, BottomBox->GetRect().GetRight() - BottomSplitterV->GetRect().GetRight(), BottomBox->GetRect().Height));
		BottomSplitterV->SideRB = BottomRight;

	BottomBox->AddChild(BottomLeft);
	BottomBox->AddChild(BottomSplitterV);
	BottomBox->AddChild(BottomRight);

	RootWindow = RootBox;
	Viewports.Add(TopLeft); Viewports.Add(TopRight); Viewports.Add(BottomLeft); Viewports.Add(BottomRight);

	FVector2 DragInfo;
	SplitterH->SetLabel("SplitterH");
	if (SplitterH->TryLoadDragInfo(DragInfo))
	{
		SplitterH->Drag(DragInfo);
		SplitterH->DragEnd();
	}
	TopSplitterV->SetLabel("TopSplitterV");
	if (TopSplitterV->TryLoadDragInfo(DragInfo))
	{
		TopSplitterV->Drag(DragInfo);
		TopSplitterV->DragEnd();
	}
	BottomSplitterV->SetLabel("BottomSplitterV");
	if (BottomSplitterV->TryLoadDragInfo(DragInfo))
	{
		BottomSplitterV->Drag(DragInfo);
		BottomSplitterV->DragEnd();
	}
}
void UViewportManager::InitializeSubCamera(UCamera* InCamera)
{
	MainCamera = InCamera;
	int32 CameraCnt = Viewports.Num();
	for (int32 Idx = 0; Idx < CameraCnt; Idx++)
	{
		if (Viewports[Idx])
		{
			if (Viewports[Idx]->GetViewportInfo()->ViewType != EViewportViewType::Perspective) continue;
			Viewports[Idx]->GetViewportInfo()->Camera = NewObject<UCamera>();
			Viewports[Idx]->GetViewportInfo()->Camera->CopyFrom(*InCamera);
		}
	}
}

void UViewportManager::Update()
{
	if (bIsWindowDivided)
	{
		// Order Need Tobe Preserved
		if (UInputManager::GetInstance().IsKeyPressed(EKeyInput::MouseLeft))
			SetMainCamera();

		if (UInputManager::GetInstance().IsKeyPressed(EKeyInput::MouseRight))
			bOrthoManipulating = true;
		if (UInputManager::GetInstance().IsKeyReleased(EKeyInput::MouseRight))
			bOrthoManipulating = false;

		UpdateSubCamera();
	}
}

/*
* Candidate Viewport Idx 반영
*/
void UViewportManager::SetMainCamera()
{
	// 왼쪽 마우스 클릭 시 선택 카메라 조작 가능
	SelectedViewportIdx = CandidateViewportIdx;
	MainCamera->CopyFrom(*Viewports[SelectedViewportIdx]->GetViewportInfo()->Camera);
	MainCamera->SetCameraType(Viewports[SelectedViewportIdx]->GetViewportInfo()->ViewType, bIsWindowDivided);
}

void UViewportManager::UpdateSubCamera()
{
	int32 CameraCnt = sizeof(Viewports) / sizeof(Viewports[0]);
	for (int32 Idx = 0; Idx < CameraCnt; Idx++)
	{
		FViewportInfo* CurViewport = Viewports[Idx]->GetViewportInfo();

		// ViewType Perpective라면 MainCamera Transform Copy
		if (Viewports[SelectedViewportIdx]->GetViewportInfo()->ViewType == EViewportViewType::Perspective)
		{
			if (Idx != SelectedViewportIdx) continue;
			CurViewport->Camera->CopyFrom(*MainCamera);
		}

		// Ortho Viewport를 조작 중이라면 전체 Ortho Viewport에도 반영
		else if (bOrthoManipulating && CurViewport->ViewType != EViewportViewType::Perspective)
		{
			FVector NewLocation = CurViewport->Camera->GetLocation() + MainCamera->GetOrthoMoveDelta();
			CurViewport->Camera->SetLocation(NewLocation);
			CurViewport->Camera->RefreshViewMatrices();
		}
	}
}

void UViewportManager::UpdateViewportRects(const POINT& WindowSize)
{
	RootWindow->OnWindowResized(WindowSize);
}

void UViewportManager::SetProjectionMode(uint32 InIdx, EViewportViewType InViewType)
{
	if (Viewports[InIdx])
	{
		Viewports[InIdx]->GetViewportInfo()->SetViewType(InViewType);
		SetMainCamera();
	}
}

void UViewportManager::SetViewMode(uint32 InIdx, EViewportRenderMode InRenderType)
{
	if (Viewports[InIdx])
	{
		Viewports[InIdx]->GetViewportInfo()->RenderMode = InRenderType;
	}
}

FViewportInfo* UViewportManager::GetViewportInfo(uint32 ViewportIdx)
{
	if (Viewports[ViewportIdx])
	{
		return Viewports[ViewportIdx]->GetViewportInfo();
	}
	return nullptr;
}

FVector2 UViewportManager::UpdateMouseInputNdcInViewports(FVector2 MousePositionNdc)
{
	if (bIsWindowDivided)
	{
		FRect SelectedRect = Viewports[SelectedViewportIdx]->GetViewportPixelRect();
		const float W = SelectedRect.Width;
		const float H = SelectedRect.Height;
		MainCamera->SetAspect(W / H);

		// Mouse Left : Split 이면 Drag / Click만
		bool IsDragging = UInputManager::GetInstance().IsKeyDown(EKeyInput::MouseLeft);
		SetSplitterMouseInput(POINT(W, H), MousePositionNdc, IsDragging);

		// Viewport는 Viewport의 NDC를 따로 구해야함
		FVector2 MousePosition = UInputManager::GetInstance().GetMousePosition();
		MousePositionNdc = GetViewportMouseInputNdc(POINT(W, H), MousePosition);
	}
	return MousePositionNdc;
}

void UViewportManager::SetSplitterMouseInput(const POINT& WindowSize, const FVector2& InMouseNDC, bool bIsDragging)
{
	/* Splitter를 드래깅 하고 있다면 */
	if (bIsDragging && DraggingWindow)
	{
		DraggingWindow->Drag(InMouseNDC);
	}
	/* 드래깅 안 할 시*/
	else
	{
		SWindow* SelectedWindow = RootWindow->HitTest(InMouseNDC);
		if (SelectedWindow)
		{
			for (uint32 Idx = 0; Idx < Viewports.Num(); Idx++)
			{
				if (Viewports[Idx] == SelectedWindow)
				{
					CandidateViewportIdx = Idx;
					break;
				}
			}
		}
		if (bIsDragging) { DraggingWindow = SelectedWindow; }
		else
		{
			if (DraggingWindow) { DraggingWindow->DragEnd(); }
			DraggingWindow = nullptr;
		}
	}
}

FVector2 UViewportManager::GetViewportMouseInputNdc(const POINT& WindowSize, const FVector2& InMouse)
{
	const FRect& R = Viewports[SelectedViewportIdx]->GetViewportPixelRect();

	// Change to Single Viewport MouseInput
	const float u = (InMouse.X - R.X) / R.Width;
	const float v = (InMouse.Y - R.Y) / R.Height;

	return FVector2(2.0f * u - 1.0f, 1.0f - 2.0f * v);
}
