#include "pch.h"
#include "Slate/SplitterH.h"

void SSplitterH::Drag(FVector2 MouseCoord)
{
	FRect NewRect = GetRect();
	NewRect.Y = MouseCoord.Y - NewRect.Height / 2.0f;
	SetRect(NewRect);

	if (SideLT)
	{
		FRect SideLTRect = SideLT->GetRect();
		SideLTRect.Height = MouseCoord.Y - SideLTRect.Y;
		SideLT->SetRect(SideLTRect);
	}
	if (SideRB)
	{
		FRect SideRBRect = SideRB->GetRect();
		SideRBRect.Y = MouseCoord.Y;
		SideRB->SetRect(SideRBRect);
	}
}
