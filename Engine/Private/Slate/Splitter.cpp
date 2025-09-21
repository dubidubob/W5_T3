#include "pch.h"
#include "Slate/Splitter.h"

bool SSplitter::CanRender(FRect& OutRect, FVector4& OutColor, const FVector2& MouseCoord) const
{
	OutRect = GetRect();
	OutColor = FVector4(0.1f, 0.1f, 0.1f, 1.0f);
	if (IsHover(MouseCoord) || bIsDragging) { OutColor *= 4; }
	return true;
}
