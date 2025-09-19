#include "pch.h"
#include "Slate/HorizontalBox.h"

void SHorizontalBox::OnResized()
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

void SHorizontalBox::AddChild(SWindow* NewChild)
{
	if (NewChild)
	{
		Children.Add(NewChild);
	}
}
