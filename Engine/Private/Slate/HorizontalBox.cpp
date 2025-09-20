#include "pch.h"
#include "Slate/HorizontalBox.h"

void SHorizontalBox::OnWindowResized(const POINT& InWindowSize)
{
	float TotalWidth = 0.0f;
	for (SWindow* Child : Children) {
		TotalWidth += Child->GetRect().Width;
	}

	if (TotalWidth == 0.0f) return;

	FRect Rect = GetRect();
	float CurrentX = Rect.X;
	float ParentHeight = Rect.Height;
	float ParentWidth = Rect.Width;

	for (SWindow* Child : Children)
	{
		float Ratio = Child->GetRect().Width / TotalWidth;
		float NewWidth = ParentWidth * Ratio;

		FRect NewChildRect = { CurrentX, Rect.Y, NewWidth, ParentHeight };
		Child->SetRect(NewChildRect);

		CurrentX += NewWidth;
	}
	for (SWindow* Child : Children)
	{
		Child->OnWindowResized(InWindowSize);
	}
}
