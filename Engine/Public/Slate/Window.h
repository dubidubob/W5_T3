#pragma once
#include "Global/Vector.h"
#include "Global/Rect.h"

class SWindow
{
public:
	virtual ~SWindow() = default;

	virtual SWindow* HitTest(const FVector2& MouseCoord);
	virtual void Drag(FVector2 MouseCoord) {}
	virtual void DragEnd() {}
	bool IsHover(const FVector2& MouseCoord) const;

	FRect GetRect() const { return Rect; }
	void SetRect(const FRect& NewRect)
	{
		Rect = NewRect;
		OnResized();
	}
	virtual bool CanRender(FRect& OutRect, FVector4& OutColor, const FVector2& MouseCoord) const { return false; }
	virtual void OnWindowResized(const POINT& WindowSize) {}

protected:
	/**
	* @brief 리사이즈시 특정 로직을 수행하는 콜백식 함수
	*/
	virtual void OnResized() {};

private:
	FRect Rect;
};
