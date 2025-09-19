#pragma once
#include "Core/Object.h"

struct FNormalVertex
{
	FVector Pos;
	FVector Normal;
	FVector4 Color;
	FVector2 Tex;
};

struct FStaticMaterial
{
	FVector AmbientColor;
	FVector DiffuseColor;
	FVector SpecularColor;
	float SpecularExponent;
	float Alpha;

	ID3D11ShaderResourceView* DiffuseSRV = nullptr;
	ID3D11ShaderResourceView* NormalSRV = nullptr;
};

struct FStaticMeshSection
{
	uint32 FirstIndex;
	uint32 NumIndices;
	int32 MaterialIndex;
};

/**
* @brief 렌더링에 필요한 모든 데이터를 담는 구조체
*/ 
struct FStaticMesh
{
	FName FileName;
	TArray<FNormalVertex> Vertices;
	TArray<uint32> Indices;

	ID3D11Buffer* VertexBuffer = nullptr; 
	ID3D11Buffer* IndexBuffer = nullptr;

	uint32 VertexCount = 0;
	uint32 IndexCount = 0;
	uint32 ByteWidth = 0;
	TArray<FStaticMaterial> Materials;
	TArray<FStaticMeshSection> Sections;

	FStaticMesh() = default;
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



