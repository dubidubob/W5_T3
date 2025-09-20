#include "pch.h"
#include "Slate/SplitterH.h"

void SSplitterH::Drag(FVector2 MouseCoord)
{
	if (!SideLT || !SideRB) { DragEnd(); return; }
	SSplitter::Drag(MouseCoord);

	FRect NewRect = GetRect();

	float NewCenterY = MouseCoord.Y;
	float HalfHeight = NewRect.Height / 2.0f;

	// NDC Clamp (-1 ~ 1)
	float MinY = -1.0f + HalfHeight;
	float MaxY = 1.0f - HalfHeight;

	NewCenterY = clamp(NewCenterY, MinY, MaxY);
	NewRect.Y = NewCenterY - HalfHeight;
	SetRect(NewRect);

	// Top Side Rect
	FRect SideLTRect = SideLT->GetRect();
	SideLTRect.Y = NewRect.GetTop();
	SideLTRect.Height = 1.0f - NewRect.GetTop();

	constexpr float MinHeight = 0.1f;
	// 위쪽 사이드가 최소 크기보다 작아지지 않도록
	if (SideLTRect.Height < MinHeight)
	{
		SideLTRect.Height = MinHeight;
		SideLTRect.Y = 1.0f - MinHeight;

		NewRect.Y = SideLTRect.Y - NewRect.Height;
		SetRect(NewRect);
	}

	// Bottom Side Rect
	FRect SideRBRect = SideRB->GetRect();
	SideRBRect.Height = NewRect.Y - SideRBRect.Y;

	// 아래쪽 사이드가 최소 크기보다 작아지지 않도록
	if (SideRBRect.Height < MinHeight)
	{
		SideRBRect.Height = MinHeight;
		NewRect.Y = SideRBRect.GetTop();
		SetRect(NewRect);

		SideLTRect.Y = NewRect.GetTop();
		SideLTRect.Height = 1.0f - NewRect.GetTop();
	}

	SideLT->SetRect(SideLTRect);
	SideRB->SetRect(SideRBRect);
}
