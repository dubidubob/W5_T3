#pragma once
#include "Core/Object.h"
#include "Math/MeshBVH.h"
class UMaterial;
class FArchive;

struct FNormalVertex
{
	FVector Pos;
	FVector Normal;
	FVector4 Color;
	FVector2 Tex;

	void Serialize(class FArchive& Ar);
};

inline FArchive& operator<<(FArchive& Ar, FNormalVertex& Vertex);

struct FStaticMaterial
{
    FString Name;
    FVector AmbientColor;
    FVector DiffuseColor;
    FVector SpecularColor;
    float SpecularExponent = 0.0f;
    float Alpha = 1.0f;
    FString SpecularPath;
    FString DiffusePath;

	// -- Do not need to Serialize --
	//ID3D11ShaderResourceView* DiffuseSRV = nullptr;
	ID3D11ShaderResourceView* TextureSRV = nullptr;
	bool bUseTexture = false;
	// -- Do not need to Serialize --

	void Serialize(class FArchive& Ar);
};
inline FArchive& operator<<(FArchive& Ar, FStaticMaterial& StaticMaterial);

struct FStaticMeshSection
{
	uint32 FirstIndex;
	uint32 NumIndices;
	int32 MaterialIndex;

	void Serialize(class FArchive& Ar);
};
inline FArchive& operator<<(FArchive& Ar, FStaticMeshSection& StaticMeshSection);

/**
* @brief 렌더링에 필요한 모든 데이터를 담는 구조체
*/ 
struct FStaticMesh
{
	FName FileName;
	TArray<FNormalVertex> Vertices;
	TArray<uint32> Indices;

	TArray<FStaticMaterial> Materials;
	TArray<FStaticMeshSection> Sections;

	// -- Do not need to Serialize --
	ID3D11Buffer* VertexBuffer = nullptr;
	ID3D11Buffer* IndexBuffer = nullptr;

	uint32 VertexCount = 0;
	uint32 IndexCount = 0;
	uint32 ByteWidth = 0;
	// -- Do not need to Serialize --

private:
	TMap<FStaticMaterial*, TArray<FStaticMeshSection*>> SectionMap;
	FMeshBVH MeeshBVH;

public:
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

	FString GetFileName() const
	{
		return FileName.ToString();
	}

	FString GetBaseFileName() const
	{
		FString FullName = FileName.ToString();
		size_t DotPos = FullName.find_last_of('.');
		if (DotPos != FString::npos)
			return FullName.substr(0, DotPos);
		return FullName; 
	}

	void CreateBVH();
	void Serialize(class FArchive& Ar);

	const TArray<FStaticMeshSection*>& GetSectionMap(FStaticMaterial* Material) const
	{
		return *SectionMap.Find(Material);
	}
	void SetSectionMap()
	{
		for (FStaticMeshSection& Section : Sections)
		{
			FStaticMaterial* pMat = &Materials[Section.MaterialIndex];
			SectionMap[pMat].Push(&Section);
		}
	}
};

class UStaticMesh : public UObject
{
	DECLARE_CLASS(UStaticMesh, UObject)

public:
	UStaticMesh();
	virtual ~UStaticMesh();

	//Getter
	FStaticMesh* GetStaticMeshAsset() const { return StaticMeshAsset; }
	const FName& GetAssetPathFileName() const;

	//Setter
	void SetStaticMeshAsset(FStaticMesh* InStaticMeshAsset) { StaticMeshAsset = InStaticMeshAsset; }

private:
	FStaticMesh* StaticMeshAsset;
};

struct FRenderStreamKey
{
	FStaticMaterial* Material;
	uint32 Idx;
	FRenderStreamKey(FStaticMaterial* InMat, uint32 InIdx) : Material(InMat), Idx(InIdx){}
};



