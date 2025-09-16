#include "pch.h"
#include "Mesh/SceneComponent.h"
#include "Mesh/ResourceManager.h"

#include <algorithm>

IMPLEMENT_CLASS(USceneComponent, UActorComponent)

USceneComponent::USceneComponent()
{
	ComponentType = EComponentType::Scene;
}

void USceneComponent::SetParentAttachment(USceneComponent* NewParent)
{
	if (NewParent == this)
	{
		return;
	}

	if (NewParent == ParentAttachment)
	{
		return;
	}

	//부모의 조상중에 내 자식이 있으면 순환참조 -> 스택오버플로우 일어남.
	for (USceneComponent* Ancester = NewParent; Ancester; Ancester = NewParent->ParentAttachment)
	{
		if (NewParent == this) //조상중에 내 자식이 있다면 조상중에 내가 있을 것임.
			return;
	}

	//부모가 될 자격이 있음, 이제 부모를 바꿈.

	if (ParentAttachment) //부모 있었으면 이제 그 부모의 자식이 아님
	{
		ParentAttachment->RemoveChild(this);
	}

	ParentAttachment = NewParent;

	NewParent->Children.push_back(this);

	MarkAsDirty();

}

void USceneComponent::RemoveChild(USceneComponent* ChildDeleted)
{
	Children.erase(std::remove(Children.begin(), Children.end(), this), Children.end());
}

void USceneComponent::MarkAsDirty()
{
	bIsTransformDirty = true;
	bIsTransformDirtyInverse = true;

	for (USceneComponent* Child : Children)
	{
		Child->MarkAsDirty();
	}
}

IMPLEMENT_CLASS(UPrimitiveComponent, USceneComponent)
UPrimitiveComponent::UPrimitiveComponent()
{
	ComponentType = EComponentType::Primitive;
}

void USceneComponent::SetRelativeLocation(const FVector& Location)
{
	RelativeLocation = Location;
	MarkAsDirty();
}

void USceneComponent::SetRelativeRotation(const FVector& Rotation)
{
    RelativeRotation = Rotation;
    // Keep quaternion in sync with UI degrees
    RelativeRotationQuat = FQuat::FromEulerXYZ(RelativeRotation);
    MarkAsDirty();
}
void USceneComponent::SetRelativeScale3D(const FVector& Scale)
{
	FVector ActualScale = Scale;
	ActualScale.X = std::max(ActualScale.X, MinScale);
	ActualScale.Y = std::max(ActualScale.Y, MinScale);
	ActualScale.Z = std::max(ActualScale.Z, MinScale);
	RelativeScale3D = ActualScale;
	MarkAsDirty();
}

void USceneComponent::SetUniformScale(bool bIsUniform)
{
	bIsUniformScale = bIsUniform;
}

bool USceneComponent::IsUniformScale() const
{
	return bIsUniformScale;
}

const FVector& USceneComponent::GetRelativeLocation() const
{
	return RelativeLocation;
}
const FVector& USceneComponent::GetRelativeRotation() const
{
	return RelativeRotation;
}
const FVector& USceneComponent::GetRelativeScale3D() const
{
	return RelativeScale3D;
}

const FVector& USceneComponent::GetWorldLocation() const
{
	const FMatrix& WorldMatrix = GetWorldTransformMatrix();
	return FVector(WorldMatrix.Data[3][0], WorldMatrix.Data[3][1], WorldMatrix.Data[3][2]);
}

const FMatrix& USceneComponent::GetWorldTransformMatrix() const
{
    if (bIsTransformDirty)
    {
        // Quaternion-based TRS (row-major): I * S * R * T
        WorldTransformMatrix = FMatrix::GetModelMatrix(RelativeLocation, RelativeRotationQuat, RelativeScale3D);

        for (USceneComponent* Ancester = ParentAttachment; Ancester; Ancester = Ancester->ParentAttachment)
        {
            WorldTransformMatrix *= FMatrix::GetModelMatrix(Ancester->RelativeLocation, Ancester->RelativeRotationQuat, Ancester->RelativeScale3D);
        }

        bIsTransformDirty = false;
    }

	return WorldTransformMatrix;
}

const FMatrix& USceneComponent::GetWorldTransformMatrixInverse() const
{

    if (bIsTransformDirtyInverse)
    {
        WorldTransformMatrixInverse = FMatrix::Identity;
        for (USceneComponent* Ancestor = ParentAttachment; Ancestor; Ancestor = Ancestor->ParentAttachment)
        {
            WorldTransformMatrixInverse = FMatrix::GetModelMatrixInverse(Ancestor->RelativeLocation, Ancestor->RelativeRotationQuat, Ancestor->RelativeScale3D) * WorldTransformMatrixInverse;
        }
        WorldTransformMatrixInverse = WorldTransformMatrixInverse * FMatrix::GetModelMatrixInverse(RelativeLocation, RelativeRotationQuat, RelativeScale3D);

        bIsTransformDirtyInverse = false;
    }

	return WorldTransformMatrixInverse;
}

const TArray<FVertex>* UPrimitiveComponent::GetVerticesData() const
{
    UResourceManager& ResourceManager = UResourceManager::GetInstance();
    return ResourceManager.GetVertexData(Type);
}

const TArray<FVertex>* UPrimitiveComponent::GetReducedVerticesData() const
{
	UResourceManager& ResourceManager = UResourceManager::GetInstance();
	return ResourceManager.GetReducedVertexData(Type);
}

const TArray<uint32>* UPrimitiveComponent::GetIndicesData() const
{
	UResourceManager& ResourceManager = UResourceManager::GetInstance();
	return ResourceManager.GetIndexData(Type);
}

void UPrimitiveComponent::SetTopology(D3D11_PRIMITIVE_TOPOLOGY InTopology)
{
	Topology = InTopology;
}

D3D11_PRIMITIVE_TOPOLOGY UPrimitiveComponent::GetTopology() const
{
	return Topology;
}

FAABB UPrimitiveComponent::GetWorldBounds() const
{
	if (!Vertices || Vertices->empty())
	{
		return FAABB();
	}

	FAABB Bounds;
	const FMatrix& Transform = GetWorldTransformMatrix();

	for (const FVertex& Vertex : *Vertices)
	{
		FVector4 TransformedPoint = FVector4(Vertex.Position.X, Vertex.Position.Y, Vertex.Position.Z, 1.0f) * Transform;
		Bounds.AddPoint(FVector(TransformedPoint.X, TransformedPoint.Y, TransformedPoint.Z));
	}
	return Bounds;
}

//FAABB UPrimitiveComponent::GetWorldBounds() const
//{
//	return GetLocalBounds();
//}

//void UPrimitiveComponent::Render(const URenderer& Renderer) const
//{
//	Renderer.RenderPrimitive(Vertexbuffer, NumVertices);
//}


/*
* 리소스는 Manager가 관리하고 component는 참조만 함.
*
*/

IMPLEMENT_CLASS(USphereComponent, UPrimitiveComponent)
USphereComponent::USphereComponent()
{
    UResourceManager& ResourceManager = UResourceManager::GetInstance();
    Type = EPrimitiveType::Sphere;

    Vertices = ResourceManager.GetVertexData(Type);

    VertexBuffer = ResourceManager.GetVertexBuffer(Type);
	ReducedVertexBuffer = ResourceManager.GetReducedVertexBuffer(Type);
	IndexBuffer = ResourceManager.GetIndexBuffer(Type);

    VertexNum = ResourceManager.GetVertexNum(Type);
	ReducedVertexNum = ResourceManager.GetReducedVertexNum(Type);
	IndexNum = ResourceManager.GetIndexNum(Type);

	RenderState.CullMode = ECullMode::Back;
	RenderState.FillMode = EFillMode::Solid;
}

FAABB USphereComponent::GetWorldBounds() const
{
	// 실제 버텍스 데이터에서 바운딩 박스 계산
	return UPrimitiveComponent::GetWorldBounds();
}

IMPLEMENT_CLASS(UCubeComponent, UPrimitiveComponent)
UCubeComponent::UCubeComponent()
{
	UResourceManager& ResourceManager = UResourceManager::GetInstance();
	Type = EPrimitiveType::Cube;
	Vertices = ResourceManager.GetVertexData(Type);

	VertexBuffer = ResourceManager.GetVertexBuffer(Type);
	ReducedVertexBuffer = ResourceManager.GetReducedVertexBuffer(Type);
	IndexBuffer = ResourceManager.GetIndexBuffer(Type);

	VertexNum = ResourceManager.GetVertexNum(Type);
	ReducedVertexNum = ResourceManager.GetReducedVertexNum(Type);
	IndexNum = ResourceManager.GetIndexNum(Type);

	RenderState.CullMode = ECullMode::Back;
	RenderState.FillMode = EFillMode::Solid;
}

FAABB UCubeComponent::GetWorldBounds() const
{
	// 실제 버텍스 데이터에서 바운딩 박스 계산
	return UPrimitiveComponent::GetWorldBounds();
}

IMPLEMENT_CLASS(ULineComponent, UPrimitiveComponent)
ULineComponent::ULineComponent()
{
	UResourceManager& ResourceManager = UResourceManager::GetInstance();
	Type = EPrimitiveType::Line;
	Vertices = ResourceManager.GetVertexData(Type);

	VertexBuffer = ResourceManager.GetVertexBuffer(Type);
	ReducedVertexBuffer = ResourceManager.GetReducedVertexBuffer(Type);
	IndexBuffer = ResourceManager.GetIndexBuffer(Type);

	VertexNum = ResourceManager.GetVertexNum(Type);
	ReducedVertexNum = ResourceManager.GetReducedVertexNum(Type);
	IndexNum = ResourceManager.GetIndexNum(Type);

	Topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
	RenderState.CullMode = ECullMode::None;
	RenderState.FillMode = EFillMode::WireFrame;
}

FAABB ULineComponent::GetWorldBounds() const
{
	return UPrimitiveComponent::GetWorldBounds();
}

IMPLEMENT_CLASS(UTriangleComponent, UPrimitiveComponent)
UTriangleComponent::UTriangleComponent()
{
	UResourceManager& ResourceManager = UResourceManager::GetInstance();
	Type = EPrimitiveType::Triangle;
	Vertices = ResourceManager.GetVertexData(Type);

	VertexBuffer = ResourceManager.GetVertexBuffer(Type);
	ReducedVertexBuffer = ResourceManager.GetReducedVertexBuffer(Type);
	IndexBuffer = ResourceManager.GetIndexBuffer(Type);

	VertexNum = ResourceManager.GetVertexNum(Type);
	ReducedVertexNum = ResourceManager.GetReducedVertexNum(Type);
	IndexNum = ResourceManager.GetIndexNum(Type);

	RenderState.CullMode = ECullMode::None;
	RenderState.FillMode = EFillMode::Solid;
}

FAABB UTriangleComponent::GetWorldBounds() const
{
	return UPrimitiveComponent::GetWorldBounds();
}

IMPLEMENT_CLASS(USquareComponent, UPrimitiveComponent)
USquareComponent::USquareComponent()
{
	UResourceManager& ResourceManager = UResourceManager::GetInstance();
	Type = EPrimitiveType::Square;
	Vertices = ResourceManager.GetVertexData(Type);

	VertexBuffer = ResourceManager.GetVertexBuffer(Type);
	ReducedVertexBuffer = ResourceManager.GetReducedVertexBuffer(Type);
	IndexBuffer = ResourceManager.GetIndexBuffer(Type);

	VertexNum = ResourceManager.GetVertexNum(Type);
	ReducedVertexNum = ResourceManager.GetReducedVertexNum(Type);
	IndexNum = ResourceManager.GetIndexNum(Type);

	RenderState.CullMode = ECullMode::None;
	RenderState.FillMode = EFillMode::Solid;
}

FAABB USquareComponent::GetWorldBounds() const
{
	return UPrimitiveComponent::GetWorldBounds();
}
