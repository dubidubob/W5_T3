#include "pch.h"
#include "Render/UI/Widget/PickingPerformanceWidget.h"

#include "Editor/Editor.h"
#include "Manager/Time/TimeManager.h"
#include "Core/ObjectIterator.h"

IMPLEMENT_CLASS(UPickingPerformanceWidget, UWidget)

UPickingPerformanceWidget::UPickingPerformanceWidget()
{
}

UPickingPerformanceWidget::~UPickingPerformanceWidget() = default;

void UPickingPerformanceWidget::Initialize()
{
	DisplayTotalPickTime = 0.0;
	DisplayLastPickTime = 0.0;
	DisplayTotalPickCount = 0;
	AveragePickTime = 0.0;
	bShowDetails = false;
	LastUpdateTime = 0.0f;

	UE_LOG("PickingPerformanceWidget: Successfully Initialized");
}

void UPickingPerformanceWidget::Update()
{
	auto& TimeManager = UTimeManager::GetInstance();
	float CurrentTime = TimeManager.GetGameTime();

	// 일정 간격으로만 업데이트
	if (CurrentTime - LastUpdateTime > REFRESH_INTERVAL)
	{
		UEditor* Editor = nullptr;
		for (TObjectIterator<UEditor> it; it; ++it)
		{
			if (it->IsA(UEditor::StaticClass()))
			{
				Editor = static_cast<UEditor*>(*it);
				break;
			}
		}
		// UEditor에서 피킹 성능 데이터 가져오기
		if (Editor != nullptr)
		{
			DisplayTotalPickTime = Editor->GetTotalPickTime();
			DisplayLastPickTime = Editor->GetLastPickTime();
			DisplayTotalPickCount = Editor->GetTotalPickCount();

			// 평균 피킹 시간 계산 (microseconds)
			if (DisplayTotalPickCount > 0)
			{
				AveragePickTime = DisplayTotalPickTime / DisplayTotalPickCount;
			}
		}

		LastUpdateTime = CurrentTime;
	}
}

void UPickingPerformanceWidget::RenderWidget()
{
	ImGui::Text("=== Picking Performance ===");

	// 총 피킹 카운트)
	ImGui::Text("Total Pick Count: %u", DisplayTotalPickCount);

	if (DisplayTotalPickCount > 0)
	{
		// 마지막 피킹 시간
		double LastPickTimeMs = DisplayLastPickTime;
		ImVec4 LastPickColor = GetPerformanceColor(LastPickTimeMs);
		ImGui::TextColored(LastPickColor, "Last Pick Time: %.10f ms", LastPickTimeMs);

		// 평균 피킹 시간
		double AveragePickTimeMs = AveragePickTime;
		ImVec4 AverageColor = GetPerformanceColor(AveragePickTimeMs);
		ImGui::TextColored(AverageColor, "Average Pick Time: %.10f ms", AveragePickTimeMs);

		// 총 피킹 시간
		double TotalPickTimeMs = DisplayTotalPickTime;
		ImGui::Text("Total Pick Time: %.10f ms", TotalPickTimeMs);
	}
	else
	{
		ImGui::Text("No picking data available");
	}

	ImGui::Separator();
}

ImVec4 UPickingPerformanceWidget::GetPerformanceColor(double InTimeMs)
{
	if (InTimeMs < 1.0)
	{
		return {0.0f, 1.0f, 0.0f, 1.0f}; // 녹색 (우수: < 1ms)
	}
	else if (InTimeMs < 5.0)
	{
		return {0.5f, 1.0f, 0.0f, 1.0f}; // 연두색 (좋음: < 5ms)
	}
	else if (InTimeMs < 10.0)
	{
		return {1.0f, 1.0f, 0.0f, 1.0f}; // 노란색 (보통: < 10ms)
	}
	else
	{
		return {1.0f, 0.0f, 0.0f, 1.0f}; // 빨간색 (주의: >= 10ms)
	}
}
