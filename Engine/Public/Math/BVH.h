#pragma once
#include "Global/Vector.h"
#include "Math/AABB.h"
#include "Math/Frustum.h"
#include "Core/Object.h"

class UPrimitiveComponent;
class ULineBatchRenderer;

/**
 * @brief BVH 노드 - 이진 트리 구조
 */
class FBVHNode
{
public:
	FAABB Bounds;							// 노드의 바운딩 박스
	FBVHNode* LeftChild;					// 왼쪽 자식 노드
	FBVHNode* RightChild;					// 오른쪽 자식 노드
	TArray<UPrimitiveComponent*> Objects;	// 리프 노드의 객체들
	int32 Depth;							// 트리 깊이

	// 생성자
	FBVHNode() : LeftChild(nullptr), RightChild(nullptr), Depth(0) {}
	~FBVHNode();

	// 리프 노드인지 확인
	bool IsLeaf() const { return LeftChild == nullptr && RightChild == nullptr; }

	// 노드가 유효한지 확인
	bool IsValid() const { return Bounds.IsValid(); }

	// 객체 추가
	void AddObject(UPrimitiveComponent* Object);

	// 바운딩 박스 계산
	void CalculateBounds();

	// 노드 분할 (재귀적으로 자식 노드 생성)
	void Split(int32 MaxObjectsPerNode, int32 MaxDepth);

	// Ray 쿼리
	void GetObjectsIntersectingRay(const FRay& Ray, TArray<UPrimitiveComponent*>& OutObjects) const;

	// Frustum 쿼리
	void GetObjectsInFrustum(const FFrustum& Frustum, TArray<UPrimitiveComponent*>& OutObjects) const;

	// AABB 쿼리
	void GetObjectsIntersectingAABB(const FAABB& QueryBounds, TArray<UPrimitiveComponent*>& OutObjects) const;

	// 통계 수집
	void CollectStats(int32& OutNodeCount, int32& OutLeafCount, int32& OutObjectCount, int32& OutMaxDepth) const;

	// 시각화
	void DebugDraw(ULineBatchRenderer* Renderer, int32 CurrentDepth, int32 MaxDepthToDraw) const;

private:
	// 분할 축 결정 (가장 긴 축)
	int32 GetLongestAxis() const;

	// 객체들을 축을 기준으로 정렬
	void SortObjectsByAxis(int32 Axis);

	// 최적의 분할점 찾기 (SAH - Surface Area Heuristic)
	int32 FindBestSplit() const;
};

/**
 * @brief BVH (Bounding Volume Hierarchy) 트리
 */
class FBVH : public UObject
{
	DECLARE_CLASS(FBVH, UObject)

public:
	FBVH();
	~FBVH() override;

	/** 초기화 및 정리 */
	void Initialize(const FAABB& InWorldBounds);
	void Clear();

	/** 객체 관리 */
	void InsertObject(UPrimitiveComponent* Object);
	void RemoveObject(UPrimitiveComponent* Object);
	void UpdateObject(UPrimitiveComponent* Object); // 객체 이동 시 호출

	/** 쿼리 */
	void QueryBounds(const FAABB& QueryBounds, TArray<UPrimitiveComponent*>& OutObjects) const;
	void QueryRay(const FRay& Ray, TArray<UPrimitiveComponent*>& OutObjects) const;
	void QueryFrustum(const FFrustum& Frustum, TArray<UPrimitiveComponent*>& OutObjects) const;

	/** 편의 메서드 - 반환값으로 결과 제공 */
	TArray<UPrimitiveComponent*> QueryFrustum(const FFrustum& Frustum) const;

	/** 업데이트 관리 */
	void MarkForRebuild() { bNeedsRebuild = true; }
	void ConditionalUpdate();
	void ForceRebuild();
	void RebuildWithObjects(const TArray<UPrimitiveComponent*>& Objects);

	/** 상태 */
	bool IsValid() const { return Root != nullptr; }
	int32 GetTotalObjectCount() const;
	int32 GetMaxDepth() const;

	/** 시각화 */
	void DebugDraw(ULineBatchRenderer* Renderer, int32 MaxDepthToDraw = -1) const;

	/** 성능 통계 */
	struct FStats
	{
		int32 TotalNodes = 0;
		int32 LeafNodes = 0;
		int32 TotalObjects = 0;
		int32 MaxDepth = 0;
		float AverageObjectsPerLeaf = 0.0f;
		float LastRebuildTime = 0.0f;
	};
	FStats GetStats() const;

private:
	FBVHNode* Root;
	FAABB WorldBounds;

	/** 구성 매개변수 */
	static constexpr int32 MAX_OBJECTS_PER_NODE = 8;
	static constexpr int32 MAX_TREE_DEPTH = 12;

	/** 업데이트 관리 */
	bool bNeedsRebuild;

	/** 전체 재구성 */
	void RebuildTree();
	void CollectAllObjects(TArray<UPrimitiveComponent*>& OutObjects) const;
	void CollectAllObjectsRecursive(FBVHNode* Node, TArray<UPrimitiveComponent*>& OutObjects) const;
};
