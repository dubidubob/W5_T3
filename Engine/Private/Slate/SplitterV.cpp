#include "pch.h"
#include "Slate/SplitterV.h"

void SSplitterV::Drag(FVector2 MouseCoord)
{
	if (!SideLT || !SideRB) { return; }

	FRect NewRect = GetRect();
	NewRect.X = MouseCoord.X + NewRect.Width / 2.0f;
	SetRect(NewRect);

	FRect SideLTRect = SideLT->GetRect();
	SideLTRect.Width = NewRect.X - SideLTRect.X;

	FRect SideRBRect = SideRB->GetRect();
	SideRBRect.X = NewRect.GetRight();
	SideRBRect.Width = 2 - SideRBRect.X;

	SideLT->SetRect(SideLTRect);
	SideRB->SetRect(SideRBRect);
}

void SSplitterV::Render() const
{
	FRect Rect = GetRect();
	// NDC 좌표와 크기로 막대를 그립니다.
	// DrawRectNDC(deviceContext, rect.X, rect.Y, splitterWidthNDC, rect.Height);
}
