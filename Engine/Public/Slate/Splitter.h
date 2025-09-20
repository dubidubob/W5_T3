#pragma once
#include "Window.h"

class SSplitter : public SWindow
{
public:
	bool CanRender(FRect& OutRect, FVector4& OutColor, const FVector2& MouseCoord) const override;
	virtual void Drag(FVector2 MouseCoord) override { bIsDragging = true; }
	virtual void DragEnd() { bIsDragging = false; }

	SWindow* SideLT = nullptr; // Left or Top
	SWindow* SideRB = nullptr; // Right or Bottom

private:
	bool bIsDragging = false;
};
