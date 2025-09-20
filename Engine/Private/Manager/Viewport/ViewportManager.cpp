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
}

void UViewportManager::SetSubCamera(UCamera* InCamera)
{
	Camera = InCamera;
	int CameraCnt = sizeof(Viewports) / sizeof(Viewports[0]);
	for (int i = 0; i < CameraCnt; i++)
	{
		if (Viewports[Idx])
		{
			Viewports[Idx]->GetViewportInfo()->Camera = NewObject<UCamera>();
			Viewports[Idx]->GetViewportInfo()->Camera->CopyFrom(*InCamera);
		}
	}
}

void UViewportManager::Update()
{
	// jft urgent..
	if (URenderer::GetInstance().GetDividedWindow())
	{
		// Order Need Tobe Preserved
		if (UInputManager::GetInstance().IsKeyPressed(EKeyInput::MouseLeft))
		{
			SetMainCamera();
		}

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
	Camera->CopyFrom(*Viewports[SelectedViewportIdx].Camera);
	Camera->SetCameraType(Viewports[SelectedViewportIdx].ViewType);
}

void UViewportManager::UpdateSubCamera()
{
	int32 CameraCnt = sizeof(Viewports) / sizeof(Viewports[0]);
	for (int32 Idx = 0; Idx < CameraCnt; Idx++)
	{
		if (Viewports[SelectedViewportIdx].ViewType == EViewportViewType::Perspective)
		{
			if (i != SelectedViewportIdx) continue;
			Viewports[i].Camera->CopyFrom(*Camera);
		}
		else
		{
			// jft
			if (bOrthoManipulating && Viewports[i].ViewType != EViewportViewType::Perspective)
			{				
				FVector NewLocation = Viewports[i].Camera->GetLocation() + Camera->GetOrthoMoveDelta();
				Viewports[i].Camera->SetLocation(NewLocation);
				Viewports[i].Camera->RefreshViewMatrices();
			}
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
	}
}

void UViewportManager::SetProjectionMode(int InIdx, EViewportViewType InViewType)
{
	Viewports[InIdx].ViewType = InViewType;
	Viewports[InIdx].Camera->SetCameraType(InViewType);
	Viewports[InIdx].Camera->RefreshViewMatrices(); // 카메라/VP 갱신

	SetMainCamera();
}

FVector UViewportManager::GetSelectedViewportMousePositionNdc(const POINT& WindowSize, const POINT& InMouse)
{
	float W = WindowSize.x;
	float H = WindowSize.y;
	const float boundaryW = W * ViewportRatio.X;
	const float boundaryH = H * ViewportRatio.Y;

	int selected = 0;

	// Define Rect
	struct FRect { float x, y, w, h; };
	FRect rects[4] = {
		{ 0.0f,        0.0f,        boundaryW,           boundaryH           }, // 0: LT
		{ boundaryW,   0.0f,        (W - boundaryW),     boundaryH           }, // 1: RT
		{ 0.0f,        boundaryH,   boundaryW,           (H - boundaryH)     }, // 2: LB
		{ boundaryW,   boundaryH,   (W - boundaryW),     (H - boundaryH)     }  // 3: RB
	};


	// Select Viewport
	auto hitRect = [&](const FRect& r)->bool {
		return (InMouse.x >= r.x && InMouse.x < r.x + r.w &&
			InMouse.y >= r.y && InMouse.y < r.y + r.h);
		};
	if (hitRect(rects[0])) selected = 0;
	else if (hitRect(rects[1])) selected = 1;
	else if (hitRect(rects[2])) selected = 2;
	else                        selected = 3;

	const FRect& R = rects[selected];

	// Change to Default Viewport MouseInput
	const float u = (InMouse.x - R.x) / R.w;
	const float v = (InMouse.y - R.y) / R.h;


void UViewportManager::SetMouseInputNDC(const POINT& WindowSize, const FVector2& InMouseNDC, bool bIsDragging)
{
	SelectedViewportIdx = -1;
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
					SelectedViewportIdx = Idx;
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

UCamera* UViewportManager::GetSelectedViewportCamera()
{
	if (SelectedViewportIdx >= 0 && Viewports[SelectedViewportIdx])
	{
		return Viewports[SelectedViewportIdx]->GetViewportInfo()->Camera;
	}
	return nullptr;
}
