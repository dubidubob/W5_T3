#include "pch.h"
#include "Math/BVH.h"
#include "Mesh/SceneComponent.h"
#include "Render/Renderer/LineBatchRenderer.h"
#include "Manager/Time/TimeManager.h"
#include <algorithm>

IMPLEMENT_CLASS(FBVH, UObject)

// =============================================================================
// FBVHNode Implementation
// =============================================================================

FBVHNode::~FBVHNode()
{
	delete LeftChild;
	delete RightChild;
}

void FBVHNode::AddObject(UPrimitiveComponent* Object)
{
	if (Object)
	{
		Objects.push_back(Object);
	}
}

void FBVHNode::CalculateBounds()
{
	if (Objects.empty())
	{
		Bounds = FAABB();
		return;
	}

	// 첫 번째 객체의 바운딩 박스로 초기화
	Bounds = Objects[0]->GetWorldBounds();

	// 모든 객체의 바운딩 박스를 포함하도록 확장
	for (size_t i = 1; i < Objects.size(); ++i)
	{
		FAABB ObjectBounds = Objects[i]->GetWorldBounds();
		if (ObjectBounds.IsValid())
		{
			Bounds.AddAABB(ObjectBounds);
		}
	}
}

int32 FBVHNode::GetLongestAxis() const
{
	FVector Size = Bounds.GetSize();

	if (Size.X >= Size.Y && Size.X >= Size.Z)
		return 0; // X축
	else if (Size.Y >= Size.Z)
		return 1; // Y축
	else
		return 2; // Z축
}

void FBVHNode::SortObjectsByAxis(int32 Axis)
{
	std::sort(Objects.begin(), Objects.end(),
		[Axis](UPrimitiveComponent* A, UPrimitiveComponent* B)
		{
			FVector CenterA = A->GetWorldBounds().GetCenter();
			FVector CenterB = B->GetWorldBounds().GetCenter();

			switch (Axis)
			{
			case 0: return CenterA.X < CenterB.X;
			case 1: return CenterA.Y < CenterB.Y;
			case 2: return CenterA.Z < CenterB.Z;
			default: return false;
			}
		});
}

int32 FBVHNode::FindBestSplit() const
{
	// 단순한 중점 분할 (향후 SAH로 개선 가능)
	return static_cast<int32>(Objects.size()) / 2;
}

void FBVHNode::Split(int32 MaxObjectsPerNode, int32 MaxDepth)
{
	// 종료 조건: 객체 수가 적거나 최대 깊이에 도달
	if (Objects.size() <= MaxObjectsPerNode || Depth >= MaxDepth)
	{
		return;
	}

	// 바운딩 박스 계산
	CalculateBounds();

	// 가장 긴 축을 기준으로 정렬
	int32 SplitAxis = GetLongestAxis();
	SortObjectsByAxis(SplitAxis);

	// 분할점 찾기
	int32 SplitIndex = FindBestSplit();
	if (SplitIndex <= 0 || SplitIndex >= Objects.size())
	{
		return; // 분할할 수 없음
	}

	// 자식 노드 생성
	LeftChild = new FBVHNode();
	RightChild = new FBVHNode();

	LeftChild->Depth = Depth + 1;
	RightChild->Depth = Depth + 1;

	// 객체를 자식 노드에 분배
	for (int32 i = 0; i < SplitIndex; ++i)
	{
		LeftChild->AddObject(Objects[i]);
	}

	for (int32 i = SplitIndex; i < Objects.size(); ++i)
	{
		RightChild->AddObject(Objects[i]);
	}

	// 자식 노드의 바운딩 박스 계산
	LeftChild->CalculateBounds();
	RightChild->CalculateBounds();

	// 현재 노드의 객체 목록 정리 (내부 노드는 객체를 직접 보유하지 않음)
	Objects.clear();

	// 자식 노드 재귀적으로 분할
	LeftChild->Split(MaxObjectsPerNode, MaxDepth);
	RightChild->Split(MaxObjectsPerNode, MaxDepth);

	// 자식 노드들의 바운딩 박스를 계산한 후 현재 노드 바운딩 박스 업데이트
	if (LeftChild->IsValid() && RightChild->IsValid())
	{
		Bounds = LeftChild->Bounds;
		Bounds.AddAABB(RightChild->Bounds);
	}
	else if (LeftChild->IsValid())
	{
		Bounds = LeftChild->Bounds;
	}
	else if (RightChild->IsValid())
	{
		Bounds = RightChild->Bounds;
	}
}

void FBVHNode::GetObjectsIntersectingRay(const FRay& Ray, TArray<UPrimitiveComponent*>& OutObjects) const
{
	// 바운딩 박스 유효성 검사
	if (!Bounds.IsValid())
	{
		UE_LOG("BVH Node has invalid bounds");
		return;
	}

	// 바운딩 박스와 레이 교차 검사 (Octree와 정확히 동일한 방식)
	bool bIntersects = Bounds.IntersectsRay(Ray.Origin, Ray.Direction);
	UE_LOG("BVH Node (Depth %d): Bounds=[%.2f,%.2f,%.2f]-[%.2f,%.2f,%.2f], Ray Intersects=%s",
		Depth,
		Bounds.Min.X, Bounds.Min.Y, Bounds.Min.Z,
		Bounds.Max.X, Bounds.Max.Y, Bounds.Max.Z,
		bIntersects ? "YES" : "NO");

	if (!bIntersects)
	{
		return;
	}

	if (IsLeaf())
	{
		// 리프 노드: 모든 객체 추가
		UE_LOG("BVH Leaf Node: Adding %d objects", static_cast<int32>(Objects.size()));
		for (UPrimitiveComponent* Object : Objects)
		{
			OutObjects.push_back(Object);
		}
	}
	else
	{
		// 내부 노드: 자식 노드 재귀 탐색
		UE_LOG("BVH Internal Node: Checking children (Left=%s, Right=%s)",
			LeftChild ? "Valid" : "NULL",
			RightChild ? "Valid" : "NULL");

		if (LeftChild)
			LeftChild->GetObjectsIntersectingRay(Ray, OutObjects);
		if (RightChild)
			RightChild->GetObjectsIntersectingRay(Ray, OutObjects);
	}
}

void FBVHNode::GetObjectsInFrustum(const FFrustum& Frustum, TArray<UPrimitiveComponent*>& OutObjects) const
{
	// 바운딩 박스 유효성 검사
	if (!Bounds.IsValid())
	{
		return;
	}

	// 바운딩 박스와 프러스텀 교차 검사
	bool bIntersects = Frustum.IntersectsAABB(Bounds);

	if (!bIntersects)
	{
		return;
	}

	if (IsLeaf())
	{
		// 리프 노드: 개별 객체와 프러스텀 교차 검사
		int32 AddedCount = 0;
		for (UPrimitiveComponent* Object : Objects)
		{
			FAABB ObjectBounds = Object->GetWorldBounds();
			if (Frustum.IntersectsAABB(ObjectBounds))
			{
				OutObjects.push_back(Object);
				AddedCount++;
			}
		}
	}
	else
	{
		if (LeftChild)
			LeftChild->GetObjectsInFrustum(Frustum, OutObjects);
		if (RightChild)
			RightChild->GetObjectsInFrustum(Frustum, OutObjects);
	}
}

void FBVHNode::GetObjectsIntersectingAABB(const FAABB& QueryBounds, TArray<UPrimitiveComponent*>& OutObjects) const
{
	// 바운딩 박스 교차 검사
	if (!Bounds.Intersects(QueryBounds))
	{
		return;
	}

	if (IsLeaf())
	{
		// 리프 노드: 개별 객체와 AABB 교차 검사
		for (UPrimitiveComponent* Object : Objects)
		{
			FAABB ObjectBounds = Object->GetWorldBounds();
			if (ObjectBounds.Intersects(QueryBounds))
			{
				OutObjects.push_back(Object);
			}
		}
	}
	else
	{
		// 내부 노드: 자식 노드 재귀 탐색
		if (LeftChild)
			LeftChild->GetObjectsIntersectingAABB(QueryBounds, OutObjects);
		if (RightChild)
			RightChild->GetObjectsIntersectingAABB(QueryBounds, OutObjects);
	}
}

void FBVHNode::CollectStats(int32& OutNodeCount, int32& OutLeafCount, int32& OutObjectCount, int32& OutMaxDepth) const
{
	OutNodeCount++;
	OutMaxDepth = std::max(OutMaxDepth, Depth);

	if (IsLeaf())
	{
		OutLeafCount++;
		OutObjectCount += static_cast<int32>(Objects.size());
	}
	else
	{
		if (LeftChild)
			LeftChild->CollectStats(OutNodeCount, OutLeafCount, OutObjectCount, OutMaxDepth);
		if (RightChild)
			RightChild->CollectStats(OutNodeCount, OutLeafCount, OutObjectCount, OutMaxDepth);
	}
}

void FBVHNode::DebugDraw(ULineBatchRenderer* Renderer, int32 CurrentDepth, int32 MaxDepthToDraw) const
{
	if (!Renderer || !Bounds.IsValid())
		return;

	if (MaxDepthToDraw >= 0 && CurrentDepth > MaxDepthToDraw)
		return;

	// 깊이에 따른 색상 설정 (Octree와 동일한 방식)
	FVector4 Color;
	switch (CurrentDepth % 6)
	{
	case 0: Color = FVector4(1.0f, 0.0f, 0.0f, 1.0f); break; // 빨강
	case 1: Color = FVector4(0.0f, 1.0f, 0.0f, 1.0f); break; // 초록
	case 2: Color = FVector4(0.0f, 0.0f, 1.0f, 1.0f); break; // 파랑
	case 3: Color = FVector4(1.0f, 1.0f, 0.0f, 1.0f); break; // 노랑
	case 4: Color = FVector4(1.0f, 0.0f, 1.0f, 1.0f); break; // 마젠타
	case 5: Color = FVector4(0.0f, 1.0f, 1.0f, 1.0f); break; // 시안
	}

	// 바운딩 박스 그리기 (Octree와 동일한 방식)
	Renderer->AddAABB(Bounds.Min, Bounds.Max, Color);

	// 자식 노드 그리기
	if (LeftChild)
		LeftChild->DebugDraw(Renderer, CurrentDepth + 1, MaxDepthToDraw);
	if (RightChild)
		RightChild->DebugDraw(Renderer, CurrentDepth + 1, MaxDepthToDraw);
}

// =============================================================================
// FBVH Implementation
// =============================================================================

FBVH::FBVH() : Root(nullptr), bNeedsRebuild(false)
{
}

FBVH::~FBVH()
{
	Clear();
}

void FBVH::Initialize(const FAABB& InWorldBounds)
{
	Clear();
	WorldBounds = InWorldBounds;
	Root = new FBVHNode();
	Root->Bounds = WorldBounds;
}

void FBVH::Clear()
{
	delete Root;
	Root = nullptr;
	bNeedsRebuild = false;
}

void FBVH::InsertObject(UPrimitiveComponent* Object)
{
	if (!Object)
		return;

	MarkForRebuild(); // BVH는 동적 삽입이 복잡하므로 재구성 스케줄링
}

void FBVH::RemoveObject(UPrimitiveComponent* Object)
{
	if (!Object)
		return;

	MarkForRebuild(); // BVH는 동적 제거가 복잡하므로 재구성 스케줄링
}

void FBVH::UpdateObject(UPrimitiveComponent* Object)
{
	if (!Object)
		return;

	MarkForRebuild(); // 객체 이동 시 재구성
}

void FBVH::QueryBounds(const FAABB& QueryBounds, TArray<UPrimitiveComponent*>& OutObjects) const
{
	if (!Root)
		return;

	Root->GetObjectsIntersectingAABB(QueryBounds, OutObjects);
}

void FBVH::QueryRay(const FRay& Ray, TArray<UPrimitiveComponent*>& OutObjects) const
{
	if (!Root)
	{
		UE_LOG("BVH QueryRay: Root is NULL");
		return;
	}

	UE_LOG("BVH QueryRay: Starting ray query - Origin=[%.2f,%.2f,%.2f], Direction=[%.2f,%.2f,%.2f]",
		Ray.Origin.X, Ray.Origin.Y, Ray.Origin.Z,
		Ray.Direction.X, Ray.Direction.Y, Ray.Direction.Z);

	int32 InitialCount = OutObjects.size();
	Root->GetObjectsIntersectingRay(Ray, OutObjects);
	int32 FinalCount = OutObjects.size();

	UE_LOG("BVH QueryRay: Found %d objects (added %d)", FinalCount, FinalCount - InitialCount);
}

void FBVH::QueryFrustum(const FFrustum& Frustum, TArray<UPrimitiveComponent*>& OutObjects) const
{
	if (!Root)
	{
		return;
	}

	int32 InitialCount = OutObjects.size();
	Root->GetObjectsInFrustum(Frustum, OutObjects);
	int32 FinalCount = OutObjects.size();
}

TArray<UPrimitiveComponent*> FBVH::QueryFrustum(const FFrustum& Frustum) const
{
	TArray<UPrimitiveComponent*> Result;
	QueryFrustum(Frustum, Result);
	return Result;
}

void FBVH::ConditionalUpdate()
{
	if (bNeedsRebuild)
	{
		ForceRebuild();
	}
}

void FBVH::ForceRebuild()
{
	if (!Root)
		return;

	float StartTime = UTimeManager::GetInstance().GetGameTime();

	// 모든 객체 수집
	TArray<UPrimitiveComponent*> AllObjects;
	CollectAllObjects(AllObjects);

	// 트리 재구성
	delete Root;
	Root = new FBVHNode();

	// 모든 객체를 루트 노드에 추가
	for (UPrimitiveComponent* Object : AllObjects)
	{
		Root->AddObject(Object);
	}

	// 바운딩 박스 계산 및 분할
	if (!AllObjects.empty())
	{
		Root->CalculateBounds();
		Root->Split(MAX_OBJECTS_PER_NODE, MAX_TREE_DEPTH);
	}

	bNeedsRebuild = false;

	float EndTime = UTimeManager::GetInstance().GetGameTime();
	float BuildTime = EndTime - StartTime;

	UE_LOG("BVH rebuilt in %.6f seconds with %d objects",
		BuildTime, static_cast<int32>(AllObjects.size()));
}

void FBVH::RebuildWithObjects(const TArray<UPrimitiveComponent*>& Objects)
{
	float StartTime = UTimeManager::GetInstance().GetGameTime();

	// 기존 트리 삭제
	delete Root;
	Root = new FBVHNode();

	// 객체가 없으면 빈 트리로 유지
	if (Objects.empty())
	{
		bNeedsRebuild = false;
		UE_LOG("BVH: No objects to build with");
		return;
	}

	// 모든 객체를 루트 노드에 추가
	for (UPrimitiveComponent* Object : Objects)
	{
		if (Object)
		{
			Root->AddObject(Object);
		}
	}

	// 바운딩 박스 계산 및 분할
	Root->CalculateBounds();
	Root->Split(MAX_OBJECTS_PER_NODE, MAX_TREE_DEPTH);

	bNeedsRebuild = false;

	float EndTime = UTimeManager::GetInstance().GetGameTime();
	float BuildTime = EndTime - StartTime;

	UE_LOG("BVH: Successfully built with %d objects in %.6f seconds",
		static_cast<int32>(Objects.size()), BuildTime);
}

int32 FBVH::GetTotalObjectCount() const
{
	if (!Root)
		return 0;

	int32 NodeCount = 0, LeafCount = 0, ObjectCount = 0, MaxDepth = 0;
	Root->CollectStats(NodeCount, LeafCount, ObjectCount, MaxDepth);
	return ObjectCount;
}

int32 FBVH::GetMaxDepth() const
{
	if (!Root)
		return 0;

	int32 NodeCount = 0, LeafCount = 0, ObjectCount = 0, MaxDepth = 0;
	Root->CollectStats(NodeCount, LeafCount, ObjectCount, MaxDepth);
	return MaxDepth;
}

void FBVH::DebugDraw(ULineBatchRenderer* Renderer, int32 MaxDepthToDraw) const
{
	if (!Root || !Renderer)
	{
		UE_LOG("BVH DebugDraw failed: Root=%s, Renderer=%s", Root ? "Valid" : "NULL", Renderer ? "Valid" : "NULL");
		return;
	}

	Root->DebugDraw(Renderer, 0, MaxDepthToDraw);
}

FBVH::FStats FBVH::GetStats() const
{
	FStats Stats;

	if (Root)
	{
		Root->CollectStats(Stats.TotalNodes, Stats.LeafNodes, Stats.TotalObjects, Stats.MaxDepth);

		if (Stats.LeafNodes > 0)
		{
			Stats.AverageObjectsPerLeaf = static_cast<float>(Stats.TotalObjects) / Stats.LeafNodes;
		}
	}

	return Stats;
}

void FBVH::RebuildTree()
{
	ForceRebuild();
}

void FBVH::CollectAllObjects(TArray<UPrimitiveComponent*>& OutObjects) const
{
	if (Root)
	{
		CollectAllObjectsRecursive(Root, OutObjects);
	}
}

void FBVH::CollectAllObjectsRecursive(FBVHNode* Node, TArray<UPrimitiveComponent*>& OutObjects) const
{
	if (!Node)
		return;

	// 리프 노드의 객체들 수집
	for (UPrimitiveComponent* Object : Node->Objects)
	{
		OutObjects.push_back(Object);
	}

	// 자식 노드 재귀 탐색
	if (Node->LeftChild)
		CollectAllObjectsRecursive(Node->LeftChild, OutObjects);
	if (Node->RightChild)
		CollectAllObjectsRecursive(Node->RightChild, OutObjects);
}
