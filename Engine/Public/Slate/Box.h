#pragma once
#include "Window.h"
class SBox : public SWindow
{
public:
	virtual ~SBox();

	virtual SWindow* HitTest(const FVector2& MouseCoord);
	virtual void OnResized() override;
	virtual void OnWindowResized(const POINT& WindowSize) {};
	void AddChild(SWindow* NewChild);

protected:
	TArray<SWindow*> Children;
};

