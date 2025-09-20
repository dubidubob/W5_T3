#pragma once
#include "Box.h"

class SHorizontalBox : public SBox
{
public:
	virtual void OnWindowResized(const POINT& WindowSize) override;
};
