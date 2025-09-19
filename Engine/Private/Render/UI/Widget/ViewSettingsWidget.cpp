#include "pch.h"
#include "Render/UI/Widget/ViewSettingsWidget.h"
#include "Render/Renderer/Renderer.h"
#include "Manager/Viewport/ViewportManager.h"
#include "ViewportTypes.h"
#include "Editor/Grid.h"

IMPLEMENT_CLASS(UViewSettingsWidget, UWidget)

UViewSettingsWidget::UViewSettingsWidget() : Grid(nullptr) {}

UViewSettingsWidget::~UViewSettingsWidget() = default;

void UViewSettingsWidget::Initialize() {}

void UViewSettingsWidget::Update() {}

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
			int ViewTypeIdx = static_cast<int>(ViewportArray[i].ViewType);
			int RenderModeIdx = static_cast<int>(ViewportArray[i].RenderMode);

			if (ImGui::Combo("Projection", &ViewTypeIdx, ViewportUI::ViewTypeLabels.data(), static_cast<int>(ViewportUI::ViewTypeLabels.size())))
			{
				auto NewType = static_cast<EViewportViewType>(ViewTypeIdx);
				ViewportManager->SetProjectionMode(i, NewType);
			}

			if (ImGui::Combo("Render Mode", &RenderModeIdx, ViewportUI::RenderModeLabels.data(), static_cast<int>(ViewportUI::RenderModeLabels.size())))
			{
				auto NewMode = static_cast<EViewportRenderMode>(RenderModeIdx);
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

	if (ImGui::Combo("View Mode", &ViewModeIndex, ViewportUI::RenderModeLabels.data(), static_cast<int>(ViewportUI::RenderModeLabels.size())))
	{
		if (ViewModeIndex >= 0 && ViewModeIndex < static_cast<int32>(EViewportRenderMode::End))
		{
			URenderer::GetInstance().SetViewMode(static_cast<EViewportRenderMode>(ViewModeIndex));
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
