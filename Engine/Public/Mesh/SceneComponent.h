#pragma once
#include "Mesh/ActorComponent.h"
#include "ResourceManager.h"
#include "Math/AABB.h"
#include "Global/Quat.h"

class USceneComponent : public UActorComponent
{
	DECLARE_CLASS(USceneComponent, UActorComponent)

public:
	USceneComponent();

	void SetParentAttachment(USceneComponent* SceneComponent);
	void RemoveChild(USceneComponent* ChildDeleted);

	void MarkAsDirty();

	void SetRelativeLocation(const FVector& Location);
    void SetRelativeRotation(const FVector& Rotation);
    void SetRelativeRotation(const FQuat& Rotation) { RelativeRotationQuat = Rotation; RelativeRotation = FQuat::ToEulerXYZ(Rotation); MarkAsDirty(); }
	void SetRelativeScale3D(const FVector& Scale);
	void SetUniformScale(bool bIsUniform);

	bool IsUniformScale() const;

	const FVector& GetRelativeLocation() const;
	const FVector& GetRelativeRotation() const;
	const FQuat&   GetRelativeRotationQuat() const { return RelativeRotationQuat; }
	const FVector& GetRelativeScale3D() const;

	const FVector& GetWorldLocation() const;

	const FMatrix& GetWorldTransformMatrix() const;
	const FMatrix& GetWorldTransformMatrixInverse() const;

private:
	mutable bool bIsTransformDirty = true;
	mutable bool bIsTransformDirtyInverse = true;
	mutable FMatrix WorldTransformMatrix;
	mutable FMatrix WorldTransformMatrixInverse;

	USceneComponent* ParentAttachment = nullptr;
	TArray<USceneComponent*> Children;
	FVector RelativeLocation = FVector{ 0,0,0.f };
	FVector RelativeRotation = FVector{ 0,0,0.f };
	FQuat   RelativeRotationQuat = FQuat::Identity;
	FVector RelativeScale3D = FVector{ 0.3f,0.3f,0.3f };
	bool bIsUniformScale = false;
	const float MinScale = 0.01f;
};

class UPrimitiveComponent : public USceneComponent
{
	DECLARE_CLASS(UPrimitiveComponent, USceneComponent)

public:
	UPrimitiveComponent();

	const TArray<FVertex>* GetVerticesData() const;
	const TArray<FVertex>* GetReducedVerticesData() const;
	const TArray<uint32>* GetIndicesData() const;

	ID3D11Buffer* GetVertexBuffer() const { return VertexBuffer; }
	ID3D11Buffer* GetReducedVertexBuffer() const { return ReducedVertexBuffer; }
	ID3D11Buffer* GetIndexBuffer() const { return IndexBuffer; }

	uint32 GetVertexNum() const { return VertexNum; }
	uint32 GetReducedVertexNum() const { return ReducedVertexNum; }
	uint32 GetIndexNum() const { return IndexNum; }

	const FRenderState& GetRenderState() const { return RenderState; }

	void SetTopology(D3D11_PRIMITIVE_TOPOLOGY InTopology);
	D3D11_PRIMITIVE_TOPOLOGY GetTopology() const;
	//void Render(const URenderer& Renderer) const override;

	bool IsVisible() const { return bVisible; }
	void SetVisibility(bool bVisibility) { bVisible = bVisibility; }

	FVector4 GetColor() const { return Color; }
	void SetColor(const FVector4& InColor) { Color = InColor; }

	virtual FAABB GetWorldBounds() const;
	//FAABB GetWorldBounds() const;

protected:
	const TArray<FVertex>* Vertices = nullptr;
	FVector4 Color = FVector4{ 0.f,0.f,0.f,0.f };
	ID3D11Buffer* VertexBuffer = nullptr;
	ID3D11Buffer* ReducedVertexBuffer = nullptr;
	ID3D11Buffer* IndexBuffer = nullptr;
	uint32 VertexNum = 0;
	uint32 ReducedVertexNum = 0;
	uint32 IndexNum = 0;
	D3D11_PRIMITIVE_TOPOLOGY Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	FRenderState RenderState = {};
	EPrimitiveType Type = EPrimitiveType::Cube;

	bool bVisible = true;

};

class UTriangleComponent : public UPrimitiveComponent
{
	DECLARE_CLASS(UTriangleComponent, UPrimitiveComponent)
public:
	UTriangleComponent();
	virtual FAABB GetWorldBounds() const override;
};

class USquareComponent : public UPrimitiveComponent
{
	DECLARE_CLASS(USquareComponent, UPrimitiveComponent)
public:
	USquareComponent();
	virtual FAABB GetWorldBounds() const override;
};

class UCubeComponent : public UPrimitiveComponent
{
	DECLARE_CLASS(UCubeComponent, UPrimitiveComponent)
public:
	UCubeComponent();
	virtual FAABB GetWorldBounds() const override;
};

class USphereComponent : public UPrimitiveComponent
{
	DECLARE_CLASS(USphereComponent, UPrimitiveComponent)
public:
	USphereComponent();
	virtual FAABB GetWorldBounds() const override;
};

class ULineComponent : public UPrimitiveComponent
{
	DECLARE_CLASS(ULineComponent, UPrimitiveComponent)
public:
	ULineComponent();
	virtual FAABB GetWorldBounds() const override;
};
