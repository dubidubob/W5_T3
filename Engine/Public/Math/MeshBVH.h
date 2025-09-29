#pragma once
#include "Global/Types.h"
#include "Math//BVH.h"

class FStaticMesh;
class FAABB;
struct FMeshBVHItem
{
	float VertexPosX[3];
	float VertexPosY[3];
	float VertexPosZ[3];

	FAABB Bounds;
	FVector Centroid;
	int OriginIdx;
};

class FMeshBVH : public FBVH
{
public:
	virtual ~FMeshBVH() = default;

	void Build(const FStaticMesh& Mesh);
	void QueryRayLocalMesh(const FRay& ModelRay, int MaxK, TArray<FMeshBVHItem*>& Out); // return Triangle, always 4*n

protected:
	virtual void Clear() override;
	virtual int32 BuildRange(int32 First, int32 Last, int32 Depth) override;

private:
	TArray<FMeshBVHItem> Items; // 정보 Data Carrier
};

