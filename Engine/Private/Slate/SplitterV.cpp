#include "pch.h"
#include "Slate/SplitterV.h"

void SSplitterV::Drag(FVector2 MouseCoord)
{
	FRect NewRect = GetRect();
	NewRect.X = MouseCoord.X - NewRect.Width / 2.0f;
	SetRect(NewRect);

	if (SideLT)
	{
		FRect SideLTRect = SideLT->GetRect();
		SideLTRect.Width = MouseCoord.X - SideLTRect.X;
		SideLT->SetRect(SideLTRect);
	}

	if (SideRB)
	{
		FRect SideRBRect = SideRB->GetRect();
		SideRBRect.X = MouseCoord.X;
		SideRBRect.Width = (SideRBRect.X + SideRBRect.Width) - MouseCoord.X;
		SideRB->SetRect(SideRBRect);
	}
}
