#pragma once
#include "Global/Vector.h"
#include "Global/Rect.h"

class SWindow
{
public:
	virtual bool IsHover(FVector2 Coord) const
	{
		return (Coord.X >= Rect.X && Coord.X <= Rect.X + Rect.Width &&
			Coord.Y >= Rect.Y && Coord.Y <= Rect.Y + Rect.Height);
	}

	FRect GetRect() const { return Rect; }

	void SetRect(const FRect& NewRect)
	{
		Rect = NewRect;
		OnResized();
	}
	virtual void Render() const {}

protected:
	/**
	* @brief 리사이즈시 특정 로직을 수행하는 콜백식 함수
	*/
	virtual void OnResized() {};

private:
	FRect Rect;
};
