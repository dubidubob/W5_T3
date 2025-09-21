#include "pch.h"
#include "Public/Mesh/StaticMesh/StaticMesh.h"
#include "Mesh/Material/Material.h"

IMPLEMENT_CLASS(UStaticMesh, UObject)

UStaticMesh::UStaticMesh() : StaticMeshAsset(nullptr)
{
}

UStaticMesh::~UStaticMesh()
{
}

const FString& UStaticMesh::GetAssetPathFileName() const
{
	if (StaticMeshAsset)
	{
		return StaticMeshAsset->FileName.ToString();
	}

	static const FString EmptyString = "";
	return EmptyString;
}
