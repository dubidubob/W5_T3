#include "pch.h"
#include "Render/UI/Widget/InputInformationWidget.h"

#include "Manager/Input/InputManager.h"
#include "Editor/Editor.h"
#include "Core/ObjectIterator.h"
#include "ImGui/imgui.h"
#include "Math/Octree.h"
#include "Math/BVH.h"

constexpr uint8 MaxKeyHistory = 10;

IMPLEMENT_CLASS(UInputInformationWidget, UWidget)

UInputInformationWidget::UInputInformationWidget()
	: Editor(nullptr)
	, CurrentSpatialStructure(ESpatialDataStructure::None)
	, bShowSpatialVisualization(false)
{
}

UInputInformationWidget::~UInputInformationWidget() = default;

void UInputInformationWidget::Initialize()
{
}

void UInputInformationWidget::Update()
{
	auto& InputManager = UInputManager::GetInstance();

	// 마우스 위치 업데이트
	FVector2 CurrentMousePosition = InputManager.GetMousePosition();
	MouseDelta = CurrentMousePosition - LastMousePosition;
	LastMousePosition = CurrentMousePosition;

	// 새로운 키 입력 확인 및 히스토리에 추가
	TArray<EKeyInput> PressedKeys = InputManager.GetPressedKeys();
	for (const auto& Key : PressedKeys)
	{
		const wchar_t* KeyString = UInputManager::KeyInputToString(Key);
		FString KeyName = FString(reinterpret_cast<const char*>(KeyString));

		// 키 통계 업데이트
		if (KeyPressCount.find(KeyName) == KeyPressCount.end())
		{
			KeyPressCount[KeyName] = 0;
		}
		++KeyPressCount[KeyName];

		// 히스토리에 추가 (중복 방지)
		if (RecentKeyPresses.empty() || RecentKeyPresses.back() != KeyName)
		{
			AddKeyToHistory(KeyName);
		}
	}
}

void UInputInformationWidget::RenderWidget()
{
	auto& InputManager = UInputManager::GetInstance();
	TArray<EKeyInput> PressedKeys = InputManager.GetPressedKeys();

	// 탭으로 구분
	if (ImGui::BeginTabBar("InputTabs"))
	{
		// 현재 입력 탭
		if (ImGui::BeginTabItem("Current Input"))
		{
			RenderKeyList(PressedKeys);
			ImGui::EndTabItem();
		}

		// 마우스 정보 탭
		if (ImGui::BeginTabItem("Mouse Info"))
		{
			RenderMouseInfo();
			ImGui::EndTabItem();
		}

		// 키 히스토리 탭
		if (ImGui::BeginTabItem("Key History"))
		{
			ImGui::Text("Recent Key Presses:");
			ImGui::Separator();

			for (int i = static_cast<int>(RecentKeyPresses.size()) - 1; i >= 0; --i)
			{
				ImGui::Text("- %s", RecentKeyPresses[i].c_str());
			}

			if (ImGui::Button("Clear History"))
			{
				RecentKeyPresses.clear();
			}

			ImGui::EndTabItem();
		}

		// 통계 탭
		if (ImGui::BeginTabItem("Statistics"))
		{
			RenderKeyStatistics();
			ImGui::EndTabItem();
		}

		// 공간 분할 구조 탭
		if (ImGui::BeginTabItem("Spatial Data Structure"))
		{
			RenderSpatialDataStructureControls();
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}
}

void UInputInformationWidget::AddKeyToHistory(const FString& InKeyName)
{
	RecentKeyPresses.push_back(InKeyName);

	// 최대 히스토리 크기 제한
	if (RecentKeyPresses.size() > MaxKeyHistory)
	{
		RecentKeyPresses.erase(RecentKeyPresses.begin());
	}
}

void UInputInformationWidget::RenderKeyList(const TArray<EKeyInput>& InPressedKeys)
{
	ImGui::Text("Pressed Keys:");
	ImGui::Separator();

	if (InPressedKeys.empty())
	{
		ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "(No Input)");
	}
	else
	{
		for (const EKeyInput& Key : InPressedKeys)
		{
			const wchar_t* KeyString = UInputManager::KeyInputToString(Key);
			ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "- %ls", KeyString);
		}

		ImGui::Separator();
		ImGui::Text("Total Keys: %d", static_cast<int>(InPressedKeys.size()));
	}
}

void UInputInformationWidget::RenderMouseInfo() const
{
	ImGui::Text("Mouse Position: (%.0f, %.0f)", LastMousePosition.X, LastMousePosition.Y);
	ImGui::Text("Mouse Delta: (%.2f, %.2f)", MouseDelta.X, MouseDelta.Y);
	ImGui::Text("Mouse Speed: %.2f", MouseDelta.Length());

	auto& InputManager = UInputManager::GetInstance();

	// 마우스 위치를 작은 그래프로 표시
	ImDrawList* DrawList = ImGui::GetWindowDrawList();
	ImVec2 CanvasPosition = ImGui::GetCursorScreenPos();
	ImVec2 CanvasSize = ImVec2(200, 100);

	// BackGround
	DrawList->AddRectFilled(CanvasPosition,
	                        ImVec2(CanvasPosition.x + CanvasSize.x, CanvasPosition.y + CanvasSize.y),
	                        IM_COL32(50, 50, 50, 255));

	// 마우스 위치에 Dot 표시
	FVector2 MouseNDC = InputManager.GetMouseNDCPosition();

	// [-1, 1] 범위를 [0, 1] 범위로 변환
	float NormalizedX = (MouseNDC.X + 1.0f) * 0.5f;
	float NormalizedY = 1 - (MouseNDC.Y + 1.0f) * 0.5f;

	// 마우스 위치에 Dot 표시
	ImVec2 MousePosNormalized = ImVec2(NormalizedX * CanvasSize.x, NormalizedY * CanvasSize.y);

	DrawList->AddCircleFilled(ImVec2(CanvasPosition.x + MousePosNormalized.x, CanvasPosition.y + MousePosNormalized.y),
	                          3.0f, IM_COL32(255, 0, 0, 255));

	ImGui::Dummy(CanvasSize);
}

void UInputInformationWidget::RenderKeyStatistics()
{
	ImGui::Text("Key Press Statistics:");
	ImGui::Separator();

	if (KeyPressCount.empty())
	{
		ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No statistics yet");
	}
	else
	{
		// 통계를 카운트 순으로 정렬
		TArray<TPair<FString, uint32>> SortedStats;
		for (const auto& Pair : KeyPressCount)
		{
			SortedStats.push_back(Pair);
		}

		std::sort(SortedStats.begin(), SortedStats.end(),
		          [](const auto& A, const auto& B) { return A.second > B.second; });

		for (const auto& [Key, Count] : SortedStats)
		{
			ImGui::Text("%s: %d times", Key.c_str(), Count);
		}

		if (ImGui::Button("Clear Statistics"))
		{
			KeyPressCount.clear();
		}
	}
}

void UInputInformationWidget::RenderSpatialDataStructureControls()
{
	if (!Editor)
	{
		for (TObjectIterator<UEditor> It; It; ++It)
		{
			if (It->IsA(UEditor::StaticClass()))
			{
				Editor = static_cast<UEditor*>(*It);
				break;
			}
		}

		// 기본값 설정
		CurrentSpatialStructure = ESpatialDataStructure::Octree; // 기본적으로 Octree 활성화
		bShowSpatialVisualization = true;

		// Editor에 설정 적용
		if (Editor)
		{
			Editor->SetUseOctreeForPicking(true);
			Editor->SetOctreeVisualization(bShowSpatialVisualization);
		}
	}

	ImGui::Text("=== Spatial Data Structure Controls ===");
	ImGui::Separator();

	// 공간 분할 구조 선택
	ImGui::Text("Picking Method:");
	const char* spatialMethods[] = { "None (Brute Force)", "Octree", "BVH", "KD-Tree (Coming Soon)", "BSP Tree (Coming Soon)" };
	int currentMethod = static_cast<int>(CurrentSpatialStructure);

	if (ImGui::Combo("##SpatialMethod", &currentMethod, spatialMethods, IM_ARRAYSIZE(spatialMethods)))
	{
		CurrentSpatialStructure = static_cast<ESpatialDataStructure>(currentMethod);

		// Editor에 설정 적용
		switch (CurrentSpatialStructure)
		{
		case ESpatialDataStructure::None:
			Editor->SetUseOctreeForPicking(false);
			Editor->SetUseBVHForPicking(false);
			bShowSpatialVisualization = false;
			Editor->SetOctreeVisualization(false);
			Editor->SetBVHVisualization(false);
			break;
		case ESpatialDataStructure::Octree:
			Editor->SetUseOctreeForPicking(true);
			Editor->SetUseBVHForPicking(false);
			Editor->SetOctreeVisualization(bShowSpatialVisualization);
			Editor->SetBVHVisualization(false);
			Editor->RebuildOctree();
			break;
		case ESpatialDataStructure::BVH:
			Editor->SetUseOctreeForPicking(false);
			Editor->SetUseBVHForPicking(true);
			Editor->SetOctreeVisualization(false);
			Editor->SetBVHVisualization(bShowSpatialVisualization);
			Editor->RebuildBVH();
			break;
		default:
			// 다른 구조들은 아직 구현되지 않음
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "This method is not implemented yet!");
			break;
		}
	}

	ImGui::Spacing();

	// 시각화 옵션 (공간 분할 구조가 선택된 경우)
	if (CurrentSpatialStructure == ESpatialDataStructure::Octree || CurrentSpatialStructure == ESpatialDataStructure::BVH)
	{
		ImGui::Text("Visualization Options:");

		const char* StructureName = (CurrentSpatialStructure == ESpatialDataStructure::Octree) ? "Octree" : "BVH";
		FString CheckboxLabel = FString("Show ") + StructureName + " Visualization";

		if (ImGui::Checkbox(CheckboxLabel.c_str(), &bShowSpatialVisualization))
		{
			if (CurrentSpatialStructure == ESpatialDataStructure::Octree)
			{
				Editor->SetOctreeVisualization(bShowSpatialVisualization);
			}
			else if (CurrentSpatialStructure == ESpatialDataStructure::BVH)
			{
				Editor->SetBVHVisualization(bShowSpatialVisualization);
			}
		}

		ImGui::Spacing();

		// Frustum Culling 컨트롤
		ImGui::Text("Rendering Optimization:");
		bool bUseFrustumCulling = Editor->IsUsingFrustumCulling();
		if (ImGui::Checkbox("Enable Frustum Culling", &bUseFrustumCulling))
		{
			Editor->SetUseFrustumCulling(bUseFrustumCulling);
		}

		ImGui::SameLine();
		ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "(?)");
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("Frustum culling reduces rendering load by only rendering objects visible to the camera");
		}

		ImGui::Spacing();

		// 통계 표시
		if (CurrentSpatialStructure == ESpatialDataStructure::Octree)
		{
			auto* Octree = Editor->GetOctree();
			if (Octree && Octree->IsValid())
			{
				ImGui::Text("--- Octree Statistics ---");
				auto Stats = Octree->GetStats();

				ImGui::Text("Total Nodes: %d", Stats.TotalNodes);
				ImGui::Text("Leaf Nodes: %d", Stats.LeafNodes);
				ImGui::Text("Total Objects: %d", Stats.TotalObjects);

				if (Stats.LeafNodes > 0)
				{
					ImGui::Text("Avg Objects/Leaf: %.2f", Stats.AverageObjectsPerLeaf);
				}

				// 효율성 표시
				if (Stats.TotalNodes > 0)
				{
					float LeafRatio = static_cast<float>(Stats.LeafNodes) / Stats.TotalNodes;
					ImVec4 EfficiencyColor = (LeafRatio > 0.5f) ? ImVec4(0, 1, 0, 1) : ImVec4(1, 1, 0, 1);
					ImGui::TextColored(EfficiencyColor, "Leaf Ratio: %.1f%%", LeafRatio * 100.0f);
				}

				ImGui::Spacing();

				// 수동 재구성 버튼
				if (ImGui::Button("Rebuild Octree"))
				{
					Editor->RebuildOctree();
				}
			}
			else
			{
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Octree not initialized");
			}
		}
		else if (CurrentSpatialStructure == ESpatialDataStructure::BVH)
		{
			auto* BVH = Editor->GetBVH();
			if (BVH && BVH->IsValid())
			{
				ImGui::Text("--- BVH Statistics ---");
				auto Stats = BVH->GetStats();

				ImGui::Text("Total Nodes: %d", Stats.TotalNodes);
				ImGui::Text("Leaf Nodes: %d", Stats.LeafNodes);
				ImGui::Text("Total Objects: %d", Stats.TotalObjects);
				ImGui::Text("Max Depth: %d", Stats.MaxDepth);

				if (Stats.LeafNodes > 0)
				{
					ImGui::Text("Avg Objects/Leaf: %.2f", Stats.AverageObjectsPerLeaf);
				}

				// 효율성 표시
				if (Stats.TotalNodes > 0)
				{
					float LeafRatio = static_cast<float>(Stats.LeafNodes) / Stats.TotalNodes;
					ImVec4 EfficiencyColor = (LeafRatio > 0.5f) ? ImVec4(0, 1, 0, 1) : ImVec4(1, 1, 0, 1);
					ImGui::TextColored(EfficiencyColor, "Leaf Ratio: %.1f%%", LeafRatio * 100.0f);
				}

				ImGui::Spacing();

				// 수동 재구성 버튼
				if (ImGui::Button("Rebuild BVH"))
				{
					Editor->RebuildBVH();
				}
			}
			else
			{
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "BVH not initialized");
			}
		}
	}

	ImGui::Spacing();

	// 성능 정보 표시
	ImGui::Text("--- Performance Info ---");
	ImGui::Text("Total Pick Count: %u", Editor->GetTotalPickCount());
	ImGui::Text("Last Pick Time: %.6f ms", Editor->GetLastPickTime());

	if (Editor->GetTotalPickCount() > 0)
	{
		double avgTime = Editor->GetTotalPickTime() / Editor->GetTotalPickCount();
		ImGui::Text("Average Pick Time: %.6f ms", avgTime);

		// 성능 상태 표시
		ImVec4 PerformanceColor;
		if (avgTime < 1.0)
			PerformanceColor = ImVec4(0, 1, 0, 1); // 녹색 (우수)
		else if (avgTime < 5.0)
			PerformanceColor = ImVec4(1, 1, 0, 1); // 노랑 (보통)
		else
			PerformanceColor = ImVec4(1, 0, 0, 1); // 빨강 (주의)

		ImGui::TextColored(PerformanceColor, "Performance: %s",
			avgTime < 1.0 ? "Excellent" : avgTime < 5.0 ? "Good" : "Needs Optimization");
	}

	ImGui::Spacing();

	// Frustum Culling 통계 표시
	if ((CurrentSpatialStructure == ESpatialDataStructure::Octree || CurrentSpatialStructure == ESpatialDataStructure::BVH) && Editor->IsUsingFrustumCulling())
	{
		ImGui::Text("--- Frustum Culling Stats ---");

		// 현재 보이는 객체 수 계산
		auto VisiblePrimitives = Editor->GetVisiblePrimitivesInFrustum();
		int32 VisibleCount = VisiblePrimitives.size();

		// 전체 객체 수
		int32 TotalCount = 0;
		if (CurrentSpatialStructure == ESpatialDataStructure::Octree)
		{
			auto* Octree = Editor->GetOctree();
			if (Octree && Octree->IsValid())
			{
				TotalCount = Octree->GetTotalObjectCount();
			}
		}
		else if (CurrentSpatialStructure == ESpatialDataStructure::BVH)
		{
			auto* BVH = Editor->GetBVH();
			if (BVH && BVH->IsValid())
			{
				TotalCount = BVH->GetTotalObjectCount();
			}
		}

		ImGui::Text("Visible Objects: %d", VisibleCount);
		ImGui::Text("Total Objects: %d", TotalCount);

		if (TotalCount > 0)
		{
			int32 CulledCount = TotalCount - VisibleCount;
			float CullingRatio = (float)CulledCount / TotalCount * 100.0f;

			ImVec4 CullingColor = (CullingRatio > 50.0f) ? ImVec4(0, 1, 0, 1) :
								  (CullingRatio > 25.0f) ? ImVec4(1, 1, 0, 1) : ImVec4(1, 0, 0, 1);

			ImGui::Text("Culled Objects: %d", CulledCount);
			ImGui::TextColored(CullingColor, "Culling Efficiency: %.1f%%", CullingRatio);
		}
	}
}
