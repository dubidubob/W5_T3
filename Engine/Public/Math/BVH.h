#pragma once

#include "Global/Types.h"
#include "Global/Vector.h"
#include "Math/AABB.h"
#include <algorithm>

class UStaticMeshComponent;

struct FBVHItem
{
    UStaticMeshComponent* Comp = nullptr;
    FAABB Bounds;
    FVector Centroid;
};

struct FBVHNode
{
    FAABB Bounds;
    int32 Left = -1;
    int32 Right = -1;
    int32 First = 0;
    int32 Count = 0;
    bool bLeaf = false;
};

class FBVH
{
public:
    void Build(const TArray<UStaticMeshComponent*>& Comps);
    void QueryAABB(const FAABB& Q, TArray<UStaticMeshComponent*>& Out) const;
    void QueryFrustum(const TStaticArray<FVector4, 6>& Planes, TArray<UStaticMeshComponent*>& Out) const;
    void Clear();

private:
    int32 BuildRange(int32 First, int32 Last, int32 Depth);
    void AddSubtree(int32 NodeIdx, TArray<UStaticMeshComponent*>& Out) const;
    void AddSubtreeAll(int32 NodeIdx, TArray<UStaticMeshComponent*>& Out) const;
    static bool AABBOutsideFrustum(const FAABB& B, const TStaticArray<FVector4,6>& Planes);
    static bool AABBInsideFrustum(const FAABB& B, const TStaticArray<FVector4,6>& Planes);
    static int32 ChooseAxis(const FAABB& Bounds);
    static bool AABBContains(const FAABB& Outer, const FAABB& Inner);

private:
    TArray<FBVHItem> Items;
    TArray<FBVHNode> Nodes;
    int32 Root = -1;
    int32 LeafMax = 6;
    int32 MaxDepth = 64;
};
