#include "pch.h"
#include "Math/Octree.h"
#include "Math/Frustum.h"
#include "Mesh/SceneComponent.h"
#include "Render/Renderer/LineBatchRenderer.h"
#include "Manager/Time/TimeManager.h"
#include "Manager/Level/LevelManager.h"

IMPLEMENT_CLASS(FOctree, UObject)

// =============================================================================
// FOctreeNode Implementation
// =============================================================================

FOctreeNode::FOctreeNode()
	: Bounds()
	, Depth(0)
{
	for (int32 i = 0; i < 8; ++i)
	{
		Children[i] = nullptr;
	}
}

FOctreeNode::FOctreeNode(const FAABB& InBounds, int32 InDepth)
	: Bounds(InBounds)
	, Depth(InDepth)
{
	for (int32 i = 0; i < 8; ++i)
	{
		Children[i] = nullptr;
	}
}

FOctreeNode::~FOctreeNode()
{
	Clear();
}

void FOctreeNode::Clear()
{
	Objects.Empty();

	for (int32 i = 0; i < 8; ++i)
	{
		if (Children[i])
		{
			delete Children[i];
			Children[i] = nullptr;
		}
	}
}

bool FOctreeNode::ShouldSubdivide() const
{
	// 객체수가 최대치 초과, 깊이 제한 미달, 리프 노드인 경우 분할
	return Objects.Num() > MAX_OBJECTS_PER_NODE &&
		   Depth < MAX_DEPTH &&
		   IsLeaf();
}

void FOctreeNode::Subdivide()
{
	if (!IsLeaf() || Depth >= MAX_DEPTH)
	{
		return;
	}

	// 8개 자식 노드 생성
	for (int32 i = 0; i < 8; ++i)
	{
		FAABB ChildBounds = GetChildBounds(i);
		Children[i] = new FOctreeNode(ChildBounds, Depth + 1);
	}

	// 기존 객체들을 자식 노드에 재배치
	TArray<UPrimitiveComponent*> CurrentObjects = Objects;
	Objects.Empty();

	for (UPrimitiveComponent* Object : CurrentObjects)
	{
		InsertObjectRecursive(Object);
	}
}

void FOctreeNode::InsertObject(UPrimitiveComponent* Object)
{
	if (!Object)
	{
		return;
	}

	FAABB ObjectBounds = Object->GetWorldBounds();

	// 객체가 이 노드 경계에 완전히 포함되는지 확인
	if (!Bounds.Intersects(ObjectBounds))
	{
		return;
	}

	InsertObjectRecursive(Object);
}

void FOctreeNode::InsertObjectRecursive(UPrimitiveComponent* Object)
{
	if (IsLeaf())
	{
		Objects.Add(Object);

		// 분할 조건 확인
		if (ShouldSubdivide())
		{
			Subdivide();
		}
	}
	else
	{
		// 자식 노드에 삽입 시도
		FAABB ObjectBounds = Object->GetWorldBounds();
		bool bInserted = false;

		for (int32 i = 0; i < 8; ++i)
		{
			if (Children[i] && Children[i]->GetBounds().Intersects(ObjectBounds))
			{
				Children[i]->InsertObjectRecursive(Object);
				bInserted = true;
			}
		}

		// 여러 자식에 걸쳐있으면 이 노드에 저장
		if (!bInserted)
		{
			Objects.Add(Object);
		}
	}
}

void FOctreeNode::RemoveObject(UPrimitiveComponent* Object)
{
	Objects.Remove(Object);

	if (!IsLeaf())
	{
		for (int32 i = 0; i < 8; ++i)
		{
			if (Children[i])
			{
				Children[i]->RemoveObject(Object);
			}
		}
	}
}

void FOctreeNode::GetObjectsInBounds(const FAABB& QueryBounds, TArray<UPrimitiveComponent*>& OutObjects) const
{
	if (!Bounds.Intersects(QueryBounds))
	{
		return;
	}

	// 이 노드의 객체들 추가
	for (UPrimitiveComponent* Object : Objects)
	{
		if (Object && QueryBounds.Intersects(Object->GetWorldBounds()))
		{
			OutObjects.AddUnique(Object);
		}
	}

	// 자식 노드들 재귀 탐색
	GetObjectsRecursive(QueryBounds, OutObjects);
}

void FOctreeNode::GetObjectsRecursive(const FAABB& QueryBounds, TArray<UPrimitiveComponent*>& OutObjects) const
{
	if (!IsLeaf())
	{
		for (int32 i = 0; i < 8; ++i)
		{
			if (Children[i])
			{
				Children[i]->GetObjectsInBounds(QueryBounds, OutObjects);
			}
		}
	}
}

void FOctreeNode::GetObjectsIntersectingRay(const FRay& Ray, TArray<UPrimitiveComponent*>& OutObjects) const
{
	// 레이가 이 노드와 교차하는지 확인
	if (!Bounds.IntersectsRay(Ray.Origin, Ray.Direction))
	{
		return;
	}

	// 이 노드의 객체들 추가
	for (UPrimitiveComponent* Object : Objects)
	{
		if (Object)
		{
			OutObjects.AddUnique(Object);
		}
	}

	// 자식 노드들 재귀 탐색
	GetObjectsRayRecursive(Ray, OutObjects);
}

void FOctreeNode::GetObjectsInFrustum(const FFrustum& Frustum, TArray<UPrimitiveComponent*>& OutObjects) const
{
	// 프러스텀이 이 노드와 교차하는지 확인
	if (!Frustum.IntersectsAABB(Bounds))
	{
		return;
	}

	// 이 노드의 객체들 추가 (프러스텀과 교차하는 것만)
	for (UPrimitiveComponent* Object : Objects)
	{
		if (Object)
		{
			FAABB ObjectBounds = Object->GetWorldBounds();
			if (Frustum.IntersectsAABB(ObjectBounds))
			{
				OutObjects.AddUnique(Object);
			}
		}
	}

	// 자식 노드들 재귀 탐색
	GetObjectsFrustumRecursive(Frustum, OutObjects);
}

void FOctreeNode::GetObjectsRayRecursive(const FRay& Ray, TArray<UPrimitiveComponent*>& OutObjects) const
{
	if (!IsLeaf())
	{
		for (int32 i = 0; i < 8; ++i)
		{
			if (Children[i])
			{
				Children[i]->GetObjectsIntersectingRay(Ray, OutObjects);
			}
		}
	}
}

void FOctreeNode::GetObjectsFrustumRecursive(const FFrustum& Frustum, TArray<UPrimitiveComponent*>& OutObjects) const
{
	if (!IsLeaf())
	{
		for (int32 i = 0; i < 8; ++i)
		{
			if (Children[i])
			{
				Children[i]->GetObjectsInFrustum(Frustum, OutObjects);
			}
		}
	}
}

int32 FOctreeNode::GetChildIndex(const FVector& Point) const
{
	FVector Center = Bounds.GetCenter();
	int32 Index = 0;

	if (Point.X >= Center.X) Index |= 1;
	if (Point.Y >= Center.Y) Index |= 2;
	if (Point.Z >= Center.Z) Index |= 4;

	return Index;
}

FAABB FOctreeNode::GetChildBounds(int32 ChildIndex) const
{
	FVector Center = Bounds.GetCenter();
	FVector Min = Bounds.Min;
	FVector Max = Bounds.Max;

	FVector ChildMin, ChildMax;

	// X축
	if (ChildIndex & 1)
	{
		ChildMin.X = Center.X;
		ChildMax.X = Max.X;
	}
	else
	{
		ChildMin.X = Min.X;
		ChildMax.X = Center.X;
	}

	// Y축
	if (ChildIndex & 2)
	{
		ChildMin.Y = Center.Y;
		ChildMax.Y = Max.Y;
	}
	else
	{
		ChildMin.Y = Min.Y;
		ChildMax.Y = Center.Y;
	}

	// Z축
	if (ChildIndex & 4)
	{
		ChildMin.Z = Center.Z;
		ChildMax.Z = Max.Z;
	}
	else
	{
		ChildMin.Z = Min.Z;
		ChildMax.Z = Center.Z;
	}

	return FAABB(ChildMin, ChildMax);
}

void FOctreeNode::DebugDraw(ULineBatchRenderer* Renderer, int32 MaxDepthToDraw) const
{
	if (!Renderer)
	{
		return;
	}

	DebugDrawRecursive(Renderer, Depth, MaxDepthToDraw);
}

void FOctreeNode::DebugDrawRecursive(ULineBatchRenderer* Renderer, int32 CurrentDepth, int32 MaxDepth) const
{
	// 깊이 제한 체크
	if (MaxDepth >= 0 && CurrentDepth > MaxDepth)
	{
		return;
	}

	// 깊이별 색상 설정
	FVector4 Colors[] = {
		{1.0f, 0.0f, 0.0f, 1.0f}, // 빨강 - Root
		{0.0f, 1.0f, 0.0f, 1.0f}, // 초록 - Depth 1
		{0.0f, 0.0f, 1.0f, 1.0f}, // 파랑 - Depth 2
		{1.0f, 1.0f, 0.0f, 1.0f}, // 노랑 - Depth 3
		{1.0f, 0.0f, 1.0f, 1.0f}, // 마젠타 - Depth 4
		{0.0f, 1.0f, 1.0f, 1.0f}, // 시안 - Depth 5
		{1.0f, 1.0f, 1.0f, 1.0f}, // 흰색 - Depth 6
		{0.5f, 0.5f, 0.5f, 1.0f}  // 회색 - Depth 7+
	};

	int32 ColorIndex = std::min(CurrentDepth, 7);
	FVector4 Color = Colors[ColorIndex];

	//UE_LOG("Drawing node at depth %d with bounds: Min(%.1f,%.1f,%.1f) Max(%.1f,%.1f,%.1f)",
	//	CurrentDepth, Bounds.Min.X, Bounds.Min.Y, Bounds.Min.Z, Bounds.Max.X, Bounds.Max.Y, Bounds.Max.Z);

	// 이 노드의 경계 박스 그리기
	Renderer->AddAABB(Bounds.Min, Bounds.Max, Color);

	// 자식 노드들 재귀 그리기
	if (!IsLeaf())
	{
		for (int32 i = 0; i < 8; ++i)
		{
			if (Children[i])
			{
				Children[i]->DebugDrawRecursive(Renderer, CurrentDepth + 1, MaxDepth);
			}
		}
	}
}

// =============================================================================
// FOctree Implementation
// =============================================================================

FOctree::FOctree()
	: Root(nullptr)
	, bNeedsRebuild(false)
{
}

FOctree::~FOctree()
{
	Clear();
}

void FOctree::Initialize(const FAABB& InWorldBounds)
{
	Clear();
	WorldBounds = InWorldBounds;
	Root = new FOctreeNode(WorldBounds, 0);
	bNeedsRebuild = false;
}

void FOctree::Clear()
{
	if (Root)
	{
		delete Root;
		Root = nullptr;
	}
	bNeedsRebuild = false;
}

void FOctree::InsertObject(UPrimitiveComponent* Object)
{
	if (!Root || !Object)
	{
		return;
	}

	Root->InsertObject(Object);
}

void FOctree::RemoveObject(UPrimitiveComponent* Object)
{
	if (!Root || !Object)
	{
		return;
	}

	Root->RemoveObject(Object);
}

void FOctree::UpdateObject(UPrimitiveComponent* Object)
{
	// 간단한 업데이트: 제거 후 다시 삽입
	RemoveObject(Object);
	InsertObject(Object);
}

void FOctree::QueryBounds(const FAABB& QueryBounds, TArray<UPrimitiveComponent*>& OutObjects) const
{
	if (!Root)
	{
		return;
	}

	Root->GetObjectsInBounds(QueryBounds, OutObjects);
}

void FOctree::QueryRay(const FRay& Ray, TArray<UPrimitiveComponent*>& OutObjects) const
{
	if (!Root)
	{
		return;
	}

	Root->GetObjectsIntersectingRay(Ray, OutObjects);
}

void FOctree::QueryFrustum(const FFrustum& Frustum, TArray<UPrimitiveComponent*>& OutObjects) const
{
	if (!Root)
	{
		return;
	}

	Root->GetObjectsInFrustum(Frustum, OutObjects);
}

TArray<UPrimitiveComponent*> FOctree::QueryFrustum(const FFrustum& Frustum) const
{
	TArray<UPrimitiveComponent*> Result;
	QueryFrustum(Frustum, Result);
	return Result;
}

void FOctree::ConditionalUpdate()
{
	float CurrentTime = UTimeManager::GetInstance().GetGameTime();

	if (bNeedsRebuild)
	{
		ForceRebuild();
	}
}

void FOctree::ForceRebuild()
{
	if (!Root)
	{
		return;
	}

	// 모든 객체 수집
	TArray<UPrimitiveComponent*> AllObjects;
	CollectAllObjects(AllObjects);

	// 트리 초기화
	Root->Clear();

	// 객체들 다시 삽입
	for (UPrimitiveComponent* Object : AllObjects)
	{
		if (Object)
		{
			Root->InsertObject(Object);
		}
	}

	bNeedsRebuild = false;
}

void FOctree::RebuildTree()
{
	ForceRebuild();
}

void FOctree::CollectAllObjects(TArray<UPrimitiveComponent*>& OutObjects) const
{
	if (!Root)
	{
		return;
	}

	// 루트부터 모든 객체 수집 (재귀적으로)
	CollectAllObjectsRecursive(Root, OutObjects);
}

void FOctree::CollectAllObjectsRecursive(FOctreeNode* Node, TArray<UPrimitiveComponent*>& OutObjects) const
{
	if (!Node)
	{
		return;
	}

	// 이 노드의 객체들 추가
	OutObjects.Append(Node->GetObjects());

	// 자식 노드들 재귀 처리
	if (!Node->IsLeaf())
	{
		for (int32 i = 0; i < 8; ++i)
		{
			if (Node->Children[i])
			{
				CollectAllObjectsRecursive(Node->Children[i], OutObjects);
			}
		}
	}
}

int32 FOctree::GetTotalObjectCount() const
{
	if (!Root)
	{
		return 0;
	}

	TArray<UPrimitiveComponent*> AllObjects;
	CollectAllObjects(AllObjects);
	return AllObjects.Num();
}

int32 FOctree::GetMaxDepth() const
{
	// TODO: 트리의 최대 깊이 계산
	return FOctreeNode::MAX_DEPTH;
}

void FOctree::DebugDraw(ULineBatchRenderer* Renderer, int32 MaxDepthToDraw) const
{
	if (!Root || !Renderer)
	{
		UE_LOG("DebugDraw failed: Root=%s, Renderer=%s", Root ? "Valid" : "NULL", Renderer ? "Valid" : "NULL");
		return;
	}

	//UE_LOG("Octree DebugDraw: Drawing octree with max depth %d", MaxDepthToDraw);
	Root->DebugDraw(Renderer, MaxDepthToDraw);
}

FOctree::FStats FOctree::GetStats() const
{
	FStats Stats;

	if (Root)
	{
		CalculateStats(Root, Stats);
	}

	return Stats;
}

void FOctree::CalculateStats(FOctreeNode* Node, FStats& Stats) const
{
	if (!Node)
	{
		return;
	}

	Stats.TotalNodes++;
	Stats.TotalObjects += Node->GetObjectCount();

	if (Node->IsLeaf())
	{
		Stats.LeafNodes++;
	}
	else
	{
		// 자식 노드들 재귀 계산
		for (int32 i = 0; i < 8; ++i)
		{
			if (Node->Children[i])
			{
				CalculateStats(Node->Children[i], Stats);
			}
		}
	}

	// 평균 계산
	if (Stats.LeafNodes > 0)
	{
		Stats.AverageObjectsPerLeaf = static_cast<float>(Stats.TotalObjects) / Stats.LeafNodes;
	}
}
