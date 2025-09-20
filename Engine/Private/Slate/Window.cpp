#include "pch.h"
#include "Slate/Window.h"

SWindow* SWindow::HitTest(const FVector2& MouseCoord)
{
	if (IsHover(MouseCoord))
	{
		return this;
	}
	return nullptr;
}

bool SWindow::IsHover(const FVector2& MouseCoord) const
{
	return (MouseCoord.X >= Rect.X && MouseCoord.X <= Rect.X + Rect.Width &&
		MouseCoord.Y >= Rect.Y && MouseCoord.Y <= Rect.Y + Rect.Height);
}
