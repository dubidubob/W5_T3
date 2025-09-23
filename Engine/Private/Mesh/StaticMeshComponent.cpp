#include "pch.h"
#include "Mesh/StaticMeshComponent.h"
#include "Mesh/StaticMesh/StaticMesh.h"
#include "Mesh/StaticMesh/ObjManager.h"
#include "Manager/Time/TimeManager.h"

IMPLEMENT_CLASS(UStaticMeshComponent, UMeshComponent)

void UStaticMeshComponent::SetStaticMesh(const FString& InMeshFName)
{
	StaticMesh = FObjManager::GetInstance().LoadObjStaticMesh(InMeshFName);
}

void UStaticMeshComponent::SetUseUVScroll(bool bEnable)
{
    if (bUseUVScroll == bEnable)
    {
        return;
    }

    const float Now = UTimeManager::GetInstance().GetGameTime();

    if (bEnable)
    {
        // 스크롤을 다시 시작: 이 시점부터의 경과 시간을 기준으로 합산
        LastUVScrollUpdateTime = Now;
    }
    else
    {
        // 스크롤을 멈출 때, 지금까지의 경과 시간을 누적값에 더해 고정
        UVScrollAccumTime += (Now - LastUVScrollUpdateTime);
    }

    bUseUVScroll = bEnable;
}

float UStaticMeshComponent::GetUVScrollTimeForShader() const
{
    const float Now = UTimeManager::GetInstance().GetGameTime();
    if (bUseUVScroll)
    {
        return UVScrollAccumTime + (Now - LastUVScrollUpdateTime);
    }
    return UVScrollAccumTime;
}

FAABB UStaticMeshComponent::GetWorldBounds() const
{
	if (GetStaticMesh()->GetStaticMeshAsset()->Vertices.empty())
	{
		return FAABB();
	}

	FAABB Bounds;
	const FMatrix& Transform = GetWorldTransformMatrix();

	for (const FNormalVertex& Vertex : GetStaticMesh()->GetStaticMeshAsset()->Vertices)
	{
		FVector4 TransformedPoint = FVector4(Vertex.Pos.X, Vertex.Pos.Y, Vertex.Pos.Z, 1.0f) * Transform;
		Bounds.AddPoint(FVector(TransformedPoint.X, TransformedPoint.Y, TransformedPoint.Z));
	}
	return Bounds;
}

const void* UStaticMeshComponent::GetRawVertexData() const
{
	if (UStaticMesh* StaticMesh = GetStaticMesh())
	{
		if (FStaticMesh* StaticMeshAsset = StaticMesh->GetStaticMeshAsset())
		{
			return StaticMeshAsset->Vertices.data();
		}
	}
	return nullptr;
}

uint32 UStaticMeshComponent::GetVertexCount() const
{
	if (UStaticMesh* StaticMesh = GetStaticMesh())
	{
		if (FStaticMesh* StaticMeshAsset = StaticMesh->GetStaticMeshAsset())
		{
			return StaticMeshAsset->Vertices.Num();
		}
	}
	return 0;
}

uint32 UStaticMeshComponent::GetVertexStride() const
{
	return sizeof(FNormalVertex);
}

uint32 UStaticMeshComponent::GetVertexPositionOffset() const
{
	return offsetof(FNormalVertex, Pos);
}

const TArray<uint32>* UStaticMeshComponent::GetIndicesData() const
{
	if (UStaticMesh* StaticMesh = GetStaticMesh())
	{
		if (FStaticMesh* StaticMeshAsset = StaticMesh->GetStaticMeshAsset())
		{
			return &StaticMeshAsset->Indices;
		}
	}
	return nullptr;
}
