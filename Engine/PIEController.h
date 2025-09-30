#pragma once

#include "Render/UI/Window/UIWindow.h"


class UPIEController : public UUIWindow
{
	DECLARE_CLASS(UPIEController, UUIWindow)
	DECLARE_SINGLETON(UPIEController)

public:
	void Initialize();
};
