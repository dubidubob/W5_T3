#include "pch.h"
#include "Math/Frustum.h"

// =============================================================================
// FFrustumPlane Implementation
// =============================================================================

FFrustumPlane::EPlaneTestResult FFrustumPlane::TestAABB(const FAABB& Box) const
{
	if (!Box.IsValid())
	{
		return EPlaneTestResult::Outside;
	}

	// AABB의 8개 모서리 중 가장 가까운 점과 가장 먼 점 계산
	FVector MinPoint, MaxPoint;

	// X축
	if (Normal.X >= 0.0f)
	{
		MinPoint.X = Box.Min.X;
		MaxPoint.X = Box.Max.X;
	}
	else
	{
		MinPoint.X = Box.Max.X;
		MaxPoint.X = Box.Min.X;
	}

	// Y축
	if (Normal.Y >= 0.0f)
	{
		MinPoint.Y = Box.Min.Y;
		MaxPoint.Y = Box.Max.Y;
	}
	else
	{
		MinPoint.Y = Box.Max.Y;
		MaxPoint.Y = Box.Min.Y;
	}

	// Z축
	if (Normal.Z >= 0.0f)
	{
		MinPoint.Z = Box.Min.Z;
		MaxPoint.Z = Box.Max.Z;
	}
	else
	{
		MinPoint.Z = Box.Max.Z;
		MaxPoint.Z = Box.Min.Z;
	}

	// 가장 가까운 점이 평면 뒤쪽에 있으면 완전히 Outside
	if (DistanceToPoint(MaxPoint) < 0.0f)
	{
		return EPlaneTestResult::Outside;
	}

	// 가장 먼 점이 평면 앞쪽에 있으면 완전히 Inside
	if (DistanceToPoint(MinPoint) >= 0.0f)
	{
		return EPlaneTestResult::Inside;
	}

	// 그 외에는 교차
	return EPlaneTestResult::Intersect;
}

// =============================================================================
// FFrustum Implementation
// =============================================================================

void FFrustum::ExtractFromMatrix(const FMatrix& ViewProjectionMatrix)
{
	// 뷰-프로젝션 매트릭스로부터 6개 평면 추출
	// 매트릭스는 행 우선(row-major) 순서라고 가정

	const float m[16] = {
		ViewProjectionMatrix.Data[0][0], ViewProjectionMatrix.Data[0][1], ViewProjectionMatrix.Data[0][2], ViewProjectionMatrix.Data[0][3],
		ViewProjectionMatrix.Data[1][0], ViewProjectionMatrix.Data[1][1], ViewProjectionMatrix.Data[1][2], ViewProjectionMatrix.Data[1][3],
		ViewProjectionMatrix.Data[2][0], ViewProjectionMatrix.Data[2][1], ViewProjectionMatrix.Data[2][2], ViewProjectionMatrix.Data[2][3],
		ViewProjectionMatrix.Data[3][0], ViewProjectionMatrix.Data[3][1], ViewProjectionMatrix.Data[3][2], ViewProjectionMatrix.Data[3][3]
	};

	// Left plane: m[3] + m[0], m[7] + m[4], m[11] + m[8], m[15] + m[12]
	Planes[static_cast<int32>(EFrustumPlanes::Left)].Normal = FVector(m[3] + m[0], m[7] + m[4], m[11] + m[8]);
	Planes[static_cast<int32>(EFrustumPlanes::Left)].Distance = m[15] + m[12];

	// Right plane: m[3] - m[0], m[7] - m[4], m[11] - m[8], m[15] - m[12]
	Planes[static_cast<int32>(EFrustumPlanes::Right)].Normal = FVector(m[3] - m[0], m[7] - m[4], m[11] - m[8]);
	Planes[static_cast<int32>(EFrustumPlanes::Right)].Distance = m[15] - m[12];

	// Bottom plane: m[3] + m[1], m[7] + m[5], m[11] + m[9], m[15] + m[13]
	Planes[static_cast<int32>(EFrustumPlanes::Bottom)].Normal = FVector(m[3] + m[1], m[7] + m[5], m[11] + m[9]);
	Planes[static_cast<int32>(EFrustumPlanes::Bottom)].Distance = m[15] + m[13];

	// Top plane: m[3] - m[1], m[7] - m[5], m[11] - m[9], m[15] - m[13]
	Planes[static_cast<int32>(EFrustumPlanes::Top)].Normal = FVector(m[3] - m[1], m[7] - m[5], m[11] - m[9]);
	Planes[static_cast<int32>(EFrustumPlanes::Top)].Distance = m[15] - m[13];

	// Near plane: m[3] + m[2], m[7] + m[6], m[11] + m[10], m[15] + m[14]
	Planes[static_cast<int32>(EFrustumPlanes::Near)].Normal = FVector(m[3] + m[2], m[7] + m[6], m[11] + m[10]);
	Planes[static_cast<int32>(EFrustumPlanes::Near)].Distance = m[15] + m[14];

	// Far plane: m[3] - m[2], m[7] - m[6], m[11] - m[10], m[15] - m[14]
	Planes[static_cast<int32>(EFrustumPlanes::Far)].Normal = FVector(m[3] - m[2], m[7] - m[6], m[11] - m[10]);
	Planes[static_cast<int32>(EFrustumPlanes::Far)].Distance = m[15] - m[14];

	// 평면들 정규화
	Normalize();
}

void FFrustum::Normalize()
{
	for (int32 i = 0; i < static_cast<int32>(EFrustumPlanes::Count); ++i)
	{
		float Length = Planes[i].Normal.Length();
		if (Length > 0.0001f)
		{
			float InvLength = 1.0f / Length;
			Planes[i].Normal *= InvLength;
			Planes[i].Distance *= InvLength;
		}
	}
}

bool FFrustum::IsAABBInside(const FAABB& Box) const
{
	if (!Box.IsValid())
	{
		return false;
	}

	// 모든 평면에 대해 AABB가 안쪽에 있어야 함
	for (int32 i = 0; i < static_cast<int32>(EFrustumPlanes::Count); ++i)
	{
		auto Result = Planes[i].TestAABB(Box);
		if (Result == FFrustumPlane::EPlaneTestResult::Outside)
		{
			return false;
		}
	}

	return true;
}

bool FFrustum::IntersectsAABB(const FAABB& Box) const
{
	if (!Box.IsValid())
	{
		return false;
	}

	// 모든 평면에 대해 AABB가 완전히 바깥쪽에 있지 않으면 교차
	for (int32 i = 0; i < static_cast<int32>(EFrustumPlanes::Count); ++i)
	{
		auto Result = Planes[i].TestAABB(Box);
		if (Result == FFrustumPlane::EPlaneTestResult::Outside)
		{
			return false; // 하나라도 완전히 바깥쪽이면 교차하지 않음
		}
	}

	return true;
}

bool FFrustum::IsPointInside(const FVector& Point) const
{
	// 모든 평면에 대해 점이 안쪽에 있어야 함
	for (int32 i = 0; i < static_cast<int32>(EFrustumPlanes::Count); ++i)
	{
		if (Planes[i].DistanceToPoint(Point) < 0.0f)
		{
			return false;
		}
	}

	return true;
}

bool FFrustum::IntersectsSphere(const FVector& Center, float Radius) const
{
	// 모든 평면에 대해 구체가 완전히 바깥쪽에 있지 않으면 교차
	for (int32 i = 0; i < static_cast<int32>(EFrustumPlanes::Count); ++i)
	{
		float Distance = Planes[i].DistanceToPoint(Center);
		if (Distance < -Radius)
		{
			return false; // 구체가 평면 바깥쪽에 완전히 위치
		}
	}

	return true;
}

void FFrustum::GetCornerPoints(FVector OutCorners[8]) const
{
	// TODO: 프러스텀의 8개 모서리 점 계산
	// 3개 평면의 교점을 구해서 8개 점을 계산
	// 현재는 간단히 초기화만

	for (int32 i = 0; i < 8; ++i)
	{
		OutCorners[i] = FVector(0, 0, 0);
	}
}
