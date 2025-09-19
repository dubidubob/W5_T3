#pragma once
class UCamera;
struct FViewportContext {
	UCamera* Camera;
	D3D11_VIEWPORT Viewport;
	ECameraViewType ViewType;
	EViewModeIndex RenderMode;
};

class UViewportManager
{
public:
	void SetSubCamera(UCamera* InCamera);
	void UpdateSubCamera(UCamera* InCamera);
	void UpdateViewportRects(const POINT& WindowSize);

	void SetProjectionMode(int InIdx, ECameraViewType InViewType);
	void SetViewMode(int InIdx, EViewModeIndex InRenderType) { Viewports[InIdx].RenderMode = InRenderType; }

	FViewportContext* GetViewports() { return Viewports; }

private:
	FViewportContext Viewports[4];
};

