#pragma once
#include "Global/Vector.h"
#include "Math/AABB.h"

/**
 * @brief Frustum Plane - 프러스텀을 구성하는 평면
 */
struct FFrustumPlane
{
	FVector Normal;    // 평면의 법선 벡터
	float Distance;    // 원점으로부터의 거리

	FFrustumPlane() : Normal(0, 0, 0), Distance(0.0f) {}
	FFrustumPlane(const FVector& InNormal, float InDistance)
		: Normal(InNormal), Distance(InDistance) {}

	/** 점과 평면의 거리 계산 (양수: 평면 앞쪽, 음수: 평면 뒤쪽) */
	float DistanceToPoint(const FVector& Point) const
	{
		return Normal.Dot(Point) + Distance;
	}

	/** AABB와 평면의 관계 체크 */
	enum class EPlaneTestResult
	{
		Inside,     // 완전히 평면 안쪽
		Outside,    // 완전히 평면 바깥쪽
		Intersect   // 평면과 교차
	};

	EPlaneTestResult TestAABB(const FAABB& Box) const;
};

/**
 * @brief View Frustum - 카메라의 시야 절두체
 */
struct FFrustum
{
	enum class EFrustumPlanes
	{
		Near = 0,
		Far,
		Left,
		Right,
		Top,
		Bottom,
		Count
	};

	FFrustumPlane Planes[static_cast<int32>(EFrustumPlanes::Count)];

	FFrustum() = default;

	/** 뷰-프로젝션 매트릭스로부터 프러스텀 생성 */
	void ExtractFromMatrix(const FMatrix& ViewProjectionMatrix);

	/** AABB가 프러스텀 안에 있는지 체크 */
	bool IsAABBInside(const FAABB& Box) const;

	/** AABB가 프러스텀과 교차하는지 체크 (컬링용) */
	bool IntersectsAABB(const FAABB& Box) const;

	/** 점이 프러스텀 안에 있는지 체크 */
	bool IsPointInside(const FVector& Point) const;

	/** 구체가 프러스텀과 교차하는지 체크 */
	bool IntersectsSphere(const FVector& Center, float Radius) const;

	/** 프러스텀 평면들의 법선 벡터 정규화 */
	void Normalize();

	/** 디버그용 프러스텀 모서리 점들 계산 */
	void GetCornerPoints(FVector OutCorners[8]) const;
};
