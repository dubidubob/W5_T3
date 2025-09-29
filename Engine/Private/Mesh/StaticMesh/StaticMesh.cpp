#include "pch.h"
#include "Public/Mesh/StaticMesh/StaticMesh.h"
#include "Mesh/Material/Material.h"
#include "Archive/Archive.h"

IMPLEMENT_CLASS(UStaticMesh, UObject)

UStaticMesh::UStaticMesh() : StaticMeshAsset(nullptr)
{
}

UStaticMesh::~UStaticMesh()
{
}

const FName& UStaticMesh::GetAssetPathFileName() const
{
	if (StaticMeshAsset)
	{
		return StaticMeshAsset->FileName;
	}

	static const FString EmptyString = "";
	return EmptyString;
}

void FStaticMesh::CreateBVH()
{
	// MeeshBVH.Build();
}

void FStaticMesh::Serialize(FArchive& Ar)
{
	FString NameString = FileName.ToString();
	Ar << NameString;
	if (Ar.IsLoading())
	{
		FileName = FName(NameString);
	}
	Ar << Vertices;
	Ar << Indices;
	Ar << Materials;
	Ar << Sections;
}

void FNormalVertex::Serialize(FArchive& Ar)
{
	Ar << Pos; Ar << Normal; Ar << Color; Ar << Tex;
}

inline FArchive& operator<<(FArchive& Ar, FNormalVertex& Vertex)
{
	Vertex.Serialize(Ar); return Ar;
}

inline FArchive& operator<<(FArchive& Ar, FStaticMaterial& StaticMaterial)
{
	StaticMaterial.Serialize(Ar); return Ar;
}

inline FArchive& operator<<(FArchive& Ar, FStaticMeshSection& StaticMeshSection)
{
	StaticMeshSection.Serialize(Ar); return Ar;
}

void FStaticMaterial::Serialize(FArchive& Ar)
{
	Ar << Name;
	Ar << AmbientColor; Ar << DiffuseColor; Ar << SpecularColor;
	Ar << SpecularExponent; Ar << Alpha;
	Ar << SpecularPath; Ar << DiffusePath;
}

void FStaticMeshSection::Serialize(FArchive& Ar)
{
	Ar << FirstIndex; Ar << NumIndices; Ar << MaterialIndex;
}
