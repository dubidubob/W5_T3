#pragma once
class UCamera;
struct FViewportContext {
	UCamera* Camera;
	D3D11_VIEWPORT Viewport;
	ECameraViewType ViewType;
	EViewModeIndex RenderMode;
};

class UViewportManager : public UObject
{
	DECLARE_CLASS(UViewportManager, UObject)

public:
	UViewportManager()
	{
		// jft
		ViewportRatio.X = 0.5f;
		ViewportRatio.Y = 0.5f;
	}

	~UViewportManager();

	void SetSubCamera(UCamera* InCamera);
	void UpdateSubCamera(UCamera* InCamera);
	void UpdateViewportRects(const POINT& WindowSize);

	void SetProjectionMode(int InIdx, ECameraViewType InViewType);
	void SetViewMode(int InIdx, EViewModeIndex InRenderType) { Viewports[InIdx].RenderMode = InRenderType; }

	FViewportContext* GetViewports() { return Viewports; }

	FVector GetViewportRatio() { return ViewportRatio; }

private:
	FViewportContext Viewports[4];
	FVector ViewportRatio;
};

