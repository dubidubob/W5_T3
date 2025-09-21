#include "pch.h"
#include "Manager/Viewport/ViewportManager.h"
#include "Editor/Camera.h"
#include "Slate/VerticalBox.h"
#include "Slate/HorizontalBox.h"
#include "Slate/SplitterV.h"
#include "Slate/SplitterH.h"
#include "Slate/Viewport.h"

// jft, 종속성 없애기!
#include "Render/Renderer/Renderer.h"
#include "Manager/Input/InputManager.h"
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

void UViewportManager::Initialize(const POINT& InWindowSize)
{
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

void UViewportManager::SetSubCamera(UCamera* InCamera)
{
	Camera = InCamera;
	int32 CameraCnt = sizeof(Viewports) / sizeof(Viewports[0]);
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
	// jft urgent.. render와의 종속성 lets go
	if (URenderer::GetInstance().GetDividedWindow())
	{
		// Order Need Tobe Preserved
		// jft input 정리
		if (UInputManager::GetInstance().IsKeyPressed(EKeyInput::MouseLeft))
			SetMainCamera();

		if (UInputManager::GetInstance().IsKeyPressed(EKeyInput::MouseRight))
			bOrthoManipulating = true;
		if (UInputManager::GetInstance().IsKeyReleased(EKeyInput::MouseRight))
			bOrthoManipulating = false;

		UpdateSubCamera();
	}
}

void UViewportManager::SetMainCamera()
{
	// jft 이때서야 Candidate Viewport Idx 비로소 반영 todo : ray update를 click 때마다 하기
	SelectedViewportIdx = CandidateViewportIdx;
	Camera->CopyFrom(*Viewports[SelectedViewportIdx]->GetViewportInfo()->Camera);
	Camera->SetCameraType(Viewports[SelectedViewportIdx]->GetViewportInfo()->ViewType);
}

void UViewportManager::UpdateSubCamera()
{
	int32 CameraCnt = sizeof(Viewports) / sizeof(Viewports[0]);
	for (int32 Idx = 0; Idx < CameraCnt; Idx++)
	{
		if (Viewports[SelectedViewportIdx]->GetViewportInfo()->ViewType == EViewportViewType::Perspective)
		{
			if (Idx != SelectedViewportIdx) continue;
			Viewports[Idx]->GetViewportInfo()->Camera->CopyFrom(*Camera);
		}
		else
		{
			// jft
			if (bOrthoManipulating && Viewports[Idx]->GetViewportInfo()->ViewType != EViewportViewType::Perspective)
			{				
				FVector NewLocation = Viewports[Idx]->GetViewportInfo()->Camera->GetLocation() + Camera->GetOrthoMoveDelta();
				Viewports[Idx]->GetViewportInfo()->Camera->SetLocation(NewLocation);
				Viewports[Idx]->GetViewportInfo()->Camera->RefreshViewMatrices();
			}
		}
	}
}

void UViewportManager::UpdateViewportRects(const POINT& WindowSize)
{
	// jft : right now
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

void UViewportManager::SetSplitterMouseInput(const POINT& WindowSize, const FVector2& InMouseNDC, bool bIsDragging)
{
	if (bIsDragging && DraggingWindow)
	{
		DraggingWindow->Drag(InMouseNDC);
	}
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
	// UE_LOG("%.2f, %.2f, %.2f, %.2f. %.2f, %.2f", InMouse.X, InMouse.Y, R.X, R.Y, R.Width, R.Height);

	// Change to Single Viewport MouseInput
	const float u = (InMouse.X - R.X) / R.Width;
	const float v = (InMouse.Y - R.Y) / R.Height;
	// UE_LOG("%.2f, %.2f", u, v);

	FVector2 MousePositionNdc(2.0f * u - 1.0f, 1.0f - 2.0f * v);
	// UE_LOG("%.2f, %.2f", MousePositionNdc.X, MousePositionNdc.Y);
	return MousePositionNdc;
}

UCamera* UViewportManager::GetSelectedViewportCamera()
{
	if (SelectedViewportIdx >= 0 && Viewports[SelectedViewportIdx])
	{
		return Viewports[SelectedViewportIdx]->GetViewportInfo()->Camera;
	}
	return nullptr;
}
