#pragma once

/**
 * @brief 2D 사각형을 나타내는 구조체
 */
struct FRect
{
public:
	float X;
	float Y;
	float Width;
	float Height;

public:
	/**
	 * @brief 모든 값이 0으로 초기화된 빈 사각형을 생성
	 */
	FRect();

	/**
	 * 매개변수 생성자
	 * @param InX 좌상단 X 좌표
	 * @param InY 좌상단 Y 좌표
	 * @param InWidth 가로 크기
	 * @param InHeight 세로 크기
	 * @brief 지정된 좌표와 크기로 사각형을 생성합니다.
	 */
	FRect(float InX, float InY, float InWidth, float InHeight);

	/**
	 * 두 점으로부터 사각형 생성
	 * @param InMin 최소 좌표 (좌상단)
	 * @param InMax 최대 좌표 (우하단)
	 * @brief 두 점을 대각선 모서리로 하는 사각형을 생성합니다.
	 */
	FRect(const FVector2& InMin, const FVector2& InMax);

	/**
	 * 중심점과 크기로부터 사각형 생성
	 * @param CenterX 중심점 X 좌표
	 * @param CenterY 중심점 Y 좌표
	 * @param InWidth 가로 크기
	 * @param InHeight 세로 크기
	 * @return 생성된 사각형
	 * @brief 중심점을 기준으로 사각형을 생성합니다.
	 */
	static FRect FromCenterAndSize(float CenterX, float CenterY, float InWidth, float InHeight);

	/**
	 * @return 왼쪽 경계의 X 좌표
	 */
	float GetLeft() const;

	/**
	 * @return 위쪽 경계의 Y 좌표
	 */
	float GetTop() const;

	/**
	 * @return 오른쪽 경계의 X 좌표
	 */
	float GetRight() const;

	/**
	 * @return 아래쪽 경계의 Y 좌표
	 */
	float GetBottom() const;

	/**
	 * @return 중심점 X 좌표
	 */
	float GetCenterX() const;

	/**
	 * @return 중심점 Y 좌표
	 */
	float GetCenterY() const;

	/**
	 * @return 중심점 벡터
	 */
	FVector2 GetCenter() const;

	/**
	 * @return 좌상단 좌표 벡터
	 */
	FVector2 GetTopLeft() const;

	/**
	 * @return 우상단 좌표 벡터
	 */
	FVector2 GetTopRight() const;

	/**
	 * @return 좌하단 좌표 벡터
	 */
	FVector2 GetBottomLeft() const;

	/**
	 * @return 우하단 좌표 벡터
	 */
	FVector2 GetBottomRight() const;

	/**
	 * @return 크기 벡터 (Width, Height)
	 */
	FVector2 GetSize() const;

	/**
	 * @return 면적 (Width * Height)
	 */
	float GetArea() const;

	/**
	 * @return 둘레 (2 * (Width + Height))
	 */
	float GetPerimeter() const;

	/**
	 * @return Width와 Height가 모두 0보다 크면 true
	 */
	bool IsValid() const;

	/**
	 * @return Width나 Height가 0 이하이면 true
	 */
	bool IsEmpty() const;

	/**
	 * 지정된 점이 사각형 내부에 포함되는지 확인
	 */
	bool Contains(float PointX, float PointY) const;

	/**
	 * 지정된 점이 사각형 내부에 포함되는지 확인
	 */
	bool Contains(const FVector2& Point) const;

	/**
	 * 다른 사각형이 이 사각형에 완전히 포함되는지 확인
	 */
	bool Contains(const FRect& Other) const;

	/**
	 * 다른 사각형과 교집합이 있는지 확인
	 * @return 교집합이 존재하면 true
	 */
	bool Intersects(const FRect& Other) const;

	/**
	 * 다른 사각형과의 교집합 사각형을 계산
	 * @return 교집합 사각형 (교집합이 없으면 빈 사각형)
	 */
	FRect GetIntersection(const FRect& Other) const;

	/**
	 * 다른 사각형과의 합집합 사각형을 계산
	 * @return 두 사각형을 모두 포함하는 최소 사각형
	 */
	FRect GetUnion(const FRect& Other) const;

	/**
	 * 사각형을 확장/축소
	 */
	FRect Expand(float Amount) const;
	FRect Expand(float HorizontalAmount, float VerticalAmount) const;

	/**
	 * 사각형을 지정된 오프셋만큼 이동
	 */
	FRect Offset(float DeltaX, float DeltaY) const;
	FRect Offset(const FVector2& Delta) const;

	/**
	 * 사각형을 크기 비율로 조정
	 * @note 좌상단 모서리는 고정되고 크기만 변경
	 */
	FRect Scale(float ScaleFactor) const;
	FRect Scale(float ScaleX, float ScaleY) const;

	/**
	 * 사각형을 중심점을 기준으로 크기 조정
	 * @note 중심점은 고정되고 크기만 변경
	 */
	FRect ScaleFromCenter(float ScaleFactor) const;

	bool operator==(const FRect& Other) const;
	bool operator!=(const FRect& Other) const;
	FRect operator+(const FVector2& Offset) const;
	FRect operator-(const FVector2& Offset) const;
	FRect& operator+=(const FVector2& Offset);
	FRect& operator-=(const FVector2& Offset);

	FString ToString() const;
};
