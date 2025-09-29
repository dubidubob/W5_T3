#pragma once
#include "Widget.h"

class UEditor;
class FOctree;

/**
 * @brief Octree 디버그 및 제어를 위한 UI Widget
 */
class UOctreeDebugWidget : public UWidget
{
	DECLARE_CLASS(UOctreeDebugWidget, UWidget)

public:
	void Initialize() override;
	void Update() override;
	void RenderWidget() override;

	// Special Member Functions
	UOctreeDebugWidget();
	~UOctreeDebugWidget() override;

private:
	UEditor* Editor;

	// UI Controls
	bool bShowOctreeVisualization = false;
	bool bUseOctreeForPicking = true;
	int32 MaxDepthToShow = -1; // -1 = all depths

	// Statistics Display
	struct FOctreeStats
	{
		int32 TotalNodes = 0;
		int32 LeafNodes = 0;
		int32 TotalObjects = 0;
		float AverageObjectsPerLeaf = 0.0f;
	} CachedStats;

	float LastStatsUpdateTime = 0.0f;
	static constexpr float STATS_UPDATE_INTERVAL = 0.5f; // 0.5초마다 통계 업데이트

	void UpdateOctreeStats();
	void RenderOctreeControls();
	void RenderOctreeStats();
};