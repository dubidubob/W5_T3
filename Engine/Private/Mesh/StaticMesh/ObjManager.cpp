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
		UE_LOG("FStaticMesh asset '%s' found in cache.", PathFileName.c_str());
		return It->second;
	}

	FStaticMesh* NewStaticMesh = FObjImporter::ParseAndConvert(PathFileName);
	if (NewStaticMesh)
	{
		ObjStaticMeshMap[PathFileName] = NewStaticMesh;
		UE_LOG("Successfully loaded FStaticMesh asset: %s", PathFileName.c_str());
		UE_LOG(" - Vertices: %d, Indices: %d", NewStaticMesh->Vertices.Num(), NewStaticMesh->Indices.Num() / 3);
		UE_LOG(" - Materials: %d, Sections: %d", NewStaticMesh->Materials.Num(), NewStaticMesh->Sections.Num());
	}
	else
	{
		UE_LOG("Failed to load FStaticMesh asset: %s", PathFileName.c_str());
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
			UE_LOG("UStaticMesh object '%s' found in cache.", PathFileName.c_str());
			return StaticMesh;
		}
	}

	FStaticMesh* StaticMeshAsset = LoadObjStaticMeshAsset(PathFileName);
	if (!StaticMeshAsset)
	{
		UE_LOG("Failed to generate UStaticMesh for '%s' due to failed asset loading.", PathFileName);
		return nullptr;
	}

	// Generate UStaticMesh
	UStaticMesh* NewStaticMesh = NewObject<UStaticMesh>();
	if (NewStaticMesh)
	{
		NewStaticMesh->SetStaticMeshAsset(StaticMeshAsset);
		UE_LOG("Successfully generated UStaticMesh for '%s'.", PathFileName.c_str());
	}
	else
	{
		UE_LOG("Failed to create UStaticMesh object for '%s'.", PathFileName.c_str());
	}

	return NewStaticMesh;
}
