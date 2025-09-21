#pragma once
#include "Box.h"
class SVerticalBox : public SBox
{
	virtual void OnWindowResized(const POINT& InWindowSize) override;
};

