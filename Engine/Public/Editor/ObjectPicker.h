#pragma once
#include "Editor/Gizmo.h"

class UPrimitiveComponent;
class AActor;
class ULevel;
class UCamera;
class UGizmo;
class FOctree;
class FBVH;
struct FRay;

class UObjectPicker : public UObject
{
	DECLARE_CLASS(UObjectPicker, UObject)

public:
	UObjectPicker();
	void SetCamera(UCamera* Camera);

	/** Original picking methods */
	UPrimitiveComponent* PickPrimitive( const FRay& WorldRay, TArray<UPrimitiveComponent*> Candidate, float* Distance);

	/** Octree-based picking methods */
	UPrimitiveComponent* PickPrimitiveWithOctree(const FRay& WorldRay, FOctree* Octree, float* Distance);

	/** BVH-based picking methods */
	UPrimitiveComponent* PickPrimitiveWithBVH(const FRay& WorldRay, FBVH* BVH, float* Distance);

	/** Gizmo and utility methods */
	void PickGizmo(const FRay& WorldRay, UGizmo* Gizmo, FVector& CollisionPoint);
	bool IsRayCollideWithPlane(const FRay& WorldRay, FVector PlanePoint, FVector Normal, FVector& PointOnPlane);

private:
	bool IsRayPrimitiveCollided(const FRay& ModelRay, UPrimitiveComponent* Primitive, const FMatrix& ModelMatrix, float* ShortestDistance);
	FRay GetModelRay(const FRay& Ray, UPrimitiveComponent* Primitive);
	bool IsRayTriangleCollided(const FRay& Ray, const FVector& Vertex1, const FVector& Vertex2, const FVector& Vertex3,
		const FMatrix& ModelMatrix, float* Distance);

	UCamera* Camera;
};
