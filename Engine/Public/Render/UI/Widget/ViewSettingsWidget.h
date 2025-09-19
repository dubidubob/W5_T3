#pragma once
#include "Widget.h"

class UGrid;
class URenderer;
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

	void SetGrid(UGrid* InGrid) { Grid = InGrid; }
	void SetRenderer(URenderer* InRenderer) { Renderer = InRenderer; }

private:
	UGrid* Grid;
	URenderer* Renderer;
	int32 ViewModeIndex = 0;
	bool bShowPrimitiveFlags = false;
};

