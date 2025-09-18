#include "pch.h"
#include "Mesh/StaticMesh/ObjManager.h"
#include "Core/ObjectIterator.h"
#include "Public/Mesh/StaticMesh/ObjImporter.h"
#include "Public/Mesh/StaticMesh/StaticMesh.h"

IMPLEMENT_SINGLETON(FObjManager)

FObjManager::FObjManager() = default;
FObjManager::~FObjManager() = default;

FStaticMesh* FObjManager::LoadObjStaticMeshAsset(const FString& PathFileName)
{
	auto It = ObjStaticMeshMap.find(PathFileName);
	if (It != ObjStaticMeshMap.end())
	{
		// Already Cached
		return It->second;
	}

	FStaticMesh* NewStaticMesh = FObjImporter::ParseAndConvert(PathFileName);
	if (NewStaticMesh)
	{
		ObjStaticMeshMap[PathFileName] = NewStaticMesh;
	}

	return NewStaticMesh;
}

UStaticMesh* FObjManager::LoadObjStaticMesh(const FString& PathFileName)
{
	for (TObjectIterator<UStaticMesh> It; It; ++It)
	{
		UStaticMesh* StaticMesh = *It;
		if (StaticMesh && StaticMesh->GetAssetPathFileName() == PathFileName)
		{
			return StaticMesh;
		}
	}

	FStaticMesh* StaticMeshAsset = LoadObjStaticMeshAsset(PathFileName);
	if (!StaticMeshAsset)
	{
		return nullptr;
	}

	// Generate UStaticMesh
	UStaticMesh* NewStaticMesh = NewObject<UStaticMesh>();
	if (NewStaticMesh)
	{
		NewStaticMesh->SetStaticMeshAsset(StaticMeshAsset);
	}

	return NewStaticMesh;
}
