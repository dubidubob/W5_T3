#pragma once
enum class EViewportViewType {
	Perspective,
	OrthoTop,
	OrthoLeft,
	OrthoRight,
	OrthoFront,
};

enum class EViewportRenderMode {
	Lit,
	Unlit,
	Wireframe,
};

class UCamera;
struct FViewportContext {
	UCamera* Camera;
	D3D11_VIEWPORT Viewport;
	EViewportViewType ViewType;
	EViewportRenderMode RenderMode;
};

class UViewportManager
{
public:
	void UpdateViewportRects(const POINT& WindowSize);

	void SetProjectionMode(int InIdx, EViewportViewType InViewType) { Viewports[InIdx].ViewType = InViewType; }
	void SetViewMode(int InIdx, EViewportRenderMode InRenderType) { Viewports[InIdx].RenderMode = InRenderType; }

	FViewportContext* GetViewports() { return Viewports; }
private:
	FViewportContext Viewports[4];

};

