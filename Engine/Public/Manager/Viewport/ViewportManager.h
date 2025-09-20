#pragma once
#include "ViewportTypes.h"
class UCamera;
struct FViewportContext {
	UCamera* Camera;
	D3D11_VIEWPORT Viewport;
	EViewportViewType ViewType;
	EViewportRenderMode RenderMode;
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

	void Update();
	void SetMainCamera();
	void UpdateSubCamera();
	void UpdateViewportRects(const POINT& WindowSize);

	void SetProjectionMode(int InIdx, EViewportViewType InViewType);
	void SetViewMode(int InIdx, EViewportRenderMode InRenderType) { Viewports[InIdx].RenderMode = InRenderType; }

	FViewportContext* GetViewports() { return Viewports; }

	void UpdateSelectedViewport();
	FVector GetSelectedViewportMousePositionNdc(const POINT& WindowSize, const POINT& InMouse);
	UCamera* GetSelectedViewportCamera() { return Viewports[SelectedViewportIdx].Camera; }

	FVector GetViewportRatio() { return ViewportRatio; }

private:
	FViewportContext Viewports[4];
	FVector ViewportRatio;
	int SelectedViewportIdx;
	int CandidateViewportIdx;
	bool bSelectUpdated;
	bool bOrthoManipulating;

	UCamera* Camera;
};

