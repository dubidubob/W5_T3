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
	void SetStaticMesh(UStaticMesh* InMesh);
	void SetUseUVScroll(bool bEnable);
	bool GetUseUVScroll () const { return bUseUVScroll; }

	// UV 스크롤 속도(U,V) 제어
	void SetUVScrollSpeed(const FVector2& InSpeed) { UVScrollSpeed = InSpeed; }
	FVector2 GetUVScrollSpeed() const { return UVScrollSpeed; }
	// 현재 프레임에서 셰이더에 넘길 UV 스크롤 시간(누적 + 진행)
	float GetUVScrollTimeForShader() const;
	virtual FAABB GetWorldBounds() const;
	UStaticMesh* GetStaticMesh() { return StaticMesh; }

	// Override
	virtual const void* GetRawVertexData() const;
	virtual uint32 GetVertexCount() const;
	virtual uint32 GetVertexStride() const;
	virtual uint32 GetVertexPositionOffset() const;
	virtual const TArray<uint32>* GetIndicesData() const;

private:
	FVector2 UVScrollOffset = FVector2(0.0f, 0.0f);
	UStaticMesh* StaticMesh;
	bool bUseUVScroll = false;
	float LastUVScrollUpdateTime = 0.0f;
	// 스크롤 정지 시 마지막 상태를 유지하기 위한 누적 시간
	float UVScrollAccumTime = 0.0f;
	// UV 스크롤 속도 (U,V)
	FVector2 UVScrollSpeed = FVector2(0.0f, -0.9f);
};

