#pragma once
#include "ViewportTypes.h"
class UCamera;
class UViewportManager : public UObject
{
	DECLARE_CLASS(UViewportManager, UObject)

public:
	UViewportManager();
	~UViewportManager();

	void Initialize(UCamera* InCamera);
	void InitializeGizmoScale(float InScale);

	void Update(); /*Editor 호출*/
	FVector2 UpdateMouseInputNdcInViewports(FVector2 MousePositionNdc); /*Editor Mouse Input 호출*/
	void UpdateViewportRects(const POINT& WindowSize); /*Renderer 호출*/

	/*View Setting Widget에서 설정*/
	void SetProjectionMode(uint32 InIdx, EViewportViewType InViewType);
	void SetViewMode(uint32 InIdx, EViewportRenderMode InRenderType);

	/*Viewport Infos*/
	struct FViewportInfo* GetViewportInfo(uint32 ViewportIdx);
	const TArray<class SWindow*>& GetWindows() { return Windows; }
	bool GetIsWindowDivided() { return bIsWindowDivided; }
	void SetIsWindowDivided(bool bInIsWindowDivided) { bIsWindowDivided = bInIsWindowDivided; }
	bool IsSelectedWindowIdx(int Idx) { return SelectedViewportIdx == Idx; }
private:
	/*Initialize()에서 호출*/
	void InitializeSubCamera(UCamera* InCamera);
	void InitializeSplitter();

	/*Update()에서 호출*/
	void UpdateSubCamera(); 
	void SetMainCamera();

	/*Mouse Input Update()에서 호출*/
	void SetSplitterMouseInput(const POINT& WindowSize, const FVector2& InMouseNDC, bool bIsDragging);
	FVector2 GetViewportMouseInputNdc(const POINT& WindowSize, const FVector2& InMouse);

private:
	bool bOrthoManipulating;
	bool bIsWindowDivided = false;

	// jft
	bool bIsGizmoScaleInitialized = false;

	UCamera* MainCamera;

	int32 CandidateViewportIdx;
	int32 SelectedViewportIdx = 0;

	class SWindow* RootWindow = nullptr;
	class SWindow* DraggingWindow = nullptr;

	TArray<class SViewport*> Viewports;
	TArray<class SWindow*> Windows;
};

