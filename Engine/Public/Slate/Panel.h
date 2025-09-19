#pragma once
#include "Window.h"

class SPanel : public SWindow
{
public:
	TArray<SWindow*> Children;

	virtual void Render() const override;
};

