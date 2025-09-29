#pragma once
#include "Math/AABB.h"
#include "Core/Object.h"

class UPrimitiveComponent;
class ULineBatchRenderer;
struct FRay;
struct FFrustum;

/**
 * @brief Octree Node - 8개 자식 노드를 가지는 공간 분할 노드
 */
class FOctreeNode
{
public:
	FOctreeNode();
	FOctreeNode(const FAABB& InBounds, int32 InDepth = 0);
	~FOctreeNode();

	/** 노드 관리 */
	void Subdivide();
	void Clear();
	bool IsLeaf() const { return Children[0] == nullptr; }
	bool ShouldSubdivide() const;

	/** 객체 관리 */
	void InsertObject(UPrimitiveComponent* Object);
	void RemoveObject(UPrimitiveComponent* Object);
	void GetObjectsInBounds(const FAABB& QueryBounds, TArray<UPrimitiveComponent*>& OutObjects) const;
	void GetObjectsIntersectingRay(const FRay& Ray, TArray<UPrimitiveComponent*>& OutObjects) const;
	void GetObjectsInFrustum(const FFrustum& Frustum, TArray<UPrimitiveComponent*>& OutObjects) const;

	/** 접근자 */
	const FAABB& GetBounds() const { return Bounds; }
	int32 GetDepth() const { return Depth; }
	int32 GetObjectCount() const { return Objects.Num(); }
	const TArray<UPrimitiveComponent*>& GetObjects() const { return Objects; }

	/** 시각화 */
	void DebugDraw(ULineBatchRenderer* Renderer, int32 MaxDepthToDraw = -1) const;

	/** 상수 */
	static const int32 MAX_OBJECTS_PER_NODE = 10;
	static const int32 MAX_DEPTH = 4;

public:
	FOctreeNode* Children[8]; // CalculateStats에서 접근 필요

private:
	FAABB Bounds;
	TArray<UPrimitiveComponent*> Objects;
	int32 Depth;

	/** 내부 헬퍼 */
	int32 GetChildIndex(const FVector& Point) const;
	FAABB GetChildBounds(int32 ChildIndex) const;
	void InsertObjectRecursive(UPrimitiveComponent* Object);
	void GetObjectsRecursive(const FAABB& QueryBounds, TArray<UPrimitiveComponent*>& OutObjects) const;
	void GetObjectsRayRecursive(const FRay& Ray, TArray<UPrimitiveComponent*>& OutObjects) const;
	void GetObjectsFrustumRecursive(const FFrustum& Frustum, TArray<UPrimitiveComponent*>& OutObjects) const;
	void DebugDrawRecursive(ULineBatchRenderer* Renderer, int32 CurrentDepth, int32 MaxDepth) const;
};

/**
 * @brief Octree - 3D 공간을 8개씩 분할하는 공간 분할 자료구조
 */
class FOctree : public UObject
{
	DECLARE_CLASS(FOctree, UObject)

public:
	FOctree();
	virtual ~FOctree();

	/** 초기화 */
	void Initialize(const FAABB& WorldBounds);
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
		float AverageObjectsPerLeaf = 0.0f;
		float LastRebuildTime = 0.0f;
	};
	FStats GetStats() const;

private:
	FOctreeNode* Root;
	FAABB WorldBounds;

	/** 업데이트 관리 */
	bool bNeedsRebuild;

	/** 전체 재구성 */
	void RebuildTree();
	void CollectAllObjects(TArray<UPrimitiveComponent*>& OutObjects) const;
	void CollectAllObjectsRecursive(FOctreeNode* Node, TArray<UPrimitiveComponent*>& OutObjects) const;
	void CalculateStats(FOctreeNode* Node, FStats& Stats) const;
};
