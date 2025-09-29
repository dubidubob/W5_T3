#pragma once
#include "Widget.h"

/**
 * @brief Picking Performance 관련 메트릭을 제공하는 UI Widget
 */
class UPickingPerformanceWidget : public UWidget
{
	DECLARE_CLASS(UPickingPerformanceWidget, UWidget)

public:
	void Initialize() override;
	void Update() override;
	void RenderWidget() override;

	static ImVec4 GetPerformanceColor(double InTime);

	// Special Member Function
	UPickingPerformanceWidget();
	~UPickingPerformanceWidget() override;

private:
	// Performance metrics from UEditor
	double DisplayTotalPickTime = 0.0;
	double DisplayLastPickTime = 0.0;
	uint32_t DisplayTotalPickCount = 0;

	// Octree vs BruteForce comparison
	double LastBruteForceTime = 0.0;
	double LastOctreeTime = 0.0;
	uint32_t BruteForceObjectCount = 0;
	uint32_t OctreeObjectCount = 0;

	// Calculated metrics
	double AveragePickTime = 0.0;
	double PerformanceGain = 0.0; // Octree speedup ratio

	// Display control
	bool bShowDetails = false;
	bool bShowOctreeComparison = true;

	// Refresh control
	float LastUpdateTime = 0.0f;
	static constexpr float REFRESH_INTERVAL = 0.1f;
};
