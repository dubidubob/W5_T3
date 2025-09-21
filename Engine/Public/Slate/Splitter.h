#pragma once
#include "Window.h"

class SSplitter : public SWindow
{
public:
	bool CanRender(FRect& OutRect, FVector4& OutColor, const FVector2& MouseCoord) const override;
	virtual void Drag(FVector2 MouseCoord) override;
	virtual void DragEnd() { bIsDragging = false; }

	void SetLabel(const FString& InLabel) { SplitterLabel = InLabel; }
	FString GetLabel() const { return SplitterLabel; }
	bool TryLoadDragInfo(FVector2& OutMouseCoord);

	SWindow* SideLT = nullptr; // Left or Top
	SWindow* SideRB = nullptr; // Right or Bottom

private:
	void SaveDragInfo(const FVector2& MouseCoord);

	FString SplitterLabel = "";
	bool bIsDragging = false;
};
