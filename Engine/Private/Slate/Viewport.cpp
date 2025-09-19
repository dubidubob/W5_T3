#include "pch.h"
#include "Slate/Viewport.h"

void SViewport::OnWindowResized(const POINT& WindowSize)
{
	CurrentWindowSize = WindowSize;
	UpdateDxViewport(CurrentWindowSize);
}

void SViewport::UpdateDxViewport(const POINT& WindowSize)
{
	FRect NormalizedRect = GetRect();

	// NDC(-1~1) -> Window(0~Width/Height)
	float PixelX = (NormalizedRect.X + 1.0f) * 0.5f * WindowSize.x;
	float PixelY = (1.0f - NormalizedRect.Y) * 0.5f * WindowSize.y;
	float PixelWidth = NormalizedRect.Width * 0.5f * WindowSize.x;
	float PixelHeight = NormalizedRect.Height * 0.5f * WindowSize.y;

	ViewportInfo.DxViewport.TopLeftX = PixelX;
	ViewportInfo.DxViewport.TopLeftY = PixelY;
	ViewportInfo.DxViewport.Width = PixelWidth;
	ViewportInfo.DxViewport.Height = PixelHeight;
	ViewportInfo.DxViewport.MinDepth = 0.0f;
	ViewportInfo.DxViewport.MaxDepth = 1.0f;
}
