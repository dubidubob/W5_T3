#include "pch.h"
#include "Slate/SplitterH.h"

void SSplitterH::Drag(FVector2 MouseCoord)
{
	if (!SideLT || !SideRB) { return; }

	FRect NewRect = GetRect();
	NewRect.Y = MouseCoord.Y + NewRect.Height / 2.0f;
	SetRect(NewRect);

	FRect SideLTRect = SideLT->GetRect();
	SideLTRect.Y = NewRect.GetTop();
	SideLTRect.Height = 2 - NewRect.GetTop();

	FRect SideRBRect = SideRB->GetRect();
	SideRBRect.Height = NewRect.Y - SideRBRect.Y;

	SideLT->SetRect(SideLTRect);
	SideRB->SetRect(SideRBRect);
}

void SSplitterH::Render() const
{
	FRect Rect = GetRect();
	// NDC 좌표와 크기로 막대를 그립니다.
	// DrawRectNDC(deviceContext, rect.X, rect.Y, splitterWidthNDC, rect.Height);
}
