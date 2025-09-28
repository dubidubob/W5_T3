#include "pch.h"
#include "Math/BVH.h"
#include "Mesh/StaticMeshComponent.h"

void FBVH::Clear()
{
    Items.clear();
    Nodes.clear();
    Root = -1;
}

void FBVH::Build(const TArray<UStaticMeshComponent*>& Comps)
{
    Clear();
    Items.reserve(Comps.size());
    for (UStaticMeshComponent* C : Comps)
    {
        if (!C) continue;
        FAABB B = C->GetWorldBounds();
        if (!B.IsValid()) continue;
        FBVHItem It;
        It.Comp = C;
        It.Bounds = B;
        It.Centroid = B.GetCenter();
        Items.push_back(It);
    }
    if (Items.empty()) { Root = -1; return; }
    Nodes.reserve(Items.size() * 2);
    Root = BuildRange(0, static_cast<int32>(Items.size()), 0);
}

int32 FBVH::ChooseAxis(const FAABB& Bounds)
{
    FVector Ext = Bounds.GetExtent();
    if (Ext.X >= Ext.Y && Ext.X >= Ext.Z) return 0;
    if (Ext.Y >= Ext.X && Ext.Y >= Ext.Z) return 1;
    return 2;
}

int32 FBVH::BuildRange(int32 First, int32 Last, int32 Depth)
{
    FBVHNode Node;
    FAABB B;
    for (int32 i = First; i < Last; ++i) {
        B.AddAABB(Items[i].Bounds);
    }
    Node.Bounds = B;
    int32 Count = Last - First;
    if (Count <= LeafMax || Depth >= MaxDepth)
    {
        Node.bLeaf = true;
        Node.First = First;
        Node.Count = Count;
        int32 Index = static_cast<int32>(Nodes.size());
        Nodes.push_back(Node);
        return Index;
    }
    FAABB CB;
    for (int32 i = First; i < Last; ++i)
    {
        CB.AddPoint(Items[i].Centroid);
    }
    int Axis = ChooseAxis(CB);
    int32 Mid = (First + Last) / 2;
    auto ItBeg = Items.begin() + First;
    auto ItMid = Items.begin() + Mid;
    auto ItEnd = Items.begin() + Last;
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

void FBVH::AddSubtreeAll(int32 NodeIdx, TArray<bool>& OutVisible) const
{
    const FBVHNode& N = Nodes[NodeIdx];
    if (N.bLeaf)
    {
        for (int32 i = 0; i < N.Count; ++i)
        {
			OutVisible[N.First + i] = true;
        }
        return;
    }
    if (N.Left >= 0) AddSubtreeAll(N.Left, OutVisible);
    if (N.Right >= 0) AddSubtreeAll(N.Right, OutVisible);
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

void FBVH::QueryFrustum(const TStaticArray<FVector4, 6>& Planes, TArray<bool>& OutVisibles) const
{
	std::fill(OutVisibles.begin(), OutVisibles.end(), false);
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
            AddSubtreeAll(Idx, OutVisibles);
            continue;
        }

		//걸칠경우
        if (N.bLeaf)
        {
            for (int32 i = 0; i < N.Count; ++i)
            {
                const FBVHItem& It = Items[N.First + i];
				if (!AABBOutsideFrustum(It.Bounds, Planes)) OutVisibles[N.First + i] = true;
            }
        }
        else
        {
            if (N.Left >= 0) Stack.push_back(N.Left);
            if (N.Right >= 0) Stack.push_back(N.Right);
        }
    }
}
