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
	int OriginIdx;
};

struct FBVHNode
{
    FAABB Bounds;
    int32 Left = -1;
    int32 Right = -1;
    int32 First = 0; // First Index
    int32 Count = 0;
    bool bLeaf = false;
};

class FBVH
{
public:
    void Build(const TArray<UStaticMeshComponent*>& Comps);
    void QueryAABB(const FAABB& Q, TArray<UStaticMeshComponent*>& Out) const;
	void QueryFrustum(const TStaticArray<FVector4, 6>& Planes, TArray<bool>& OutVisibles) const;
	void QueryRayCandidates(const FRay& Ray, int MaxK, TArray<UStaticMeshComponent*>& Out);

protected:
	virtual void Clear();
	virtual int32 BuildRange(int32 First, int32 Last, int32 Depth);
	static int32 ChooseAxis(const FAABB& Bounds);
	static bool IntersectAABB(const FRay& Ray, const FAABB& b, float tMax, float& t0, float& t1);
	static bool RayBoxEntry(const FAABB& Box, const FRay& Ray, float& OutTNear);

	TArray<FBVHItem> Items;
	TArray<FBVHNode> Nodes;
	int32 Root = -1;
	int32 LeafMax = 6; // Max Item Number in One Leaf Node
	int32 MaxDepth = 64; // Max Tree Height

private:
    void AddSubtree(int32 NodeIdx, TArray<UStaticMeshComponent*>& Out) const;
    void AddSubtreeAll(int32 NodeIdx, TArray<bool>& OutVisible) const;
    static bool AABBOutsideFrustum(const FAABB& B, const TStaticArray<FVector4,6>& Planes);
    static bool AABBInsideFrustum(const FAABB& B, const TStaticArray<FVector4,6>& Planes);
    static bool AABBContains(const FAABB& Outer, const FAABB& Inner);
};
