#pragma once
#include "Widget.h"

class UViewSettingsWidget : public UWidget
{
	DECLARE_CLASS(UViewSettingsWidget, UWidget)
public:
	UViewSettingsWidget();
	~UViewSettingsWidget() override;

	// Special Member Function
	void Initialize() override;
	void Update() override;
	void RenderWidget() override;

private:
	int32 ViewModeIndex = 0;
	bool bShowPrimitiveFlags = false;
};

