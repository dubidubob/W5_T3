#pragma once
#include "Core/Object.h"
class UMaterial;

struct FNormalVertex
{
	FVector Pos;
	FVector Normal;
	FVector4 Color;
	FVector2 Tex;
};

struct FStaticMaterial
{
	FString Name;
	FVector AmbientColor;
	FVector DiffuseColor;
	FVector SpecularColor;
	float SpecularExponent;
	float Alpha;
	FString SpecularPath;
	FString DiffusePath;
	//ID3D11ShaderResourceView* DiffuseSRV = nullptr;
	ID3D11ShaderResourceView* TextureSRV = nullptr;

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
	~FStaticMesh()
	{
		if (VertexBuffer)
		{
			VertexBuffer->Release();
			VertexBuffer = nullptr;
		}
		if (IndexBuffer)
		{
			IndexBuffer->Release();
			IndexBuffer = nullptr;
		}
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

	// 이거 두개 맵핑 되어야한다 .
	TArray<FStaticMeshSection> Sections;  // 메시 파츠
	TArray<FStaticMaterial> Materials;
	//TArray<UMaterial*> MaterialSlots;     // 섹션별로 참조하는 머티리얼
};



