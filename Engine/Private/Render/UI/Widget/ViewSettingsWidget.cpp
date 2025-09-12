#include "pch.h"
#include "Render/UI/Widget/ViewSettingsWidget.h"
#include "Render/Renderer/Renderer.h"

// Camera Mode
static const char* CameraMode[] = {
	"Lit",
	"Unlit",
	"WireFrame",
};

UViewSettingsWidget::UViewSettingsWidget()
	: UWidget("View Settings Widget")
{
}

UViewSettingsWidget::~UViewSettingsWidget() = default;

void UViewSettingsWidget::Initialize()
{
}

void UViewSettingsWidget::Update()
{
}

void UViewSettingsWidget::RenderWidget()
{
	ImGui::TextUnformatted("View Settings");
	ImGui::Spacing();

	if (ImGui::Combo("View Mode", &ViewModeIndex, CameraMode, IM_ARRAYSIZE(CameraMode)))
	{
		if (ViewModeIndex >= 0 && ViewModeIndex < static_cast<int32>(EViewModeIndex::End))
		{
			URenderer::GetInstance().SetViewMode(static_cast<EViewModeIndex>(ViewModeIndex));
		}
	}
}
