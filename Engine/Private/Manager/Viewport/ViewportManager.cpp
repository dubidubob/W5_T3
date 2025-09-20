#include "pch.h"
#include "Manager/Viewport/ViewportManager.h"
#include "Editor/Camera.h"
// jft, 종속성 없애기!
#include "Render/Renderer/Renderer.h"
#include "Manager/Input/InputManager.h"
IMPLEMENT_CLASS(UViewportManager, UObject)

UViewportManager::~UViewportManager()
{
	int CameraCnt = sizeof(Viewports) / sizeof(Viewports[0]);
	for (int i = 0; i < CameraCnt; i++)
	{
		if (Viewports[i].Camera)
		{
			delete Viewports[i].Camera;
		}
	}
}

void UViewportManager::SetSubCamera(UCamera* InCamera)
{
	Camera = InCamera;
	int CameraCnt = sizeof(Viewports) / sizeof(Viewports[0]);
	for (int i = 0; i < CameraCnt; i++)
	{
		if (Viewports[i].ViewType != EViewportViewType::Perspective) continue;
		Viewports[i].Camera = NewObject<UCamera>();
		Viewports[i].Camera->CopyFrom(*InCamera);
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
	int CameraCnt = sizeof(Viewports) / sizeof(Viewports[0]);
	for (int i = 0; i < CameraCnt; i++)
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

// jft, spliter should expand this feature
void UViewportManager::UpdateViewportRects(const POINT& WindowSize)
{
	float W = WindowSize.x;
	float H = WindowSize.y;
	float halfW = W * 0.5f, halfH = H * 0.5f; // only for 2x2 window

	// TL
	Viewports[0].Viewport = { 0,      0,      halfW, halfH, 0.f, 1.f };
	// TR
	Viewports[1].Viewport = { halfW,  0,      halfW, halfH, 0.f, 1.f };
	// BL
	Viewports[2].Viewport = { 0,      halfH,  halfW, halfH, 0.f, 1.f };
	// BR
	Viewports[3].Viewport = { halfW,  halfH,  halfW, halfH, 0.f, 1.f };
}

void UViewportManager::SetProjectionMode(int InIdx, EViewportViewType InViewType)
{
	// jft : no need to make viewtype cause it's alreay on camera
	Viewports[InIdx].ViewType = InViewType;
	Viewports[InIdx].Camera->SetCameraType(InViewType);
	Viewports[InIdx].Camera->RefreshViewMatrices(); // 카메라/VP 갱신
}

FVector UViewportManager::GetSelectedViewportMousePositionNdc(const POINT& WindowSize, const POINT& InMouse)
{
	// UCamera* SelectedCamera = Viewports[SelectedViewportIdx].Camera;
	// InCamera->CopyFrom(*SelectedCamera);

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


	// Update Results
	CandidateViewportIdx = selected;

	// NDC (-1,-1) ~ (1, 1)
	FVector MousePositionNdc(2.0f * u - 1.0f, 1.0f - 2.0f * v, 0.0f);

	return MousePositionNdc;
}

