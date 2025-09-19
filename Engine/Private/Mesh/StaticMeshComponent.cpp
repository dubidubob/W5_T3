#include "pch.h"
#include "Mesh/StaticMeshComponent.h"
#include "Mesh/StaticMesh/StaticMesh.h"
#include "Mesh/StaticMesh/ObjManager.h"

IMPLEMENT_CLASS(UStaticMeshComponent, UMeshComponent)

void UStaticMeshComponent::SetStaticMesh(const FString& InMeshFName)
{
	StaticMesh = FObjManager::GetInstance().LoadObjStaticMesh(InMeshFName);
}
