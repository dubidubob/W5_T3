#include "pch.h"
#include "Math/AABB.h"

const float FAABB::SMALL_NUMBER = 1e-6f;

FAABB::FAABB()
	: Min(FLT_MAX, FLT_MAX, FLT_MAX)
	, Max(-FLT_MAX, -FLT_MAX, -FLT_MAX)
{
}

FAABB::FAABB(const FVector& InMin, const FVector& InMax)
	: Min(InMin)
	, Max(InMax)
{
}

FAABB::FAABB(const FAABB& Other)
	: Min(Other.Min)
	, Max(Other.Max)
{
}

void FAABB::Reset()
{
	Min = FVector(FLT_MAX, FLT_MAX, FLT_MAX);
	Max = FVector(-FLT_MAX, -FLT_MAX, -FLT_MAX);
}

void FAABB::AddPoint(const FVector& Point)
{
	Min.X = std::min(Min.X, Point.X);
	Min.Y = std::min(Min.Y, Point.Y);
	Min.Z = std::min(Min.Z, Point.Z);

	Max.X = std::max(Max.X, Point.X);
	Max.Y = std::max(Max.Y, Point.Y);
	Max.Z = std::max(Max.Z, Point.Z);
}

void FAABB::AddAABB(const FAABB& Other)
{
	if (!Other.IsValid()) return;

	AddPoint(Other.Min);
	AddPoint(Other.Max);
}

FVector FAABB::GetCenter() const
{
	return (Min + Max) * 0.5f;
}

FVector FAABB::GetExtent() const
{
	return (Max - Min) * 0.5f;
}

FVector FAABB::GetSize() const
{
	return Max - Min;
}

float FAABB::GetVolume() const
{
	if (!IsValid()) return 0.0f;

	FVector Size = GetSize();
	return Size.X * Size.Y * Size.Z;
}

float FAABB::GetSurfaceArea() const
{
	if (!IsValid()) return 0.0f;

	FVector Size = GetSize();
	return 2.0f * (Size.X * Size.Y + Size.Y * Size.Z + Size.Z * Size.X);
}

bool FAABB::IsValid() const
{
	return Min.X <= Max.X && Min.Y <= Max.Y && Min.Z <= Max.Z;
}

bool FAABB::Contains(const FVector& Point) const
{
	return Point.X >= Min.X && Point.X <= Max.X &&
		   Point.Y >= Min.Y && Point.Y <= Max.Y &&
		   Point.Z >= Min.Z && Point.Z <= Max.Z;
}

bool FAABB::Intersects(const FAABB& Other) const
{
	if (!IsValid() || !Other.IsValid()) return false;

	return Min.X <= Other.Max.X && Max.X >= Other.Min.X &&
		   Min.Y <= Other.Max.Y && Max.Y >= Other.Min.Y &&
		   Min.Z <= Other.Max.Z && Max.Z >= Other.Min.Z;
}

bool FAABB::IntersectsRay(const FVector4& RayOrigin, const FVector4& RayDirection, float* Distance) const
{
	if (!IsValid()) return false;

	FVector Origin(RayOrigin.X, RayOrigin.Y, RayOrigin.Z);
	FVector Direction(RayDirection.X, RayDirection.Y, RayDirection.Z);

	float tMin = 0.0f;
	float tMax = FLT_MAX;

	for (int i = 0; i < 3; ++i)
	{
		float dirComponent = (i == 0) ? Direction.X : (i == 1) ? Direction.Y : Direction.Z;
		float originComponent = (i == 0) ? Origin.X : (i == 1) ? Origin.Y : Origin.Z;
		float minComponent = (i == 0) ? Min.X : (i == 1) ? Min.Y : Min.Z;
		float maxComponent = (i == 0) ? Max.X : (i == 1) ? Max.Y : Max.Z;

		if (std::abs(dirComponent) < SMALL_NUMBER)
		{
			if (originComponent < minComponent || originComponent > maxComponent)
				return false;
		}
		else
		{
			float invDir = 1.0f / dirComponent;
			float t1 = (minComponent - originComponent) * invDir;
			float t2 = (maxComponent - originComponent) * invDir;

			if (t1 > t2) std::swap(t1, t2);

			tMin = std::max(tMin, t1);
			tMax = std::min(tMax, t2);

			if (tMin > tMax) return false;
		}
	}

	if (Distance && tMin >= 0.0f)
	{
		*Distance = tMin;
	}

	return tMin <= tMax && tMax >= 0.0f;
}

FAABB FAABB::TransformBy(const FMatrix& Transform) const
{
	if (!IsValid()) return FAABB();

	FAABB Result;

	FVector Corners[8] = {
		FVector(Min.X, Min.Y, Min.Z),
		FVector(Min.X, Min.Y, Max.Z),
		FVector(Min.X, Max.Y, Min.Z),
		FVector(Min.X, Max.Y, Max.Z),
		FVector(Max.X, Min.Y, Min.Z),
		FVector(Max.X, Min.Y, Max.Z),
		FVector(Max.X, Max.Y, Min.Z),
		FVector(Max.X, Max.Y, Max.Z)
	};

	for (int32 i = 0; i < 8; ++i)
	{
		FVector4 TransformedPoint = FVector4(Corners[i].X, Corners[i].Y, Corners[i].Z, 1.0f) * Transform;
		Result.AddPoint(FVector(TransformedPoint.X, TransformedPoint.Y, TransformedPoint.Z));
	}

	return Result;
}

void FAABB::ExpandBy(float Amount)
{
	if (!IsValid()) return;

	FVector Expansion(Amount, Amount, Amount);
	Min -= Expansion;
	Max += Expansion;
}

void FAABB::ExpandBy(const FVector& Amount)
{
	if (!IsValid()) return;

	Min -= Amount;
	Max += Amount;
}

FAABB& FAABB::operator+=(const FVector& Point)
{
	AddPoint(Point);
	return *this;
}

FAABB& FAABB::operator+=(const FAABB& Other)
{
	AddAABB(Other);
	return *this;
}

FAABB FAABB::operator+(const FAABB& Other) const
{
	FAABB Result(*this);
	Result.AddAABB(Other);
	return Result;
}

bool FAABB::operator==(const FAABB& Other) const
{
	return (Min.X == Other.Min.X && Min.Y == Other.Min.Y && Min.Z == Other.Min.Z &&
		    Max.X == Other.Max.X && Max.Y == Other.Max.Y && Max.Z == Other.Max.Z);
}

bool FAABB::operator!=(const FAABB& Other) const
{
	return !(*this == Other);
}
