#pragma once
#include "Window.h"

struct FViewport {
	class UCamera* Camera;
	D3D11_VIEWPORT DxViewport;
	ECameraViewType ViewType;
	EViewModeIndex RenderMode;
};

class SViewport : public SWindow
{

public:
	/**
	* @brief 외부에서 창 크기 변경 이벤트를 전달받아 뷰포트 재구성
	*/ 
	void OnWindowResized(const POINT& WindowSize);

protected:
	virtual void OnResized() override
	{
		UpdateDxViewport(CurrentWindowSize);
	}

private:
	void UpdateDxViewport(const POINT& WindowSize);

	FViewport ViewportInfo;
	POINT CurrentWindowSize;
};

