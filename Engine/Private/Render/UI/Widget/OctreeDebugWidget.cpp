#include "pch.h"
#include "Render/UI/Widget/OctreeDebugWidget.h"
#include "Editor/Editor.h"
#include "Math/Octree.h"
#include "Manager/Time/TimeManager.h"
#include "Core/ObjectIterator.h"
#include "ImGui/imgui.h"

IMPLEMENT_CLASS(UOctreeDebugWidget, UWidget)

UOctreeDebugWidget::UOctreeDebugWidget()
{
	Editor = nullptr;
}

UOctreeDebugWidget::~UOctreeDebugWidget() = default;

void UOctreeDebugWidget::Initialize()
{
	// Editor 인스턴스 찾기
	for (TObjectIterator<UEditor> It; It; ++It)
	{
		if (It->IsA(UEditor::StaticClass()))
		{
			Editor = static_cast<UEditor*>(*It);
			break;
		}
	}

	if (Editor)
	{
		// Editor의 현재 상태로 초기화
		bShowOctreeVisualization = Editor->IsOctreeVisualizationEnabled();
		bUseOctreeForPicking = true; // 기본적으로 활성화
	}

	UE_LOG("OctreeDebugWidget: Successfully Initialized");
}

void UOctreeDebugWidget::Update()
{
	if (!Editor)
	{
		return;
	}

	// 주기적으로 통계 업데이트
	float CurrentTime = UTimeManager::GetInstance().GetGameTime();
	if (CurrentTime - LastStatsUpdateTime > STATS_UPDATE_INTERVAL)
	{
		UpdateOctreeStats();
		LastStatsUpdateTime = CurrentTime;
	}
}

void UOctreeDebugWidget::RenderWidget()
{
	if (!Editor)
	{
		ImGui::Text("Editor not found!");
		return;
	}

	ImGui::Text("=== Octree Debug ===");
	ImGui::Separator();

	RenderOctreeControls();
	ImGui::Separator();
	RenderOctreeStats();
}

void UOctreeDebugWidget::RenderOctreeControls()
{
	// Octree 사용 여부 제어
	if (ImGui::Checkbox("Use Octree for Picking", &bUseOctreeForPicking))
	{
		Editor->SetUseOctreeForPicking(bUseOctreeForPicking);
	}

	// Octree 시각화 제어
	if (ImGui::Checkbox("Show Octree Visualization", &bShowOctreeVisualization))
	{
		Editor->SetOctreeVisualization(bShowOctreeVisualization);
	}

	// 표시할 최대 깊이 제어
	ImGui::SliderInt("Max Depth to Show", &MaxDepthToShow, -1, 8);
	if (MaxDepthToShow == -1)
	{
		ImGui::SameLine();
		ImGui::Text("(All)");
	}

	// Octree 재구성 버튼
	if (ImGui::Button("Rebuild Octree"))
	{
		Editor->RebuildOctree();
	}

	ImGui::SameLine();

	// 수동 업데이트 버튼
	if (ImGui::Button("Update Octree"))
	{
		Editor->UpdateOctree();
	}
}

void UOctreeDebugWidget::RenderOctreeStats()
{
	ImGui::Text("--- Octree Statistics ---");

	FOctree* Octree = Editor->GetOctree();
	if (!Octree || !Octree->IsValid())
	{
		ImGui::Text("Octree not initialized");
		return;
	}

	// 기본 통계
	ImGui::Text("Total Nodes: %d", CachedStats.TotalNodes);
	ImGui::Text("Leaf Nodes: %d", CachedStats.LeafNodes);
	ImGui::Text("Total Objects: %d", CachedStats.TotalObjects);

	if (CachedStats.LeafNodes > 0)
	{
		ImGui::Text("Avg Objects/Leaf: %.2f", CachedStats.AverageObjectsPerLeaf);
	}

	// 트리 효율성 표시
	if (CachedStats.TotalNodes > 0)
	{
		float LeafRatio = static_cast<float>(CachedStats.LeafNodes) / CachedStats.TotalNodes;
		ImVec4 EfficiencyColor = (LeafRatio > 0.5f) ? ImVec4(0, 1, 0, 1) : ImVec4(1, 1, 0, 1);
		ImGui::TextColored(EfficiencyColor, "Leaf Ratio: %.1f%%", LeafRatio * 100.0f);
	}

	// 메모리 사용량 추정
	int32 EstimatedMemoryKB = (CachedStats.TotalNodes * sizeof(FOctreeNode)) / 1024;
	ImGui::Text("Est. Memory: %d KB", EstimatedMemoryKB);

	// 성능 상태 표시
	if (bUseOctreeForPicking)
	{
		ImGui::TextColored(ImVec4(0, 1, 0, 1), "Status: ACTIVE");
	}
	else
	{
		ImGui::TextColored(ImVec4(1, 0, 0, 1), "Status: DISABLED");
	}
}

void UOctreeDebugWidget::UpdateOctreeStats()
{
	if (!Editor)
	{
		return;
	}

	FOctree* Octree = Editor->GetOctree();
	if (!Octree || !Octree->IsValid())
	{
		// 초기화
		CachedStats.TotalNodes = 0;
		CachedStats.LeafNodes = 0;
		CachedStats.TotalObjects = 0;
		CachedStats.AverageObjectsPerLeaf = 0.0f;
		return;
	}

	// Octree로부터 통계 가져오기
	auto Stats = Octree->GetStats();
	CachedStats.TotalNodes = Stats.TotalNodes;
	CachedStats.LeafNodes = Stats.LeafNodes;
	CachedStats.TotalObjects = Stats.TotalObjects;
	CachedStats.AverageObjectsPerLeaf = Stats.AverageObjectsPerLeaf;
}