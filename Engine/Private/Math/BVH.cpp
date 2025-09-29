#include "pch.h"
#include "Math/BVH.h"
#include "Mesh/StaticMeshComponent.h"

void FBVH::Clear()
{
    Items.clear();
    Nodes.clear();
    Root = -1;
}

void FBVH::Build(const TArray<UStaticMeshComponent*>& Components)
{
    Clear();
    Items.reserve(Components.size());
	int Idx = 0;
    for (UStaticMeshComponent* Component : Components)
    {
        if (!Component) continue;
        FAABB BoundingBox = Component->GetWorldBounds();
        if (!BoundingBox.IsValid()) continue;
        FBVHItem CandidateItem;
        CandidateItem.Comp = Component;
        CandidateItem.Bounds = BoundingBox;
        CandidateItem.Centroid = BoundingBox.GetCenter();
        Items.push_back(CandidateItem);
    }
    if (Items.empty()) { Root = -1; return; }
    Nodes.reserve(Items.size() * 2);

    Root = BuildRange(0, static_cast<int32>(Items.size()), 0);
}

int32 FBVH::BuildRange(int32 First, int32 Last, int32 Depth)
{
    FBVHNode Node;
	// Create new AABB that Include First ~ Last Objects
    FAABB BoundingBox;
    for (int32 i = First; i < Last; ++i) {
        BoundingBox.AddAABB(Items[i].Bounds);
    }
    Node.Bounds = BoundingBox;


    int32 Count = Last - First;
	// Make Leaf Node
    if (Count <= LeafMax || Depth >= MaxDepth)
    {
        Node.bLeaf = true;
        Node.First = First;
        Node.Count = Count;
        int32 Index = static_cast<int32>(Nodes.size());
        Nodes.push_back(Node);
        return Index; // Return to Parent with Their Index
    }

	// Midian Split for Choose Axis
    FAABB CentroidBoundingBox;
    for (int32 i = First; i < Last; ++i)
    {
        CentroidBoundingBox.AddPoint(Items[i].Centroid);
    }
	// Choose Most Longest Axis
    int Axis = ChooseAxis(CentroidBoundingBox);

    int32 Mid = (First + Last) / 2;
    auto ItBeg = Items.begin() + First;
    auto ItMid = Items.begin() + Mid;
    auto ItEnd = Items.begin() + Last;

	// Midian is In ItMid after nth_element Executed, Partial reorder A : Small than Mid, B : Large than Mid
    if (Axis == 0)
    {
        std::nth_element(ItBeg, ItMid, ItEnd, [](const FBVHItem& A, const FBVHItem& B){ return A.Centroid.X < B.Centroid.X; });
    }
    else if (Axis == 1)
    {
        std::nth_element(ItBeg, ItMid, ItEnd, [](const FBVHItem& A, const FBVHItem& B){ return A.Centroid.Y < B.Centroid.Y; });
    }
    else
    {
        std::nth_element(ItBeg, ItMid, ItEnd, [](const FBVHItem& A, const FBVHItem& B){ return A.Centroid.Z < B.Centroid.Z; });
    }


    int32 Index = static_cast<int32>(Nodes.size());
    Nodes.push_back(FBVHNode());
    int32 L = BuildRange(First, Mid, Depth + 1);
    int32 R = BuildRange(Mid, Last, Depth + 1);
    Nodes[Index].Bounds = Nodes[L].Bounds + Nodes[R].Bounds;
    Nodes[Index].Left = L;
    Nodes[Index].Right = R;
    return Index;
}


int32 FBVH::ChooseAxis(const FAABB& Bounds)
{
	FVector Extent = Bounds.GetExtent();

	if (Extent.X >= Extent.Y && Extent.X >= Extent.Z) return 0; // Choose X
	if (Extent.Y >= Extent.X && Extent.Y >= Extent.Z) return 1; // Choose Y
	return 2; // Choose Z
}

bool FBVH::AABBContains(const FAABB& Outer, const FAABB& Inner)
{
    return Outer.Min.X <= Inner.Min.X && Outer.Min.Y <= Inner.Min.Y && Outer.Min.Z <= Inner.Min.Z &&
           Outer.Max.X >= Inner.Max.X && Outer.Max.Y >= Inner.Max.Y && Outer.Max.Z >= Inner.Max.Z;
}

void FBVH::AddSubtree(int32 NodeIdx, TArray<UStaticMeshComponent*>& Out) const
{
    const FBVHNode& N = Nodes[NodeIdx];
    if (N.bLeaf)
    {
        for (int32 i = 0; i < N.Count; ++i)
        {
            Out.push_back(Items[N.First + i].Comp);
        }
        return;
    }
    if (N.Left >= 0) AddSubtree(N.Left, Out);
    if (N.Right >= 0) AddSubtree(N.Right, Out);
}

void FBVH::QueryAABB(const FAABB& Q, TArray<UStaticMeshComponent*>& Out) const
{
    Out.clear();
    if (Root < 0) return;
    TArray<int32> Stack;
    Stack.push_back(Root);
    while (!Stack.empty())
    {
        int32 Idx = Stack.back();
        Stack.pop_back();
        const FBVHNode& N = Nodes[Idx];
        if (!N.Bounds.Intersects(Q)) continue;
        if (AABBContains(Q, N.Bounds))
        {
            AddSubtree(Idx, Out);
            continue;
        }
        if (N.bLeaf)
        {
            for (int32 i = 0; i < N.Count; ++i)
            {
                const FBVHItem& It = Items[N.First + i];
                if (It.Bounds.Intersects(Q)) Out.push_back(It.Comp);
            }
        }
        else
        {
            if (N.Left >= 0) Stack.push_back(N.Left);
            if (N.Right >= 0) Stack.push_back(N.Right);
        }
    }
}

void FBVH::AddSubtreeAll(int32 NodeIdx, TArray<UStaticMeshComponent*>& Out) const
{
    const FBVHNode& N = Nodes[NodeIdx];
    if (N.bLeaf)
    {
        for (int32 i = 0; i < N.Count; ++i)
        {
			Out.Push(Items[N.First + i].Comp);
        }
        return;
    }
    if (N.Left >= 0) AddSubtreeAll(N.Left, Out);
    if (N.Right >= 0) AddSubtreeAll(N.Right, Out);
}

bool FBVH::AABBOutsideFrustum(const FAABB& B, const TStaticArray<FVector4,6>& Planes)
{
    const FVector c = (B.Min + B.Max) * 0.5f;
    const FVector e = (B.Max - B.Min) * 0.5f;
    for (const auto& P : Planes)
    {
        const float r = std::fabs(P.X) * e.X + std::fabs(P.Y) * e.Y + std::fabs(P.Z) * e.Z;
        const float s = P.X * c.X + P.Y * c.Y + P.Z * c.Z + P.W;
        if (s + r < 0.0f) return true;
    }
    return false;
}

bool FBVH::AABBInsideFrustum(const FAABB& B, const TStaticArray<FVector4,6>& Planes)
{
    const FVector c = (B.Min + B.Max) * 0.5f;
    const FVector e = (B.Max - B.Min) * 0.5f;
    for (const auto& P : Planes)
    {
        const float r = std::fabs(P.X) * e.X + std::fabs(P.Y) * e.Y + std::fabs(P.Z) * e.Z;
        const float s = P.X * c.X + P.Y * c.Y + P.Z * c.Z + P.W;
        if (s - r < 0.0f) return false;
    }
    return true;
}

void FBVH::QueryFrustum(const TStaticArray<FVector4, 6>& Planes, TArray<UStaticMeshComponent*>& Out) const
{
	Out.clear();
    if (Root < 0) return;
    TArray<int32> Stack;
    Stack.push_back(Root);
    while (!Stack.empty())
    {
        int32 Idx = Stack.back();
        Stack.pop_back();
        const FBVHNode& N = Nodes[Idx];
		if (AABBOutsideFrustum(N.Bounds, Planes))
		{
			continue;
		}
        if (AABBInsideFrustum(N.Bounds, Planes))
        {
            AddSubtreeAll(Idx, Out);
            continue;
        }

		//걸칠경우
        if (N.bLeaf)
        {
            for (int32 i = 0; i < N.Count; ++i)
            {
                const FBVHItem& It = Items[N.First + i];
				if (!AABBOutsideFrustum(It.Bounds, Planes))
				{
					Out.Push(It.Comp);
				}
            }
        }
        else
        {
            if (N.Left >= 0) Stack.push_back(N.Left);
            if (N.Right >= 0) Stack.push_back(N.Right);
        }
    }
}

static inline bool IntersectAABB(const FRay& Ray, const FAABB& b, float tMax, float& t0, float& t1)
{
	const FVector invD{ 1.0f / Ray.Direction.X, 1.0f / Ray.Direction.Y, 1.0f / Ray.Direction.Z };

	const FVector MinBound = b.Min - FVector{ Ray.Origin.X, Ray.Origin.Y, Ray.Origin.Z };
	const FVector MaxBound = b.Max - FVector{ Ray.Origin.X, Ray.Origin.Y, Ray.Origin.Z };

	const FVector tminv = FVector(MinBound.X * invD.X, MinBound.Y * invD.Y, MinBound.Z * invD.Z);
	const FVector tmaxv = FVector(MaxBound.X * invD.X, MaxBound.Y * invD.Y, MaxBound.Z * invD.Z);
	const FVector t1v{ std::min(tminv.X, tmaxv.X),
					   std::min(tminv.Y, tmaxv.Y),
					   std::min(tminv.Z, tmaxv.Z) };
	const FVector t2v{ std::max(tminv.X, tmaxv.X),
					   std::max(tminv.Y, tmaxv.Y),
					   std::max(tminv.Z, tmaxv.Z) };
	t0 = std::max(std::max(t1v.X, t1v.Y), t1v.Z);
	t1 = std::min(std::min(t2v.X, t2v.Y), t2v.Z);
	return (t1 >= t0) && (t0 <= tMax) && (t1 >= 0.0f);
}

static inline bool RayBoxEntry(const FAABB& Box, const FRay& Ray, float& OutTNear)
{
	float TEntry = -1.0f;
	const bool Hit = Box.IntersectsRay(Ray.Origin, Ray.Direction, &TEntry);
	if (!Hit) return false;

	OutTNear = (TEntry >= 0.0f) ? TEntry : 0.0f; // 내부 시작 → 0으로 간주
	return true;
}

struct FCand { UStaticMeshComponent* Comp; float TNear; };

void FBVH::QueryRayCandidates(const FRay& Ray, int MaxK, TArray<UStaticMeshComponent*>& Out)
{
	Out.clear();
	if (Root < 0) return;

	float TMaxGlobal = FLT_MAX;

	struct FStackItem { int32 NodeIdx; float TNear; };
	TArray<FStackItem> Stack; Stack.reserve(64);
	Stack.push_back({ Root, 0.0f });

	TArray<FCand> Cands; Cands.reserve(MaxK > 0 ? MaxK * 2 : 64);

	while (!Stack.empty())
	{
		const FStackItem It = Stack.back();
		Stack.pop_back();
		const FBVHNode& Node = Nodes[It.NodeIdx];

		// 노드 박스와 레이 교차: front-to-back 순서를 위해 T0(엔트리), T1(출구) 확보
		float N0 = 0.0f, N1 = 0.0f;
		if (!IntersectAABB(Ray, Node.Bounds, TMaxGlobal, N0, N1)) continue;

		if (Node.bLeaf)
		{
			// 리프: 각 아이템 AABB 교차 → 후보 수집
			for (int i = 0; i < Node.Count; ++i)
			{
				const FBVHItem& Item = Items[Node.First + i];
				float TNearItem;
				if (RayBoxEntry(Item.Bounds, Ray, TNearItem))
				{
					if (TNearItem <= TMaxGlobal)
						Cands.push_back({ Item.Comp, TNearItem });
				}
			}
		}
		else
		{
			// 자식들 front-to-back 방문 (가까운 쪽이 먼저 팝되도록 먼 쪽을 먼저 푸시)
			int32 L = Node.Left, R = Node.Right;
			float L0 = 0.f, L1 = 0.f, R0 = 0.f, R1 = 0.f;
			bool HL = false, HR = false;

			if (L >= 0) HL = IntersectAABB(Ray, Nodes[L].Bounds, TMaxGlobal, L0, L1);
			if (R >= 0) HR = IntersectAABB(Ray, Nodes[R].Bounds, TMaxGlobal, R0, R1);

			if (HL && HR)
			{
				if (L0 > R0) { std::swap(L, R); std::swap(L0, R0); }
				Stack.push_back({ R, R0 });
				Stack.push_back({ L, L0 });
			}
			else if (HL)
			{
				Stack.push_back({ L, L0 });
			}
			else if (HR)
			{
				Stack.push_back({ R, R0 });
			}
		}
	}

	// 거리순 정렬 + K개 컷
	std::sort(Cands.begin(), Cands.end(),
		[](const FCand& A, const FCand& B) { return A.TNear < B.TNear; });

	if (MaxK > 0 && (int)Cands.size() > MaxK)
		Cands.resize(MaxK);

	Out.reserve(Cands.size());
	for (auto& C : Cands)
		Out.push_back(C.Comp);
}

//void FBVH::QueryRayMBVH(const FRay& ray)
//{
//	각 Leaf Node에 대해 Mesh triangle을 쪼갠 다음, (매번 쪼개야하나?)
//	Mesh triangle의 리프가 됐을 때 hit 체크를 해야하나?
//}
