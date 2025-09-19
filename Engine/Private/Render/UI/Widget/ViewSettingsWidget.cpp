#include "pch.h"
#include "Render/UI/Widget/ViewSettingsWidget.h"
#include "Render/Renderer/Renderer.h"
#include "Manager/Viewport/ViewportManager.h"
#include "Editor/Grid.h"

// Viewport Mode, Must Sync with ViewportManager Enum Classes
static const char* GViewTypeLabels[] = {
	"Perspective", "OrthoGraphic"
};
// Camera Mode
static const char* CameraMode[] = {
	"Lit",
	"Unlit",
	"WireFrame",
};


IMPLEMENT_CLASS(UViewSettingsWidget, UWidget)

UViewSettingsWidget::UViewSettingsWidget() : Grid(nullptr)
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

	// 뷰 4분할 여부
	bool bIsWindowDivided = Renderer->GetDividedWindow();
	if (ImGui::Checkbox("Viewport 분할", &bIsWindowDivided))
	{
		Renderer->SetDividedWindow(bIsWindowDivided);
	}
	if (bIsWindowDivided)
	{
		FViewportContext* ViewportArray = ViewportManager->GetViewports();

		for (int i = 0; i < 4; i++)
		{
			ImGui::PushID(i);
			ImGui::Text("Viewport %d", i);
			int viewTypeIdx = static_cast<int>(ViewportArray[i].ViewType);
			int renderModeIdx = static_cast<int>(ViewportArray[i].RenderMode);

			if (ImGui::Combo("Projection", &viewTypeIdx, GViewTypeLabels, IM_ARRAYSIZE(GViewTypeLabels)))
			{
				auto NewType = static_cast<ECameraViewType>(viewTypeIdx);
				ViewportManager->SetProjectionMode(i, NewType);
			}

			if (ImGui::Combo("Render Mode", &renderModeIdx, CameraMode, IM_ARRAYSIZE(CameraMode)))
			{
				auto NewMode = static_cast<EViewModeIndex>(renderModeIdx);
				ViewportManager->SetViewMode(i, NewMode);
			}
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::PopID();
		}
	}

	// 그리드 간격
	float CurrentCellSize = Grid->GetCellSize();
	if (ImGui::DragFloat("그리드 간격", &CurrentCellSize, 0.001f, 10.0f))
	{
		Grid->SetCellSize(CurrentCellSize);	
	}

	if (ImGui::Combo("View Mode", &ViewModeIndex, CameraMode, IM_ARRAYSIZE(CameraMode)))
	{
		if (ViewModeIndex >= 0 && ViewModeIndex < static_cast<int32>(EViewModeIndex::End))
		{
			URenderer::GetInstance().SetViewMode(static_cast<EViewModeIndex>(ViewModeIndex));
		}
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	// Primitive Render Toggle Button
	if (ImGui::Button("Primitive Render Toggle"))
	{
		bShowPrimitiveFlags = !bShowPrimitiveFlags;
	}

	// Show checkboxes when toggle is active
	if (bShowPrimitiveFlags)
	{
		ImGui::Spacing();
		ImGui::TextUnformatted("Show Flags:");
		ImGui::Indent();

		auto& Renderer = URenderer::GetInstance();
		EEngineShowFlags CurrentFlags = Renderer.GetShowFlags();

		// Primitives checkbox
		bool bShowPrimitives = HasFlag(CurrentFlags, EEngineShowFlags::SF_Primitives);
		if (ImGui::Checkbox("Primitives", &bShowPrimitives))
		{
			Renderer.ToggleShowFlag(EEngineShowFlags::SF_Primitives);
		}

		// Billboard Text checkbox
		bool bShowBillboardText = HasFlag(CurrentFlags, EEngineShowFlags::SF_BillboardText);
		if (ImGui::Checkbox("Billboard Text", &bShowBillboardText))
		{
			Renderer.ToggleShowFlag(EEngineShowFlags::SF_BillboardText);
		}

		// Grid checkbox
		bool bShowGrid = HasFlag(CurrentFlags, EEngineShowFlags::SF_Grid);
		if (ImGui::Checkbox("Grid", &bShowGrid))
		{
			Renderer.ToggleShowFlag(EEngineShowFlags::SF_Grid);
		}

		// Bounds checkbox
		bool bShowBounds = HasFlag(CurrentFlags, EEngineShowFlags::SF_Bounds);
		if (ImGui::Checkbox("Bounding Boxes (Alt + C)", &bShowBounds))
		{
			Renderer.ToggleShowFlag(EEngineShowFlags::SF_Bounds);
		}

		ImGui::Unindent();
	}
}
