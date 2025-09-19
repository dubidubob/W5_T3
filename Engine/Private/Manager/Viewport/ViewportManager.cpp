#include "pch.h"
#include "Manager/Viewport/ViewportManager.h"
#include "Editor/Camera.h"

void UViewportManager::SetSubCamera(UCamera* InCamera)
{
	int CameraCnt = sizeof(Viewports) / sizeof(Viewports[0]);
	for (int i = 0; i < CameraCnt; i++)
	{
		Viewports[i].Camera = new UCamera();
		Viewports[i].Camera->CopyFrom(*InCamera);
	}
}

void UViewportManager::UpdateSubCamera(UCamera* InCamera)
{
	int CameraCnt = sizeof(Viewports) / sizeof(Viewports[0]);
	for (int i = 0; i < CameraCnt; i++)
	{
		Viewports[i].Camera->CopyFrom(*InCamera);
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

void UViewportManager::SetProjectionMode(int InIdx, ECameraViewType InViewType)
{
	// jft : no need to make viewtype cause it's alreay on camera
	Viewports[InIdx].ViewType = InViewType;
	Viewports[InIdx].Camera->SetCameraType(InViewType);
	Viewports[InIdx].Camera->RefreshViewMatrices(); // 카메라/VP 갱신
}

