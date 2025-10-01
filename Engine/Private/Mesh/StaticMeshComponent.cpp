#include "pch.h"
#include "Mesh/StaticMeshComponent.h"
#include "Mesh/StaticMesh/StaticMesh.h"
#include "Mesh/StaticMesh/ObjManager.h"
#include "Manager/Time/TimeManager.h"
#include "Global/PlatformTime.h"

IMPLEMENT_CLASS(UStaticMeshComponent, UMeshComponent)

void UStaticMeshComponent::SetStaticMeshByPath(path Path)
{
	StaticMesh = FObjManager::GetInstance().LoadObjStaticMesh(Path);
	bIsLocalBoundsDirty = true;
}

void UStaticMeshComponent::SetStaticMesh(UStaticMesh* InMesh)
{
	StaticMesh = InMesh;
	bIsLocalBoundsDirty = true;
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
        LastUVScrollUpdateTime = Now;
    }
    else
    {
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

FAABB UStaticMeshComponent::GetLocalBounds() const
{
	if (bIsLocalBoundsDirty && StaticMesh && StaticMesh->GetStaticMeshAsset())
	{
		const auto& Vertices = StaticMesh->GetStaticMeshAsset()->Vertices;
		if (Vertices.empty())
		{
			CachedLocalBounds = FAABB();
		}
		else
		{
			CachedLocalBounds.Reset();
			for (const FNormalVertex& Vertex : Vertices)
			{
				CachedLocalBounds.AddPoint(Vertex.Pos);
			}
		}
		bIsLocalBoundsDirty = false;
	}
	return CachedLocalBounds;
}

FAABB UStaticMeshComponent::GetWorldBounds() const
{
	if (bIsLocalBoundsDirty || GetTransformDirty())
	{
		if (!StaticMesh || !StaticMesh->GetStaticMeshAsset())
		{
			return FAABB();
		}

		const FAABB LocalBounds = GetLocalBounds();
		if (!LocalBounds.IsValid())
		{
			return FAABB();
		}
		CachedWorldBounds = LocalBounds.TransformBy(GetWorldTransformMatrix());
	}
	return CachedWorldBounds;
}

void UStaticMeshComponent::DuplicateSubObjects()
{
}

UStaticMeshComponent* UStaticMeshComponent::Duplicate()
{
	//UStaticMeshComponent* NewComp = static_cast<UStaticMeshComponent*>(UPrimitiveComponent::Duplicate());

	UStaticMeshComponent* NewComp = new UStaticMeshComponent(*this);

	NewComp->DuplicateSubObjects();

	return NewComp;
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
