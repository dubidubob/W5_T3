#pragma once
#include "Window.h"

class SSplitter : public SWindow
{
public:
	SWindow* SideLT = nullptr; // Left or Top
	SWindow* SideRB = nullptr; // Right or Bottom

	virtual void Drag(FVector2 MouseCoord) = 0;
};
