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

	float TMin = 0.0f;
	float TMax = FLT_MAX;

	for (int i = 0; i < 3; ++i)
	{
		float DirComponent, OriginComponent, MinComponent, MaxComponent;

		if (i == 0)
		{
			DirComponent = Direction.X;
			OriginComponent = Origin.X;
			MinComponent = Min.X;
			MaxComponent = Max.X;
		}
		else if (i == 1)
		{
			DirComponent = Direction.Y;
			OriginComponent = Origin.Y;
			MinComponent = Min.Y;
			MaxComponent = Max.Y;
		}
		else
		{
			DirComponent = Direction.Z;
			OriginComponent = Origin.Z;
			MinComponent = Min.Z;
			MaxComponent = Max.Z;
		}

		// 해당 축 성분이 0이면, 광선은 해당 슬랩 평면과 평행해진다.
		if (std::abs(DirComponent) < SMALL_NUMBER)
		{
			// 거기다가 Ray의 위치가 AABB를 벗어난다면
			if (OriginComponent < MinComponent || OriginComponent > MaxComponent)
			{
				// Ray와 AABB는 만날 수 없다.
				return false;
			}
		}
		else
		{
			float InvDir = 1.0f / DirComponent;
			float T1 = (MinComponent - OriginComponent) * InvDir;
			float T2 = (MaxComponent - OriginComponent) * InvDir;

			if (T1 > T2) std::swap(T1, T2);

			TMin = std::max(TMin, T1);	// 시작점 중에 가장 큰 값
			TMax = std::min(TMax, T2);	// 끝점 중에 가장 작은 값

			// 한번이라도 TMin이 TMax보다 커진다면, 그 광선은 AABB를 벗어난 것
			if (TMin > TMax) return false;
		}
	}

	if (Distance && TMin >= 0.0f)
	{
		*Distance = TMin;
	}

	return TMin <= TMax && TMax >= 0.0f;
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
