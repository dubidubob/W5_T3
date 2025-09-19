#pragma once
#include "MeshComponent.h"
#include "Math/AABB.h"
class UStaticMesh;
struct FAABB;

class UStaticMeshComponent : public UMeshComponent
{
	DECLARE_CLASS(UStaticMeshComponent, UMeshComponent)

public:
	// Getter
	UStaticMesh* GetStaticMesh() const { return StaticMesh; }

	// Setter (직접 UStaticMesh 포인터를 넘겨줌)
	void SetStaticMesh(const FString& InMeshFName);
	virtual FAABB GetWorldBounds() const;
	UStaticMesh* GetStaticMesh() { return StaticMesh; }
private:
	UStaticMesh* StaticMesh;
};

