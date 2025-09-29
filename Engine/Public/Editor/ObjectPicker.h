#pragma once
#include "Editor/Gizmo.h"

class UPrimitiveComponent;
class UStaticMeshComponent;
class AActor;
class ULevel;
class UCamera;
class UGizmo;
class UDeviceResources;
struct FRay;

class UObjectPicker : public UObject
{
	DECLARE_CLASS(UObjectPicker, UObject)

public:
	UObjectPicker();
	void SetCamera(UCamera* Camera);
	void SetDeviceResources(UDeviceResources* InDeviceResources);

	UStaticMeshComponent* PickPrimitive(const FRay& WorldRay, TArray<UStaticMeshComponent*> Candidate, float* Distance);
	UPrimitiveComponent* PickPrimitiveByColor(int32 MouseX, int32 MouseY);

	void PickGizmo(const FRay& WorldRay, UGizmo* Gizmo, FVector& CollisionPoint);
	bool IsRayCollideWithPlane(const FRay& WorldRay, FVector PlanePoint, FVector Normal, FVector& PointOnPlane);

private:
	bool IsRayPrimitiveCollided(const FRay& ModelRay, UPrimitiveComponent* Primitive, const FMatrix& ModelMatrix, float* ShortestDistance);
	FRay GetModelRay(const FRay& Ray, UPrimitiveComponent* Primitive);
	bool IsRayTriangleCollided(const FRay& Ray, const FVector& Vertex1, const FVector& Vertex2, const FVector& Vertex3,
		const FMatrix& ModelMatrix, float* Distance);

	UCamera* Camera;
	UDeviceResources* DeviceResources;
};
