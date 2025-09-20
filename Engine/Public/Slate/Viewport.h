#pragma once
#include "Window.h"
#include "ViewportTypes.h"

struct FViewportInfo {
	class UCamera* Camera;
	D3D11_VIEWPORT DxViewport;
	EViewportViewType ViewType;
	EViewportRenderMode RenderMode;

	void SetViewType(EViewportViewType InViewType);
};

class SViewport : public SWindow
{

public:
	/**
	* @brief 외부에서 창 크기 변경 이벤트를 전달받아 뷰포트 재구성
	*/ 
	virtual void OnWindowResized(const POINT& WindowSize) override;
	FViewportInfo* GetViewportInfo() { return &ViewportInfo; }
	FRect GetViewportPixelRect();
protected:
	virtual void OnResized() override;

private:
	void UpdateDxViewport(const POINT& WindowSize);

	FViewportInfo ViewportInfo;
	POINT CurrentWindowSize;
};

