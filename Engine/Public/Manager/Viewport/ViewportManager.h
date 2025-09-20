#pragma once
#include "ViewportTypes.h"
class UCamera;
class UViewportManager : public UObject
{
	DECLARE_CLASS(UViewportManager, UObject)

public:
	UViewportManager();
	~UViewportManager();

	void Initialize(const POINT& InWindowSize);

	void SetSubCamera(UCamera* InCamera);

	void Update();
	void SetMainCamera();
	void UpdateSubCamera();
	void UpdateViewportRects(const POINT& WindowSize);

	void SetProjectionMode(uint32 InIdx, EViewportViewType InViewType);
	void SetViewMode(uint32 InIdx, EViewportRenderMode InRenderType);

	struct FViewportInfo* GetViewportInfo(uint32 ViewportIdx);

	void SetSplitterMouseInput(const POINT& WindowSize, const FVector2& InMouseNDC, bool bIsDragging);
	FVector2 GetViewportMouseInputNdc(const POINT& WindowSize, const FVector2& InMouse);

	class UCamera* GetSelectedViewportCamera();

	const TArray<class SWindow*>& GetWindows() { return Windows; }

private:
	FVector ViewportRatio;
	int CandidateViewportIdx;
	bool bSelectUpdated;
	bool bOrthoManipulating;

	UCamera* Camera;
	int32 SelectedViewportIdx = 0;
	class SWindow* RootWindow = nullptr;
	class SWindow* DraggingWindow = nullptr;

	TArray<class SViewport*> Viewports;
	TArray<class SWindow*> Windows;
};

