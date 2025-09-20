#include "pch.h"
#include "Slate/SplitterV.h"

void SSplitterV::Drag(FVector2 MouseCoord)
{
	if (!SideLT || !SideRB) { DragEnd(); return; }
	SSplitter::Drag(MouseCoord);

	FRect NewRect = GetRect();

	float NewCenterX = MouseCoord.X;
	float HalfWidth = NewRect.Width / 2.0f;

	// NDC Clamp (-1 ~ 1)
	float MinX = -1.0f + HalfWidth;
	float MaxX = 1.0f - HalfWidth;

	NewCenterX = clamp(NewCenterX, MinX, MaxX);
	NewRect.X = NewCenterX - HalfWidth;
	SetRect(NewRect);

	// Left Side Rect
	FRect SideLTRect = SideLT->GetRect();
	SideLTRect.Width = NewRect.X - SideLTRect.X;

	constexpr float MinWidth = 0.1f;
	// 왼쪽 사이드가 최소 크기보다 작아지지 않도록
	if (SideLTRect.Width < MinWidth)
	{
		SideLTRect.Width = MinWidth;
		NewRect.X = SideLTRect.GetRight();
		SetRect(NewRect);
	}

	// Right Side Rect
	FRect SideRBRect = SideRB->GetRect();
	SideRBRect.X = NewRect.GetRight();
	SideRBRect.Width = 1.0f - SideRBRect.X;

	if (SideRBRect.Width < MinWidth)
	{
		SideRBRect.Width = MinWidth;
		SideRBRect.X = 1.0f - MinWidth;

		NewRect.X = SideRBRect.X - NewRect.Width;
		SetRect(NewRect);

		SideLTRect.Width = NewRect.X - SideLTRect.X;
	}

	SideLT->SetRect(SideLTRect);
	SideRB->SetRect(SideRBRect);
}
