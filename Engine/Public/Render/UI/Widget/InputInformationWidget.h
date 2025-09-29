#pragma once
#include "Widget.h"

class UEditor;

enum class ESpatialDataStructure
{
	None = 0,
	Octree = 1,
	KDTree = 2,  // 추후 구현
	BVH = 3,     // 추후 구현
	BSPTree = 4  // 추후 구현
};

class UInputInformationWidget : public UWidget
{
	DECLARE_CLASS(UInputInformationWidget, UWidget)
public:
	void Initialize() override;
	void Update() override;
	void RenderWidget() override;
	void AddKeyToHistory(const FString& InKeyName);
	static void RenderKeyList(const TArray<EKeyInput, std::allocator<EKeyInput>>& InPressedKeys);
	void RenderMouseInfo() const;
	void RenderKeyStatistics();
	void RenderSpatialDataStructureControls();

	// Special Member Function
	UInputInformationWidget();
	~UInputInformationWidget() override;

private:
	// 키 입력 히스토리
	TArray<FString> RecentKeyPresses;

	// 마우스 관련
	FVector2 LastMousePosition;
	FVector2 MouseDelta;

	// 키 입력 통계
	TMap<FString, uint32> KeyPressCount;

	// 공간 분할 구조 관련
	UEditor* Editor;
	ESpatialDataStructure CurrentSpatialStructure;
	bool bShowSpatialVisualization;
};
