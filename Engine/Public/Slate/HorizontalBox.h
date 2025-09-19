#pragma once
#include "Window.h"

class SHorizontalBox : public SWindow
{
public:
	virtual void OnResized() override;
	void AddChild(SWindow* NewChild);

private:
	TArray<SWindow*> Children;
};
