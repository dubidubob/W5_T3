#pragma once
#include "MeshComponent.h"

class UStaticMesh;
class UStaticMeshComponent : public UMeshComponent
{
	DECLARE_CLASS(UStaticMeshComponent, UMeshComponent)

public:
	// Getter
	UStaticMesh* GetStaticMesh() const { return StaticMesh; }

	// Setter (직접 UStaticMesh 포인터를 넘겨줌)
	void SetStaticMesh(const FString& InMeshFName);
	UStaticMesh* GetStaticMesh() { return StaticMesh; }
private:
	UStaticMesh* StaticMesh;
};

