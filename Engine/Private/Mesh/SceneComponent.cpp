#include "pch.h"
#include "Mesh/SceneComponent.h"
#include "Mesh/ResourceManager.h"

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
	MarkAsDirty();
}
void USceneComponent::SetRelativeScale3D(const FVector& Scale)
{
	FVector ActualScale = Scale;
	if (ActualScale.X < MinScale)
		ActualScale.X = MinScale;
	if (ActualScale.Y < MinScale)
		ActualScale.Y = MinScale;
	if (ActualScale.Z < MinScale)
		ActualScale.Z = MinScale;
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

const FMatrix& USceneComponent::GetWorldTransformMatrix() const
{
	if (bIsTransformDirty)
	{
		WorldTransformMatrix = FMatrix::GetModelMatrix(RelativeLocation, FVector::GetDegreeToRadian(RelativeRotation), RelativeScale3D);


		for (USceneComponent* Ancester = ParentAttachment; Ancester; Ancester = Ancester->ParentAttachment)
		{
			WorldTransformMatrix *= FMatrix::GetModelMatrix(Ancester->RelativeLocation, FVector::GetDegreeToRadian(Ancester->RelativeRotation), Ancester->RelativeScale3D);
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
		for (USceneComponent* Ancestor = ParentAttachment; Ancestor && Ancestor->ParentAttachment; Ancestor = Ancestor->ParentAttachment)
		{
			WorldTransformMatrixInverse = FMatrix::GetModelMatrixInverse(Ancestor->RelativeLocation, FVector::GetDegreeToRadian(Ancestor->RelativeRotation), Ancestor->RelativeScale3D) * WorldTransformMatrixInverse;

		}
		WorldTransformMatrixInverse = WorldTransformMatrixInverse * FMatrix::GetModelMatrixInverse(RelativeLocation, FVector::GetDegreeToRadian(RelativeRotation), RelativeScale3D);

		bIsTransformDirtyInverse = false;
	}

	return WorldTransformMatrixInverse;
}

const TArray<FVertex>* UPrimitiveComponent::GetVerticesData() const
{
    UResourceManager& ResourceManager = UResourceManager::GetInstance();
    return ResourceManager.GetVertexData(Type);
}

void UPrimitiveComponent::SetTopology(D3D11_PRIMITIVE_TOPOLOGY InTopology)
{
	Topology = InTopology;
}

D3D11_PRIMITIVE_TOPOLOGY UPrimitiveComponent::GetTopology() const
{
	return Topology;
}

FAABB UPrimitiveComponent::GetLocalBounds() const
{
	if (!Vertices || Vertices->empty())
	{
		return FAABB();
	}

	FAABB Bounds;
	for (const FVertex& Vertex : *Vertices)
	{
		Bounds.AddPoint(Vertex.Position);
	}
	return Bounds;
}

FAABB UPrimitiveComponent::GetWorldBounds() const
{
	FAABB LocalBounds = GetLocalBounds();
	if (!LocalBounds.IsValid())
	{
		return FAABB();
	}

	return LocalBounds.TransformBy(GetWorldTransformMatrix());
}

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
    Vertexbuffer = ResourceManager.GetVertexbuffer(Type);
    NumVertices = ResourceManager.GetNumVertices(Type);
	RenderState.CullMode = ECullMode::Back;
	RenderState.FillMode = EFillMode::Solid;
}

FAABB USphereComponent::GetLocalBounds() const
{
	// 실제 버텍스 데이터에서 바운딩 박스 계산
	return UPrimitiveComponent::GetLocalBounds();
}

IMPLEMENT_CLASS(UCubeComponent, UPrimitiveComponent)
UCubeComponent::UCubeComponent()
{
	UResourceManager& ResourceManager = UResourceManager::GetInstance();
	Type = EPrimitiveType::Cube;
	Vertices = ResourceManager.GetVertexData(Type);
	Vertexbuffer = ResourceManager.GetVertexbuffer(Type);
	NumVertices = ResourceManager.GetNumVertices(Type);
	RenderState.CullMode = ECullMode::Back;
	RenderState.FillMode = EFillMode::Solid;
}

FAABB UCubeComponent::GetLocalBounds() const
{
	// 실제 버텍스 데이터에서 바운딩 박스 계산
	return UPrimitiveComponent::GetLocalBounds();
}

IMPLEMENT_CLASS(ULineComponent, UPrimitiveComponent)
ULineComponent::ULineComponent()
{
	UResourceManager& ResourceManager = UResourceManager::GetInstance();
	Type = EPrimitiveType::Line;
	Vertices = ResourceManager.GetVertexData(Type);
	Vertexbuffer = ResourceManager.GetVertexbuffer(Type);
	NumVertices = ResourceManager.GetNumVertices(Type);
	Topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
	RenderState.CullMode = ECullMode::None;
	RenderState.FillMode = EFillMode::WireFrame;
}

FAABB ULineComponent::GetLocalBounds() const
{
	return UPrimitiveComponent::GetLocalBounds();
}

IMPLEMENT_CLASS(UTriangleComponent, UPrimitiveComponent)
UTriangleComponent::UTriangleComponent()
{
	UResourceManager& ResourceManager = UResourceManager::GetInstance();
	Type = EPrimitiveType::Triangle;
	Vertices = ResourceManager.GetVertexData(Type);
	Vertexbuffer = ResourceManager.GetVertexbuffer(Type);
	NumVertices = ResourceManager.GetNumVertices(Type);
	RenderState.CullMode = ECullMode::None;
	RenderState.FillMode = EFillMode::Solid;
}

FAABB UTriangleComponent::GetLocalBounds() const
{
	return UPrimitiveComponent::GetLocalBounds();
}

IMPLEMENT_CLASS(USquareComponent, UPrimitiveComponent)
USquareComponent::USquareComponent()
{
	UResourceManager& ResourceManager = UResourceManager::GetInstance();
	Type = EPrimitiveType::Square;
	Vertices = ResourceManager.GetVertexData(Type);
	Vertexbuffer = ResourceManager.GetVertexbuffer(Type);
	NumVertices = ResourceManager.GetNumVertices(Type);
	RenderState.CullMode = ECullMode::None;
	RenderState.FillMode = EFillMode::Solid;
}

FAABB USquareComponent::GetLocalBounds() const
{
	return UPrimitiveComponent::GetLocalBounds();
}
