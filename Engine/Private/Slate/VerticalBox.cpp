#include "pch.h"
#include "Slate/VerticalBox.h"

void SVerticalBox::OnWindowResized(const POINT& InWindowSize)
{
	float TotalHeight = 0.0f;
	for (SWindow* Child : Children) {
		TotalHeight += Child->GetRect().Height;
	}

	if (TotalHeight == 0.0f) return;

	FRect Rect = GetRect();
	float CurrentY = Rect.Y;
	float ParentHeight = Rect.Height;
	float ParentWidth = Rect.Width;

	for (SWindow* Child : Children)
	{
		float Ratio = Child->GetRect().Height / TotalHeight;
		float NewHeight = ParentHeight * Ratio;

		FRect NewChildRect = { Rect.X, CurrentY, ParentWidth, NewHeight };
		Child->SetRect(NewChildRect);

		CurrentY += NewHeight;
	}
	for (SWindow* Child : Children)
	{
		Child->OnWindowResized(InWindowSize);
	}
}
