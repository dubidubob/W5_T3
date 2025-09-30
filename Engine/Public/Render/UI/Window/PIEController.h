#pragma once

#include "UIWindow.h"

class ULevelManager;

class UPIEController : public UUIWindow
{
	DECLARE_CLASS(UPIEController, UUIWindow)
public:
	PIEController();
	void Initialize() override;
};

