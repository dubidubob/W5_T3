#include "pch.h"
#include "Rect.h"
FRect::FRect()
	: X(0.0f), Y(0.0f), Width(0.0f), Height(0.0f)
{
}

FRect::FRect(float InX, float InY, float InWidth, float InHeight)
	: X(InX), Y(InY), Width(InWidth), Height(InHeight)
{
}

FRect::FRect(const FVector2& InMin, const FVector2& InMax)
	: X(InMin.X), Y(InMin.Y), Width(InMax.X - InMin.X), Height(InMax.Y - InMin.Y)
{
}

// 정적 생성 함수
FRect FRect::FromCenterAndSize(float CenterX, float CenterY, float InWidth, float InHeight)
{
	return FRect(CenterX - InWidth * 0.5f, CenterY - InHeight * 0.5f, InWidth, InHeight);
}

// 기본 접근자들
float FRect::GetLeft() const
{
	return X;
}

float FRect::GetTop() const
{
	return Y;
}

float FRect::GetRight() const
{
	return X + Width;
}

float FRect::GetBottom() const
{
	return Y + Height;
}

float FRect::GetCenterX() const
{
	return X + Width * 0.5f;
}

float FRect::GetCenterY() const
{
	return Y + Height * 0.5f;
}

FVector2 FRect::GetCenter() const
{
	return FVector2(GetCenterX(), GetCenterY());
}

FVector2 FRect::GetTopLeft() const
{
	return FVector2(X, Y);
}

FVector2 FRect::GetTopRight() const
{
	return FVector2(GetRight(), Y);
}

FVector2 FRect::GetBottomLeft() const
{
	return FVector2(X, GetBottom());
}

FVector2 FRect::GetBottomRight() const
{
	return FVector2(GetRight(), GetBottom());
}

FVector2 FRect::GetSize() const
{
	return FVector2(Width, Height);
}

float FRect::GetArea() const
{
	return Width * Height;
}

float FRect::GetPerimeter() const
{
	return 2.0f * (Width + Height);
}

bool FRect::IsValid() const
{
	return Width > 0.0f && Height > 0.0f;
}

bool FRect::IsEmpty() const
{
	return Width <= 0.0f || Height <= 0.0f;
}

// 포인트 포함 검사
bool FRect::Contains(float PointX, float PointY) const
{
	return PointX >= X && PointX <= GetRight() &&
		PointY >= Y && PointY <= GetBottom();
}

bool FRect::Contains(const FVector2& Point) const
{
	return Contains(Point.X, Point.Y);
}

// 사각형 포함 검사
bool FRect::Contains(const FRect& Other) const
{
	return Other.X >= X && Other.Y >= Y &&
		Other.GetRight() <= GetRight() &&
		Other.GetBottom() <= GetBottom();
}

// 사각형 교집합 검사
bool FRect::Intersects(const FRect& Other) const
{
	return !(Other.X > GetRight() || Other.GetRight() < X ||
		Other.Y > GetBottom() || Other.GetBottom() < Y);
}

// 교집합 사각형 계산
FRect FRect::GetIntersection(const FRect& Other) const
{
	if (!Intersects(Other))
		return FRect(); // 빈 사각형 반환

	float NewX = std::max(X, Other.X);
	float NewY = std::max(Y, Other.Y);
	float NewRight = std::min(GetRight(), Other.GetRight());
	float NewBottom = std::min(GetBottom(), Other.GetBottom());

	return FRect(NewX, NewY, NewRight - NewX, NewBottom - NewY);
}

// 합집합 사각형 계산
FRect FRect::GetUnion(const FRect& Other) const
{
	if (IsEmpty()) return Other;
	if (Other.IsEmpty()) return *this;

	float NewX = std::min(X, Other.X);
	float NewY = std::min(Y, Other.Y);
	float NewRight = std::max(GetRight(), Other.GetRight());
	float NewBottom = std::max(GetBottom(), Other.GetBottom());

	return FRect(NewX, NewY, NewRight - NewX, NewBottom - NewY);
}

// 사각형 확장/축소
FRect FRect::Expand(float Amount) const
{
	return FRect(X - Amount, Y - Amount, Width + Amount * 2.0f, Height + Amount * 2.0f);
}

FRect FRect::Expand(float HorizontalAmount, float VerticalAmount) const
{
	return FRect(X - HorizontalAmount, Y - VerticalAmount,
		Width + HorizontalAmount * 2.0f, Height + VerticalAmount * 2.0f);
}

// 사각형 이동
FRect FRect::Offset(float DeltaX, float DeltaY) const
{
	return FRect(X + DeltaX, Y + DeltaY, Width, Height);
}

FRect FRect::Offset(const FVector2& Delta) const
{
	return Offset(Delta.X, Delta.Y);
}

// 크기 조정
FRect FRect::Scale(float ScaleFactor) const
{
	return FRect(X, Y, Width * ScaleFactor, Height * ScaleFactor);
}

FRect FRect::Scale(float ScaleX, float ScaleY) const
{
	return FRect(X, Y, Width * ScaleX, Height * ScaleY);
}

// 중심 기준으로 크기 조정
FRect FRect::ScaleFromCenter(float ScaleFactor) const
{
	FVector2 Center = GetCenter();
	float NewWidth = Width * ScaleFactor;
	float NewHeight = Height * ScaleFactor;
	return FromCenterAndSize(Center.X, Center.Y, NewWidth, NewHeight);
}

// 연산자 오버로딩
bool FRect::operator==(const FRect& Other) const
{
	return X == Other.X && Y == Other.Y &&
		Width == Other.Width && Height == Other.Height;
}

bool FRect::operator!=(const FRect& Other) const
{
	return !(*this == Other);
}

FRect FRect::operator+(const FVector2& Offset) const
{
	return FRect(X + Offset.X, Y + Offset.Y, Width, Height);
}

FRect FRect::operator-(const FVector2& Offset) const
{
	return FRect(X - Offset.X, Y - Offset.Y, Width, Height);
}

FRect& FRect::operator+=(const FVector2& Offset)
{
	X += Offset.X;
	Y += Offset.Y;
	return *this;
}

FRect& FRect::operator-=(const FVector2& Offset)
{
	X -= Offset.X;
	Y -= Offset.Y;
	return *this;
}

FString FRect::ToString() const
{
	return FString("FRect(X=") + std::to_string(X) +
		FString(", Y=") + std::to_string(Y) +
		FString(", Width=") + std::to_string(Width) +
		FString(", Height=") + std::to_string(Height) +
		FString(")");
}
