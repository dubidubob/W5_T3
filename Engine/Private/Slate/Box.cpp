#include "pch.h"
#include "Slate/Box.h"
#include "Slate/SplitterV.h"
#include "Slate/Viewport.h"

SBox::~SBox()
{
	for (SWindow* Child : Children)
	{
		if (Child) { delete Child; }
	}
}

SWindow* SBox::HitTest(const FVector2& MouseCoord)
{
	if (!SWindow::HitTest(MouseCoord)) { return nullptr; }
	for (SWindow* Child : Children)
	{
		SWindow* Hit = Child->HitTest(MouseCoord);
		if (Hit) { return Hit; }
	}
	return this;
}

void SBox::OnResized()
{
	FRect NewRect = GetRect();
	for (SWindow* Child : Children)
	{
		if (Child)
		{
			FRect ChildRect = Child->GetRect();
			ChildRect.Y = NewRect.Y;
			ChildRect.Height = NewRect.Height;

			Child->SetRect(ChildRect);
		}
	}
}

void SBox::AddChild(SWindow* NewChild)
{
	if (NewChild)
	{
		Children.Add(NewChild);
	}
}
