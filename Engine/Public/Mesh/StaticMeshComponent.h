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
	void SetUseUVScroll(bool bEnable);
	bool GetUseUVScroll () const { return bUseUVScroll; }
	virtual FAABB GetWorldBounds() const;
	UStaticMesh* GetStaticMesh() { return StaticMesh; }

	// Override
	virtual const void* GetRawVertexData() const;
	virtual uint32 GetVertexCount() const;
	virtual uint32 GetVertexStride() const;
	virtual uint32 GetVertexPositionOffset() const;
	virtual const TArray<uint32>* GetIndicesData() const;
private:
	UStaticMesh* StaticMesh;
	bool bUseUVScroll = false;

};

