#include "pch.h"
#include "Slate/Viewport.h"
#include "Editor/Camera.h"

void FViewportInfo::SetViewType(EViewportViewType InViewType)
{
	ViewType = InViewType;
	Camera->SetCameraType(InViewType);
	Camera->RefreshViewMatrices();
}

void SViewport::OnWindowResized(const POINT& WindowSize)
{
	CurrentWindowSize = WindowSize;
	UpdateDxViewport(CurrentWindowSize);
	if (ViewportInfo.Camera)
	{
		ViewportInfo.Camera->SetAspect(GetRect().Width / GetRect().Height);
	}
}

void SViewport::OnResized()
{
	if (ViewportInfo.Camera)
	{
		ViewportInfo.Camera->SetAspect(GetRect().Width / GetRect().Height);
	}
	UpdateDxViewport(CurrentWindowSize);
}

void SViewport::UpdateDxViewport(const POINT& WindowSize)
{
	FRect NormalizedRect = GetRect();

	// NDC X좌표 변환
	float PixelX = (NormalizedRect.X + 1.0f) * 0.5f * WindowSize.x;
	// NDC Y좌표 변환 (Y축 방향 반전)
	float TopY = NormalizedRect.Y + NormalizedRect.Height;
	float PixelY = (1.0f - TopY) * 0.5f * WindowSize.y;

	float PixelWidth = NormalizedRect.Width * 0.5f * WindowSize.x;
	float PixelHeight = NormalizedRect.Height * 0.5f * WindowSize.y;

	ViewportInfo.DxViewport.TopLeftX = PixelX;
	ViewportInfo.DxViewport.TopLeftY = PixelY;
	ViewportInfo.DxViewport.Width = PixelWidth;
	ViewportInfo.DxViewport.Height = PixelHeight;
	ViewportInfo.DxViewport.MinDepth = 0.0f;
	ViewportInfo.DxViewport.MaxDepth = 1.0f;
}

// jft : independent with viewport's actual rect... but why..?
FRect SViewport::GetViewportPixelRect()
{
	return FRect(ViewportInfo.DxViewport.TopLeftX, ViewportInfo.DxViewport.TopLeftY,
		ViewportInfo.DxViewport.Width, ViewportInfo.DxViewport.Height);
}

