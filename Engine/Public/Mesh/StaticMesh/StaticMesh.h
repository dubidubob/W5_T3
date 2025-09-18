#pragma once
#include "Core/Object.h"

struct FNormalVertex
{
	FVector Pos;
	FVector Normal;
	FVector4 Color;
	FVector2 Tex;
};

// 렌더링에 필요한 모든 데이터를 담는 구조체
struct FStaticMesh
{
	FName FileName;
	TArray<FNormalVertex> Vertices;
	TArray<uint32> Indices;

	TArray<int32> MaterialIndices;

	FStaticMesh() = default;
	~FStaticMesh()
	{
		Vertices.Empty();
		Indices.Empty();
	}
};

class UStaticMesh : public UObject
{
	DECLARE_CLASS(UStaticMesh, UObject)

public:
	UStaticMesh();
	virtual ~UStaticMesh();

	FStaticMesh* GetStaticMeshAsset() const { return StaticMeshAsset; }
	void SetStaticMeshAsset(FStaticMesh* InStaticMeshAsset) { StaticMeshAsset = InStaticMeshAsset; }

	const FString& GetAssetPathFileName() const;
private:
	FStaticMesh* StaticMeshAsset;
};
